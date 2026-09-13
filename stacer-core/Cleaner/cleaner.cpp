#include "cleaner.h"

#include "Utils/procfs.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QtConcurrent>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KIO/EmptyTrashJob>
#include <KJob>

namespace
{
struct CleanCategory {
    QString id;
    QString title;
    QStringList targets;
    bool root = false;
};

QVariantList entriesForTargets(const QStringList &targets, bool root)
{
    QVariantList entries;
    for (const QString &target : targets) {
        const QFileInfo info(target);
        if (!info.exists() && !info.isSymLink()) {
            continue;
        }
        entries.append(QVariantMap {
            { QStringLiteral("title"), info.fileName().isEmpty() ? target : info.fileName() },
            { QStringLiteral("path"), target },
            { QStringLiteral("size"), Cleaner::directorySize(target) },
            { QStringLiteral("root"), root },
        });
    }
    return entries;
}

QVariantMap buildCategory(const CleanCategory &category)
{
    const QVariantList entries = entriesForTargets(category.targets, category.root);
    qulonglong total = 0;
    for (const QVariant &entry : entries) {
        total += entry.toMap().value(QStringLiteral("size")).toULongLong();
    }

    return QVariantMap {
        { QStringLiteral("id"), category.id },
        { QStringLiteral("title"), category.title },
        { QStringLiteral("size"), total },
        { QStringLiteral("entries"), entries },
    };
}
}

Cleaner::Cleaner(QObject *parent) :
    QObject(parent)
{
}

bool Cleaner::scanning() const
{
    return m_scanning;
}

QString Cleaner::trashDirectory()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/Trash");
}

qulonglong Cleaner::directorySize(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists()) {
        return 0;
    }
    if (info.isFile()) {
        return static_cast<qulonglong>(info.size());
    }

    qulonglong total = 0;
    QDirIterator iterator(path, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        total += static_cast<qulonglong>(iterator.fileInfo().size());
    }
    return total;
}

QVariantList Cleaner::buildCategories() const
{
    const QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    // Trash
    CleanCategory trash;
    trash.id = QStringLiteral("trash");
    trash.title = tr("Trash");
    trash.targets = { trashDirectory() + QStringLiteral("/files"), trashDirectory() + QStringLiteral("/info") };

    // Application caches (user writable)
    CleanCategory caches;
    caches.id = QStringLiteral("caches");
    caches.title = tr("Application Caches");

    const QString genericCache = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    const QDir cacheDir(genericCache);
    const QStringList cacheEntries = cacheDir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (const QString &entry : cacheEntries) {
        caches.targets.append(cacheDir.filePath(entry));
    }
    const QStringList knownCaches = {
        home + QStringLiteral("/.npm"),
        home + QStringLiteral("/.bun/install/cache"),
        home + QStringLiteral("/.gradle/caches"),
        home + QStringLiteral("/.cargo/registry"),
        home + QStringLiteral("/.m2/repository"),
    };
    for (const QString &target : knownCaches) {
        if (QFileInfo::exists(target)) {
            caches.targets.append(target);
        }
    }

    // Application logs (root owned)
    CleanCategory logs;
    logs.id = QStringLiteral("logs");
    logs.title = tr("Application Logs");
    logs.root = true;
    const QDir logDir(QStringLiteral("/var/log"));
    const QStringList logEntries = logDir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (const QString &entry : logEntries) {
        if (entry.endsWith(QLatin1String(".log")) || entry.contains(QLatin1String(".log.")) || entry.endsWith(QLatin1String(".old"))) {
            logs.targets.append(logDir.filePath(entry));
        }
    }

    // Crash reports (root owned)
    CleanCategory crash;
    crash.id = QStringLiteral("crash");
    crash.title = tr("Crash Reports");
    crash.root = true;
    const QStringList crashDirs = {
        QStringLiteral("/var/crash"),
        QStringLiteral("/var/lib/systemd/coredump"),
    };
    for (const QString &dirPath : crashDirs) {
        const QDir dir(dirPath);
        const QStringList entries = dir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
        for (const QString &entry : entries) {
            crash.targets.append(dir.filePath(entry));
        }
    }

    return {
        buildCategory(trash),
        buildCategory(caches),
        buildCategory(logs),
        buildCategory(crash),
    };
}

void Cleaner::scan()
{
    if (m_scanning) {
        return;
    }

    m_scanning = true;
    Q_EMIT scanningChanged();

    QPointer<Cleaner> guard(this);
    (void)QtConcurrent::run([guard] {
        if (!guard) {
            return;
        }
        const QVariantList categories = guard->buildCategories();
        QMetaObject::invokeMethod(guard, [guard, categories] {
            if (!guard) {
                return;
            }
            guard->m_scanning = false;
            Q_EMIT guard->scanningChanged();
            Q_EMIT guard->scanned(categories);
        }, Qt::QueuedConnection);
    });
}

void Cleaner::clean(const QStringList &paths)
{
    if (paths.isEmpty()) {
        Q_EMIT cleaned(0, {});
        return;
    }

    const QString trash = trashDirectory();
    QStringList userPaths;
    QStringList rootPaths;
    bool emptyTrash = false;
    int count = 0;

    for (const QString &path : paths) {
        const QString cleanedPath = QDir::cleanPath(path);
        if (cleanedPath.startsWith(trash + QLatin1Char('/'))) {
            emptyTrash = true;
            ++count;
            continue;
        }

        const QFileInfo info(cleanedPath);
        const QFileInfo parentInfo(info.absolutePath());
        const bool writable = (info.exists() ? info.isWritable() : parentInfo.isWritable()) && parentInfo.isWritable();
        if (writable) {
            userPaths.append(cleanedPath);
        } else {
            rootPaths.append(cleanedPath);
        }
        ++count;
    }

    struct CleanState {
        int remaining = 0;
        int count = 0;
        QString error;
    };
    auto state = QSharedPointer<CleanState>::create();
    state->count = count;

    const auto finish = [this, state](const QString &error) {
        if (!error.isEmpty() && state->error.isEmpty()) {
            state->error = error;
        }
        if (--state->remaining == 0) {
            Q_EMIT cleaned(state->error.isEmpty() ? state->count : 0, state->error);
        }
    };

    if (!rootPaths.isEmpty()) {
        ++state->remaining;

        KAuth::Action action(QStringLiteral(STACER_APP_ID ".clean"));
        action.setHelperId(QStringLiteral(STACER_HELPER_ID));
        action.addArgument(QStringLiteral("paths"), rootPaths);

        KAuth::ExecuteJob *job = action.execute();
        connect(job, &KJob::result, this, [finish](KJob *kjob) {
            finish(kjob->error() != KJob::NoError ? kjob->errorString() : QString());
        });
    }

    state->remaining += userPaths.size();
    if (emptyTrash) {
        ++state->remaining;
    }

    if (state->remaining == 0) {
        Q_EMIT cleaned(count, {});
        return;
    }

    for (const QString &path : userPaths) {
        const QFileInfo info(path);
        bool success = false;
        if (info.isDir() && !info.isSymLink()) {
            success = QDir(path).removeRecursively();
        } else {
            success = QFile::remove(path);
        }
        finish(success ? QString() : tr("Failed to remove %1").arg(path));
    }

    if (emptyTrash) {
        KIO::EmptyTrashJob *job = KIO::emptyTrash();
        connect(job, &KJob::result, this, [finish](KJob *kjob) {
            finish(kjob->error() != KJob::NoError ? kjob->errorString() : QString());
        });
    }
}

// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "cleaner.h"

#include "Utils/procfs.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QProcess>
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
        // Pseudo entries such as "pkg:name" describe packages removed through
        // the privileged package-manager action instead of plain file deletion.
        if (target.startsWith(QLatin1String("pkg:"))) {
            entries.append(QVariantMap {
                { QStringLiteral("title"), target.mid(4) },
                { QStringLiteral("path"), target },
                { QStringLiteral("size"), 0 },
                { QStringLiteral("root"), root },
            });
            continue;
        }

        // The systemd journal is vacuumed through journalctl, not deleted by hand.
        if (target.startsWith(QLatin1String("journal:"))) {
            const qulonglong journalSize = Cleaner::directorySize(QStringLiteral("/var/log/journal"));
            constexpr qulonglong vacuumLimit = 50ULL * 1024 * 1024;
            entries.append(QVariantMap {
                { QStringLiteral("title"), Cleaner::tr("Systemd Journal") },
                { QStringLiteral("path"), target },
                { QStringLiteral("size"), journalSize > vacuumLimit ? journalSize - vacuumLimit : 0 },
                { QStringLiteral("root"), true },
            });
            continue;
        }

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

QVariantList Cleaner::buildCategories()
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

    // System logs (root owned)
    CleanCategory logs;
    logs.id = QStringLiteral("logs");
    logs.title = tr("System Logs");
    logs.root = true;
    const QDir logDir(QStringLiteral("/var/log"));
    const QStringList logEntries = logDir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (const QString &entry : logEntries) {
        const int dot = entry.lastIndexOf(QLatin1Char('.'));
        const bool rotatedNumber = dot >= 0 && entry.mid(dot + 1).toInt() > 0;
        if (entry.endsWith(QLatin1String(".log")) || entry.contains(QLatin1String(".log.")) || entry.endsWith(QLatin1String(".old"))
            || entry.endsWith(QLatin1String(".gz")) || entry.endsWith(QLatin1String(".xz")) || entry.endsWith(QLatin1String(".zst"))
            || rotatedNumber) {
            logs.targets.append(logDir.filePath(entry));
        }
    }
    logs.targets.append(QStringLiteral("journal:"));

    // Pacman package cache (root owned)
    CleanCategory packages;
    packages.id = QStringLiteral("pacman");
    packages.title = tr("Pacman Package Cache");
    packages.root = true;
    const QDir packageDir(QStringLiteral("/var/cache/pacman/pkg"));
    const QStringList cachedPackages = packageDir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (const QString &package : cachedPackages) {
        packages.targets.append(packageDir.filePath(package));
    }

    // Orphan packages (removed with pacman through the privileged helper)
    CleanCategory orphans;
    orphans.id = QStringLiteral("orphans");
    orphans.title = tr("Orphan Packages");
    QProcess pacmanQuery;
    pacmanQuery.start(QStringLiteral("pacman"), { QStringLiteral("-Qtdq") });
    if (pacmanQuery.waitForFinished(10000) && pacmanQuery.exitCode() == 0) {
        const QStringList names = QString::fromLocal8Bit(pacmanQuery.readAllStandardOutput()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString &name : names) {
            orphans.targets.append(QStringLiteral("pkg:") + name.trimmed());
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
        buildCategory(packages),
        buildCategory(orphans),
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
        const QVariantList categories = Cleaner::buildCategories();
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

void Cleaner::clean(const QStringList &paths, const QStringList &orphanPackages, bool vacuumJournal)
{
    if (paths.isEmpty() && orphanPackages.isEmpty() && !vacuumJournal) {
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

        KAuth::Action action(QStringLiteral(KLEANER_APP_ID ".clean"));
        action.setHelperId(QStringLiteral(KLEANER_HELPER_ID));
        action.addArgument(QStringLiteral("paths"), rootPaths);

        KAuth::ExecuteJob *job = action.execute();
        connect(job, &KJob::result, this, [finish](KJob *kjob) {
            finish(kjob->error() != KJob::NoError ? kjob->errorString() : QString());
        });
    }

    if (!orphanPackages.isEmpty()) {
        ++state->remaining;

        KAuth::Action action(QStringLiteral(KLEANER_APP_ID ".removeorphans"));
        action.setHelperId(QStringLiteral(KLEANER_HELPER_ID));
        action.addArgument(QStringLiteral("packages"), orphanPackages);

        KAuth::ExecuteJob *job = action.execute();
        connect(job, &KJob::result, this, [finish, state, job](KJob *kjob) {
            if (kjob->error() == KJob::NoError) {
                state->count += job->data().value(QStringLiteral("removed")).toInt();
            }
            finish(kjob->error() != KJob::NoError ? kjob->errorString() : QString());
        });
    }

    if (vacuumJournal) {
        ++state->remaining;
        ++state->count;

        KAuth::Action action(QStringLiteral(KLEANER_APP_ID ".vacuumjournal"));
        action.setHelperId(QStringLiteral(KLEANER_HELPER_ID));

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

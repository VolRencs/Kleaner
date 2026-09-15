// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "cleaner.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QDirListing>
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
    QString title;
    QStringList targets;
    bool root = false;
    // Categories that are cleaned as a whole (e.g. the trash through KIO)
    // expose a single pseudo path instead of child entries.
    QString action;
    qulonglong size = 0;
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

        // The systemd journal is rotated and vacuumed through journalctl.
        if (target.startsWith(QLatin1String("journal:"))) {
            const qulonglong journalSize = Cleaner::directorySize(QStringLiteral("/var/log/journal"))
                                          + Cleaner::directorySize(QStringLiteral("/run/log/journal"));
            entries.append(QVariantMap {
                { QStringLiteral("title"), Cleaner::tr("Systemd Journal") },
                { QStringLiteral("path"), target },
                { QStringLiteral("size"), journalSize },
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
    const QVariantList entries = category.action.isEmpty() ? entriesForTargets(category.targets, category.root) : QVariantList();
    qulonglong total = category.action.isEmpty() ? 0 : category.size;
    for (const QVariant &entry : entries) {
        total += entry.toMap().value(QStringLiteral("size")).toULongLong();
    }

    return QVariantMap {
        { QStringLiteral("title"), category.title },
        { QStringLiteral("size"), total },
        { QStringLiteral("entries"), entries },
        { QStringLiteral("action"), category.action },
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
    using Flag = QDirListing::IteratorFlag;
    for (const auto &entry : QDirListing(path, Flag::FilesOnly | Flag::Recursive | Flag::ResolveSymlinks)) {
        total += static_cast<qulonglong>(entry.size());
    }
    return total;
}

QVariantList Cleaner::buildCategories()
{
    const QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    // Trash (emptied as a whole through KIO)
    CleanCategory trash;
    trash.title = tr("Trash");
    trash.action = QStringLiteral("trash:");
    trash.size = directorySize(trashDirectory() + QStringLiteral("/files")) + directorySize(trashDirectory() + QStringLiteral("/info"));

    // Application caches (user writable)
    CleanCategory caches;
    caches.title = tr("Application Caches");

    const QString genericCache = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    for (const auto &entry : QDirListing(genericCache, QDirListing::IteratorFlag::ExcludeOther)) {
        if (entry.isSymLink() || (!entry.isDir() && !entry.isFile())) {
            continue;
        }
        caches.targets.append(entry.filePath());
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

    // Temporary files (root owned or leftovers from dead sessions)
    CleanCategory temporary;
    temporary.title = tr("Temporary Files");
    temporary.root = true;
    {
        const QDateTime now = QDateTime::currentDateTime();
        for (const auto &entry : QDirListing(QStringLiteral("/tmp"))) {
            const QString name = entry.fileName();
            // Session sockets and per-service private directories must survive.
            if (name.startsWith(QLatin1String("systemd-private-"))
                || name == QLatin1String("snap-private-tmp")
                || name == QLatin1String(".X11-unix") || name == QLatin1String(".ICE-unix")
                || name == QLatin1String(".font-unix") || name == QLatin1String(".Test-unix")
                || name == QLatin1String(".XIM-unix")) {
                continue;
            }
            // Files touched within the last hour are most likely still in use.
            if (entry.fileInfo().lastModified().secsTo(now) < 3600) {
                continue;
            }
            temporary.targets.append(entry.filePath());
        }
    }

    // System logs (root owned)
    CleanCategory logs;
    logs.title = tr("System Logs");
    logs.root = true;
    for (const auto &entry : QDirListing(QStringLiteral("/var/log"), QDirListing::IteratorFlag::FilesOnly)) {
        const QString name = entry.fileName();
        const int dot = name.lastIndexOf(QLatin1Char('.'));
        const bool rotatedNumber = dot >= 0 && name.mid(dot + 1).toInt() > 0;
        if (name.endsWith(QLatin1String(".log")) || name.contains(QLatin1String(".log.")) || name.endsWith(QLatin1String(".old"))
            || name.endsWith(QLatin1String(".gz")) || name.endsWith(QLatin1String(".xz")) || name.endsWith(QLatin1String(".zst"))
            || rotatedNumber) {
            logs.targets.append(entry.filePath());
        }
    }
    logs.targets.append(QStringLiteral("journal:"));

    // Pacman package cache (root owned)
    CleanCategory packages;
    packages.title = tr("Pacman Package Cache");
    packages.root = true;
    for (const auto &entry : QDirListing(QStringLiteral("/var/cache/pacman/pkg"), QDirListing::IteratorFlag::FilesOnly)) {
        packages.targets.append(entry.filePath());
    }

    // Orphan packages (removed with pacman through the privileged helper)
    CleanCategory orphans;
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
    crash.title = tr("Crash Reports");
    crash.root = true;
    const QStringList crashDirs = {
        QStringLiteral("/var/crash"),
        QStringLiteral("/var/lib/systemd/coredump"),
    };
    for (const QString &dirPath : crashDirs) {
        for (const auto &entry : QDirListing(dirPath, QDirListing::IteratorFlag::FilesOnly)) {
            crash.targets.append(entry.filePath());
        }
    }

    return {
        buildCategory(trash),
        buildCategory(caches),
        buildCategory(temporary),
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

void Cleaner::clean(const QStringList &paths, const QVariantMap &sizes, const QStringList &orphanPackages, bool vacuumJournal)
{
    if (paths.isEmpty() && orphanPackages.isEmpty() && !vacuumJournal) {
        Q_EMIT cleaned(0, 0, {});
        return;
    }

    QStringList userPaths;
    QStringList rootPaths;
    QHash<QString, qulonglong> userPathSizes;
    qulonglong trashSize = 0;
    bool emptyTrash = false;

    for (const QString &path : paths) {
        const QString cleanedPath = QDir::cleanPath(path);
        // The whole trash is emptied through KIO; the measured size is passed
        // along under the aggregate pseudo path.
        if (cleanedPath == QLatin1String("trash:")) {
            emptyTrash = true;
            trashSize += sizes.value(cleanedPath, sizes.value(path)).toULongLong();
            continue;
        }

        const QFileInfo info(cleanedPath);
        const QFileInfo parentInfo(info.absolutePath());
        const bool writable = (info.exists() ? info.isWritable() : parentInfo.isWritable()) && parentInfo.isWritable();
        if (writable) {
            userPaths.append(cleanedPath);
            qulonglong size = sizes.value(cleanedPath).toULongLong();
            if (size == 0) {
                size = sizes.value(path).toULongLong();
            }
            userPathSizes.insert(cleanedPath, size);
        } else {
            rootPaths.append(cleanedPath);
        }
    }

    struct CleanState {
        int remaining = 0;
        int count = 0;
        qulonglong freed = 0;
        QString error;
    };
    auto state = QSharedPointer<CleanState>::create();

    const auto finish = [this, state](const QString &error) {
        if (!error.isEmpty() && state->error.isEmpty()) {
            state->error = error;
        }
        if (--state->remaining == 0) {
            Q_EMIT cleaned(state->count, state->freed, state->error);
        }
    };

    // Every root-owned part of the cleanup (files, orphan packages, journal) is
    // bundled into one privileged action so the user authenticates only once.
    const bool privilegedWork = !rootPaths.isEmpty() || !orphanPackages.isEmpty() || vacuumJournal;
    if (privilegedWork) {
        ++state->remaining;

        KAuth::Action action(QStringLiteral(KLEANER_APP_ID ".clean"));
        action.setHelperId(QStringLiteral(KLEANER_HELPER_ID));
        action.addArgument(QStringLiteral("paths"), rootPaths);
        action.addArgument(QStringLiteral("orphanPackages"), orphanPackages);
        action.addArgument(QStringLiteral("vacuumJournal"), vacuumJournal);

        KAuth::ExecuteJob *job = action.execute();
        connect(job, &KJob::result, this, [finish, state, job](KJob *kjob) {
            // Partial results are reported even when the helper failed halfway.
            const QVariantMap data = job->data();
            if (!data.isEmpty()) {
                state->count += data.value(QStringLiteral("removed")).toInt();
                state->freed += data.value(QStringLiteral("freed")).toULongLong();
            }
            finish(kjob->error() != KJob::NoError ? kjob->errorString() : QString());
        });
        job->start();
    }

    if (emptyTrash) {
        ++state->remaining;
    }

    if (!userPaths.isEmpty()) {
        // The whole batch is a single task: the worker reports once when done.
        ++state->remaining;

        // User-owned paths can be large; delete them off the GUI thread.
        const QPointer<Cleaner> guard(this);
        (void)QtConcurrent::run([guard, state, userPaths, userPathSizes] {
            int count = 0;
            qulonglong freed = 0;
            QString error;
            for (const QString &path : userPaths) {
                const QFileInfo info(path);
                bool success = false;
                if (info.isDir() && !info.isSymLink()) {
                    success = QDir(path).removeRecursively();
                } else {
                    success = QFile::remove(path);
                }
                if (success) {
                    ++count;
                    freed += userPathSizes.value(path);
                } else if (error.isEmpty()) {
                    error = QCoreApplication::translate("Cleaner", "Failed to remove %1").arg(path);
                }
            }
            QMetaObject::invokeMethod(guard, [guard, state, count, freed, error] {
                if (!guard) {
                    return;
                }
                state->count += count;
                state->freed += freed;
                if (!error.isEmpty() && state->error.isEmpty()) {
                    state->error = error;
                }
                if (--state->remaining == 0) {
                    Q_EMIT guard->cleaned(state->count, state->freed, state->error);
                }
            }, Qt::QueuedConnection);
        });
    }

    if (emptyTrash) {
        KIO::EmptyTrashJob *job = KIO::emptyTrash();
        connect(job, &KJob::result, this, [finish, state, trashSize](KJob *kjob) {
            if (kjob->error() == KJob::NoError) {
                ++state->count;
                state->freed += trashSize;
            }
            finish(kjob->error() != KJob::NoError ? kjob->errorString() : QString());
        });
    }
}

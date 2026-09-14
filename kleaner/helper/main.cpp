// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>

using namespace KAuth;

namespace
{
qulonglong pathSize(const QString &path)
{
    const QFileInfo info(path);
    if (info.isSymLink()) {
        // The link itself is removed; following it would misreport the target size.
        return 0;
    }
    if (info.isFile()) {
        return static_cast<qulonglong>(info.size());
    }
    if (!info.isDir()) {
        return 0;
    }

    qulonglong total = 0;
    QDirIterator iterator(path, QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        const QFileInfo entryInfo = iterator.fileInfo();
        if (!entryInfo.isSymLink()) {
            total += static_cast<qulonglong>(entryInfo.size());
        }
    }
    return total;
}

// Removes a tree without ever following symlinked directories. QDirIterator
// does not follow symlinks, so swapping a validated path for a link cannot let
// a recursive delete escape the allow-list.
bool removeTree(const QString &path)
{
    const QFileInfo rootInfo(path);
    if (rootInfo.isSymLink() || !rootInfo.isDir()) {
        return QFile::remove(path);
    }

    QStringList entries;
    QDirIterator iterator(path, QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        iterator.next();
        entries.append(iterator.filePath());
    }

    std::sort(entries.begin(), entries.end(), [](const QString &a, const QString &b) {
        return a.size() > b.size();
    });

    bool ok = true;
    for (const QString &entry : std::as_const(entries)) {
        const QFileInfo info(entry);
        if (info.isDir() && !info.isSymLink()) {
            ok = QDir().rmdir(entry) && ok;
        } else {
            ok = QFile::remove(entry) && ok;
        }
    }

    return QDir().rmdir(path) && ok;
}

qulonglong unitFactor(const QString &unit, bool binary)
{
    if (unit.isEmpty()) {
        return 1;
    }
    const QChar prefix = unit.at(0).toUpper();
    const qulonglong base = binary || unit.contains(QLatin1Char('i'), Qt::CaseInsensitive) ? 1024ULL : 1000ULL;
    const int power = prefix == QLatin1Char('K') ? 1
                    : prefix == QLatin1Char('M') ? 2
                    : prefix == QLatin1Char('G') ? 3
                    : prefix == QLatin1Char('T') ? 4
                                                 : 0;
    qulonglong factor = 1;
    for (int i = 0; i < power; ++i) {
        factor *= base;
    }
    return factor;
}

qulonglong parseByteSize(const QString &number, const QString &unit, bool binary)
{
    QString normalized = number;
    normalized.replace(QLatin1Char(','), QLatin1Char('.'));
    bool ok = false;
    const double value = normalized.toDouble(&ok);
    if (!ok) {
        return 0;
    }
    return static_cast<qulonglong>(value * unitFactor(unit, binary));
}

qulonglong parsePacmanFreed(const QString &output)
{
    static const QRegularExpression re(QStringLiteral("Total Removed Size:\\s*([0-9.,]+)\\s*([KMGT]?i?B)"));
    const QRegularExpressionMatch match = re.match(output);
    if (!match.hasMatch()) {
        return 0;
    }
    return parseByteSize(match.captured(1), match.captured(2), false);
}

qulonglong parseJournalFreed(const QString &output)
{
    static const QRegularExpression re(QStringLiteral("freed\\s+([0-9.,]+)\\s*([KMGT]?)(i?B)?"));
    const QRegularExpressionMatch match = re.match(output);
    if (!match.hasMatch()) {
        return 0;
    }
    // journalctl prints 1024-based sizes even without the trailing "i".
    return parseByteSize(match.captured(1), match.captured(2), true);
}

QProcessEnvironment cleanEnvironment()
{
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    return environment;
}
}

class KleanerHelper : public QObject
{
    Q_OBJECT

  public Q_SLOTS:
    ActionReply clean(const QVariantMap &args);
    ActionReply writehosts(const QVariantMap &args);
};

static ActionReply errorReply(const QString &message, const QVariantMap &data = {})
{
    ActionReply reply(ActionReply::HelperErrorType);
    reply.setErrorDescription(message);
    if (!data.isEmpty()) {
        reply.setData(data);
    }
    return reply;
}

// Performs the complete privileged cleanup in a single call so that the user
// authenticates only once: file removal, orphan packages and journal vacuum.
ActionReply KleanerHelper::clean(const QVariantMap &args)
{
    static const QStringList allowedPrefixes = {
        QStringLiteral("/var/log/"),
        QStringLiteral("/var/crash/"),
        QStringLiteral("/var/lib/systemd/coredump/"),
        QStringLiteral("/var/tmp/"),
        QStringLiteral("/var/cache/"),
        QStringLiteral("/tmp/"),
    };

    int removed = 0;
    qulonglong freed = 0;
    const auto partialData = [&removed, &freed] {
        return QVariantMap {
            { QStringLiteral("removed"), removed },
            { QStringLiteral("freed"), freed },
        };
    };

    const QStringList paths = args.value(QStringLiteral("paths")).toStringList();
    for (const QString &rawPath : paths) {
        const QString path = QDir::cleanPath(rawPath);

        if (!path.startsWith(QLatin1Char('/')) || path.contains(QLatin1String("/../"))) {
            continue;
        }

        bool allowed = false;
        for (const QString &prefix : allowedPrefixes) {
            if (path.startsWith(prefix)) {
                allowed = true;
                break;
            }
        }
        if (!allowed) {
            continue;
        }

        const QFileInfo info(path);

        // Re-validate the resolved path: if it points outside the allow-list,
        // the request is stale or hostile and must not be removed as root.
        const QString canonical = info.canonicalFilePath();
        if (!canonical.isEmpty()) {
            bool canonicalAllowed = false;
            for (const QString &prefix : allowedPrefixes) {
                if (canonical.startsWith(prefix)) {
                    canonicalAllowed = true;
                    break;
                }
            }
            if (!canonicalAllowed) {
                continue;
            }
        }

        const qulonglong size = pathSize(path);
        bool success = false;
        if (info.isDir() && !info.isSymLink()) {
            success = removeTree(path);
        } else if (info.exists() || info.isSymLink()) {
            success = QFile::remove(path);
        }
        if (success) {
            ++removed;
            freed += size;
        }
    }

    const QStringList orphanPackages = args.value(QStringLiteral("orphanPackages")).toStringList();
    if (!orphanPackages.isEmpty()) {
        static const QRegularExpression validName(QStringLiteral("^[A-Za-z0-9@._+][A-Za-z0-9@._+-]*$"));

        const QString pacman = QStandardPaths::findExecutable(QStringLiteral("pacman"), { QStringLiteral("/usr/bin"), QStringLiteral("/usr/local/bin") });
        if (pacman.isEmpty()) {
            return errorReply(QStringLiteral("pacman was not found on this system."), partialData());
        }

        // Only packages that are still real orphans are removed: the client
        // supplied list is not trusted on its own.
        QProcess queryProcess;
        queryProcess.setProcessEnvironment(cleanEnvironment());
        queryProcess.start(pacman, { QStringLiteral("-Qdtq") });
        if (!queryProcess.waitForFinished(30000)) {
            return errorReply(QStringLiteral("Timed out while querying orphan packages."), partialData());
        }
        if (queryProcess.exitStatus() != QProcess::NormalExit || queryProcess.exitCode() != 0) {
            const QString output = QString::fromLocal8Bit(queryProcess.readAllStandardError()).trimmed();
            return errorReply(output.isEmpty() ? QStringLiteral("Failed to query orphan packages.") : output, partialData());
        }

        QSet<QString> actualOrphans;
        const QStringList orphanNames = QString::fromLocal8Bit(queryProcess.readAllStandardOutput()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString &name : orphanNames) {
            actualOrphans.insert(name.trimmed());
        }

        QStringList packages;
        for (const QString &package : orphanPackages) {
            const QString name = package.trimmed();
            if (validName.match(name).hasMatch() && actualOrphans.contains(name)) {
                packages.append(name);
            }
        }

        if (!packages.isEmpty()) {
            QStringList arguments = { QStringLiteral("-Rns"), QStringLiteral("--noconfirm") };
            arguments += packages;

            QProcess removeProcess;
            removeProcess.setProcessEnvironment(cleanEnvironment());
            removeProcess.start(pacman, arguments);
            if (!removeProcess.waitForFinished(120000)) {
                return errorReply(QStringLiteral("Timed out while removing orphan packages."), partialData());
            }
            const QString standardOutput = QString::fromLocal8Bit(removeProcess.readAllStandardOutput());
            if (removeProcess.exitStatus() != QProcess::NormalExit || removeProcess.exitCode() != 0) {
                const QString output = QString::fromLocal8Bit(removeProcess.readAllStandardError()).trimmed();
                return errorReply(output.isEmpty() ? QStringLiteral("Failed to remove orphan packages.") : output, partialData());
            }

            removed += packages.size();
            freed += parsePacmanFreed(standardOutput);
        }
    }

    if (args.value(QStringLiteral("vacuumJournal")).toBool()) {
        const QString journalctl = QStandardPaths::findExecutable(QStringLiteral("journalctl"), { QStringLiteral("/usr/bin"), QStringLiteral("/usr/local/bin") });
        if (journalctl.isEmpty()) {
            return errorReply(QStringLiteral("journalctl was not found on this system."), partialData());
        }

        // Rotate first so the currently active journal files become archived,
        // then drop the whole archive: the cleanup is meant to remove all
        // journal logs, not just shrink them.
        const QList<QStringList> commands = {
            { QStringLiteral("--rotate") },
            { QStringLiteral("--vacuum-time=1s") },
        };

        qulonglong journalFreed = 0;
        for (const QStringList &arguments : commands) {
            QProcess process;
            process.setProcessEnvironment(cleanEnvironment());
            process.start(journalctl, arguments);
            if (!process.waitForFinished(120000)) {
                return errorReply(QStringLiteral("Timed out while vacuuming the systemd journal."), partialData());
            }
            const QString standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
            const QString standardError = QString::fromLocal8Bit(process.readAllStandardError());
            if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
                const QString output = standardError.trimmed();
                return errorReply(output.isEmpty() ? QStringLiteral("Failed to vacuum the systemd journal.") : output, partialData());
            }
            if (arguments.constLast().startsWith(QLatin1String("--vacuum"))) {
                journalFreed += parseJournalFreed(standardOutput + standardError);
            }
        }

        ++removed;
        freed += journalFreed;
    }

    ActionReply reply;
    reply.setData(partialData());
    return reply;
}

ActionReply KleanerHelper::writehosts(const QVariantMap &args)
{
    const QByteArray content = args.value(QStringLiteral("content")).toByteArray();

    if (content.size() > 1024 * 1024 || content.contains('\0')) {
        return errorReply(QStringLiteral("Invalid content for /etc/hosts."));
    }

    QSaveFile file(QStringLiteral("/etc/hosts"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return errorReply(file.errorString());
    }
    if (file.write(content) != content.size()) {
        file.cancelWriting();
        return errorReply(QStringLiteral("Failed to write /etc/hosts."));
    }
    if (!file.commit()) {
        return errorReply(file.errorString());
    }

    ActionReply reply;
    reply.setData(QVariantMap { { QStringLiteral("written"), content.size() } });
    return reply;
}

KAUTH_HELPER_MAIN(KLEANER_HELPER_ID, KleanerHelper)

#include "main.moc"

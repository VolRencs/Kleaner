// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>

using namespace KAuth;

class KleanerHelper : public QObject
{
    Q_OBJECT

  public Q_SLOTS:
    ActionReply clean(const QVariantMap &args);
    ActionReply writehosts(const QVariantMap &args);
    ActionReply removeorphans(const QVariantMap &args);
    ActionReply vacuumjournal(const QVariantMap &args);
};

static ActionReply errorReply(const QString &message)
{
    ActionReply reply(ActionReply::HelperErrorType);
    reply.setErrorDescription(message);
    return reply;
}

ActionReply KleanerHelper::clean(const QVariantMap &args)
{
    static const QStringList allowedPrefixes = {
        QStringLiteral("/var/log/"),
        QStringLiteral("/var/crash/"),
        QStringLiteral("/var/lib/systemd/coredump/"),
        QStringLiteral("/var/tmp/"),
        QStringLiteral("/var/cache/"),
    };

    const QStringList paths = args.value(QStringLiteral("paths")).toStringList();

    int removed = 0;
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
        bool success = false;
        if (info.isDir() && !info.isSymLink()) {
            success = QDir(path).removeRecursively();
        } else if (info.exists() || info.isSymLink()) {
            success = QFile::remove(path);
        }
        if (success) {
            ++removed;
        }
    }

    ActionReply reply;
    reply.setData(QVariantMap { { QStringLiteral("removed"), removed } });
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

ActionReply KleanerHelper::removeorphans(const QVariantMap &args)
{
    static const QRegularExpression validName(QStringLiteral("^[A-Za-z0-9@._+][A-Za-z0-9@._+-]*$"));

    QStringList packages;
    const QStringList requested = args.value(QStringLiteral("packages")).toStringList();
    for (const QString &package : requested) {
        const QString name = package.trimmed();
        if (validName.match(name).hasMatch()) {
            packages.append(name);
        }
    }

    ActionReply reply;
    if (packages.isEmpty()) {
        reply.setData(QVariantMap { { QStringLiteral("removed"), 0 } });
        return reply;
    }

    const QString pacman = QStandardPaths::findExecutable(QStringLiteral("pacman"), { QStringLiteral("/usr/bin"), QStringLiteral("/usr/local/bin") });
    if (pacman.isEmpty()) {
        return errorReply(QStringLiteral("pacman was not found on this system."));
    }

    QStringList arguments = { QStringLiteral("-Rns"), QStringLiteral("--noconfirm") };
    arguments += packages;

    QProcess removeProcess;
    removeProcess.start(pacman, arguments);
    if (!removeProcess.waitForFinished(120000)) {
        return errorReply(QStringLiteral("Timed out while removing orphan packages."));
    }
    if (removeProcess.exitStatus() != QProcess::NormalExit || removeProcess.exitCode() != 0) {
        const QString output = QString::fromLocal8Bit(removeProcess.readAllStandardError()).trimmed();
        return errorReply(output.isEmpty() ? QStringLiteral("Failed to remove orphan packages.") : output);
    }

    reply.setData(QVariantMap { { QStringLiteral("removed"), packages.size() } });
    return reply;
}

ActionReply KleanerHelper::vacuumjournal(const QVariantMap &args)
{
    Q_UNUSED(args)

    const QString journalctl = QStandardPaths::findExecutable(QStringLiteral("journalctl"), { QStringLiteral("/usr/bin"), QStringLiteral("/usr/local/bin") });
    if (journalctl.isEmpty()) {
        return errorReply(QStringLiteral("journalctl was not found on this system."));
    }

    QProcess process;
    process.start(journalctl, { QStringLiteral("--vacuum-size=50M"), QStringLiteral("--quiet") });
    if (!process.waitForFinished(120000)) {
        return errorReply(QStringLiteral("Timed out while vacuuming the systemd journal."));
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        const QString output = QString::fromLocal8Bit(process.readAllStandardError()).trimmed();
        return errorReply(output.isEmpty() ? QStringLiteral("Failed to vacuum the systemd journal.") : output);
    }

    ActionReply reply;
    reply.setData(QVariantMap { { QStringLiteral("vacuumed"), true } });
    return reply;
}

KAUTH_HELPER_MAIN(KLEANER_HELPER_ID, KleanerHelper)

#include "main.moc"

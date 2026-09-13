#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>

#include <QDir>
#include <QFileInfo>
#include <QSaveFile>

using namespace KAuth;

class KleanerHelper : public QObject
{
    Q_OBJECT

  public Q_SLOTS:
    ActionReply clean(const QVariantMap &args);
    ActionReply writehosts(const QVariantMap &args);
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

KAUTH_HELPER_MAIN(KLEANER_HELPER_ID, KleanerHelper)

#include "main.moc"

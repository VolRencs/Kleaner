#include "startup_apps.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextStream>
#include <QVariantMap>

#include <algorithm>

namespace
{
QString desktopValue(const QHash<QString, QString> &values, const QString &key)
{
    const QString language = QLocale::system().name();
    const QString shortLanguage = language.section(QLatin1Char('_'), 0, 0);

    const QString localized = values.value(key + QLatin1Char('[') + language + QLatin1Char(']'));
    if (!localized.isEmpty()) {
        return localized;
    }
    const QString shortLocalized = values.value(key + QLatin1Char('[') + shortLanguage + QLatin1Char(']'));
    if (!shortLocalized.isEmpty()) {
        return shortLocalized;
    }
    return values.value(key);
}

QHash<QString, QString> readDesktopEntry(const QString &path)
{
    QHash<QString, QString> values;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return values;
    }

    bool desktopEntrySection = false;
    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }
        if (line.startsWith(QLatin1Char('['))) {
            desktopEntrySection = line == QLatin1String("[Desktop Entry]");
            continue;
        }
        if (!desktopEntrySection) {
            continue;
        }
        const int separator = line.indexOf(QLatin1Char('='));
        if (separator <= 0) {
            continue;
        }
        values.insert(line.left(separator).trimmed(), line.mid(separator + 1));
    }

    return values;
}
}

StartupApps::StartupApps(QObject *parent) :
    QObject(parent)
{
}

QString StartupApps::userAutostartDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QStringLiteral("/autostart");
}

StartupApp StartupApps::readEntry(const QString &path, bool system)
{
    StartupApp app;
    app.path = path;
    app.system = system;

    const QHash<QString, QString> values = readDesktopEntry(path);
    app.name = desktopValue(values, QStringLiteral("Name"));
    app.comment = desktopValue(values, QStringLiteral("Comment"));
    app.exec = desktopValue(values, QStringLiteral("Exec"));
    app.icon = desktopValue(values, QStringLiteral("Icon"));

    if (app.name.isEmpty()) {
        app.name = QFileInfo(path).completeBaseName();
    }

    const QString hidden = values.value(QStringLiteral("Hidden")).trimmed().toLower();
    const QString autostartEnabled = values.value(QStringLiteral("X-GNOME-Autostart-enabled")).trimmed().toLower();
    app.enabled = hidden != QLatin1String("true") && autostartEnabled != QLatin1String("false");

    return app;
}

QVariantList StartupApps::load() const
{
    QHash<QString, StartupApp> apps;

    const QDir systemDir(QStringLiteral("/etc/xdg/autostart"));
    const QStringList systemFiles = systemDir.entryList({ QStringLiteral("*.desktop") }, QDir::Files);
    for (const QString &fileName : systemFiles) {
        const StartupApp app = readEntry(systemDir.filePath(fileName), true);
        apps.insert(fileName, app);
    }

    const QDir userDir(userAutostartDir());
    const QStringList userFiles = userDir.entryList({ QStringLiteral("*.desktop") }, QDir::Files);
    for (const QString &fileName : userFiles) {
        const StartupApp app = readEntry(userDir.filePath(fileName), false);
        apps.insert(fileName, app);
    }

    QList<StartupApp> sorted = apps.values();
    std::sort(sorted.begin(), sorted.end(), [](const StartupApp &a, const StartupApp &b) {
        return QString::localeAwareCompare(a.name, b.name) < 0;
    });

    QVariantList result;
    result.reserve(sorted.size());
    for (const StartupApp &app : sorted) {
        result.append(QVariantMap {
            { QStringLiteral("path"), app.path },
            { QStringLiteral("name"), app.name },
            { QStringLiteral("comment"), app.comment },
            { QStringLiteral("exec"), app.exec },
            { QStringLiteral("icon"), app.icon },
            { QStringLiteral("enabled"), app.enabled },
            { QStringLiteral("system"), app.system },
        });
    }

    return result;
}

bool StartupApps::writeKey(const QString &path, const QString &key, const QString &value)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
    file.close();

    QStringList output;
    output.reserve(lines.size() + 1);

    bool inDesktopEntry = false;
    bool replaced = false;
    for (const QString &rawLine : lines) {
        QString line = rawLine;
        const QString trimmed = line.trimmed();

        if (trimmed.startsWith(QLatin1Char('['))) {
            inDesktopEntry = trimmed == QLatin1String("[Desktop Entry]");
        }

        if (inDesktopEntry && !replaced && trimmed.startsWith(key + QLatin1Char('='))) {
            line = key + QLatin1Char('=') + value;
            replaced = true;
        }
        output.append(line);
    }

    if (!replaced) {
        if (output.isEmpty() || output.last().trimmed().isEmpty()) {
            output.append(QStringLiteral("[Desktop Entry]"));
        }
        output.append(key + QLatin1Char('=') + value);
    }

    QFile outputFile(path);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return false;
    }
    outputFile.write(output.join(QLatin1Char('\n')).toUtf8());
    return true;
}

bool StartupApps::setEnabled(const QString &path, bool enabled)
{
    QString target = path;
    const QString fileName = QFileInfo(path).fileName();

    const bool isSystem = path.startsWith(QLatin1String("/etc/xdg/autostart/"));
    if (isSystem) {
        QDir().mkpath(userAutostartDir());
        const QString userPath = QDir(userAutostartDir()).filePath(fileName);
        if (!QFile::exists(userPath) && !QFile::copy(path, userPath)) {
            return false;
        }
        target = userPath;
    }

    if (!writeKey(target, QStringLiteral("Hidden"), enabled ? QStringLiteral("false") : QStringLiteral("true"))) {
        return false;
    }

    if (enabled) {
        writeKey(target, QStringLiteral("X-GNOME-Autostart-enabled"), QStringLiteral("true"));
    }

    Q_EMIT changed();
    return true;
}

bool StartupApps::save(const QString &path, const QString &name, const QString &comment, const QString &exec, const QString &icon)
{
    if (path.startsWith(QLatin1String("/etc/xdg/autostart/"))) {
        return false;
    }

    bool ok = writeKey(path, QStringLiteral("Name"), name);
    ok = writeKey(path, QStringLiteral("Comment"), comment) && ok;
    ok = writeKey(path, QStringLiteral("Exec"), exec) && ok;
    ok = writeKey(path, QStringLiteral("Icon"), icon) && ok;

    if (ok) {
        Q_EMIT changed();
    }
    return ok;
}

QString StartupApps::create(const QString &name, const QString &comment, const QString &exec, const QString &icon)
{
    QDir().mkpath(userAutostartDir());

    QString slug = name.trimmed().toLower().replace(QRegularExpression(QStringLiteral("[^a-z0-9]+")), QStringLiteral("-"));
    slug.remove(QRegularExpression(QStringLiteral("^-+|-+$")));
    if (slug.isEmpty()) {
        slug = QStringLiteral("startup-app");
    }

    QString fileName = slug + QStringLiteral(".desktop");
    int counter = 1;
    while (QFile::exists(QDir(userAutostartDir()).filePath(fileName))) {
        fileName = slug + QStringLiteral("-") + QString::number(counter++) + QStringLiteral(".desktop");
    }

    const QString path = QDir(userAutostartDir()).filePath(fileName);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    stream << "[Desktop Entry]\n";
    stream << "Type=Application\n";
    stream << "Name=" << name << "\n";
    if (!comment.isEmpty()) {
        stream << "Comment=" << comment << "\n";
    }
    stream << "Exec=" << exec << "\n";
    if (!icon.isEmpty()) {
        stream << "Icon=" << icon << "\n";
    }
    stream << "Terminal=false\n";
    stream << "Hidden=false\n";
    file.close();

    Q_EMIT changed();
    return path;
}

bool StartupApps::remove(const QString &path)
{
    if (path.startsWith(QLatin1String("/etc/xdg/autostart/"))) {
        return false;
    }

    if (!QFile::remove(path)) {
        return false;
    }

    Q_EMIT changed();
    return true;
}

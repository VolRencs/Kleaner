// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "startup_apps.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>
#include <QTextStream>
#include <QVariantMap>

#include <algorithm>

namespace
{
QString sanitizeValue(const QString &value)
{
    QString result = value;
    result.remove(QLatin1Char('\r'));
    result.remove(QLatin1Char('\n'));
    return result.trimmed();
}

bool isSystemAutostartPath(const QString &path)
{
    return QDir::cleanPath(path).startsWith(QStringLiteral("/etc/xdg/autostart/"));
}

const QRegularExpression &slugPattern()
{
    static const QRegularExpression pattern(QStringLiteral("[^a-z0-9]+"));
    return pattern;
}

const QRegularExpression &slugTrimPattern()
{
    static const QRegularExpression pattern(QStringLiteral("^-+|-+$"));
    return pattern;
}

bool writeKeys(const QString &path, const QHash<QString, QString> &updates)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
    file.close();

    QStringList output;
    output.reserve(lines.size() + updates.size());

    bool inDesktopEntry = false;
    int desktopEntryHeader = -1;
    int desktopEntryEnd = -1;
    QSet<QString> replaced;
    for (const QString &rawLine : lines) {
        const QString trimmed = rawLine.trimmed();

        if (trimmed.startsWith(QLatin1Char('['))) {
            if (inDesktopEntry && desktopEntryEnd < 0) {
                desktopEntryEnd = output.size();
            }
            inDesktopEntry = trimmed == QLatin1String("[Desktop Entry]");
            if (inDesktopEntry) {
                desktopEntryHeader = output.size();
            }
        }

        if (inDesktopEntry) {
            const int separator = trimmed.indexOf(QLatin1Char('='));
            if (separator > 0) {
                const QString key = trimmed.left(separator).trimmed();
                if (updates.contains(key)) {
                    output.append(key + QLatin1Char('=') + updates.value(key));
                    replaced.insert(key);
                    continue;
                }
            }
        }
        output.append(rawLine);
    }
    if (inDesktopEntry) {
        desktopEntryEnd = output.size();
    }

    QStringList missing;
    for (auto it = updates.constBegin(); it != updates.constEnd(); ++it) {
        if (!replaced.contains(it.key())) {
            missing.append(it.key() + QLatin1Char('=') + it.value());
        }
    }
    std::sort(missing.begin(), missing.end());

    if (!missing.isEmpty()) {
        if (desktopEntryHeader >= 0) {
            // Add the keys at the end of the existing [Desktop Entry] group so
            // they cannot end up inside a later [Desktop Action ...] group.
            int insertAt = desktopEntryEnd >= 0 ? desktopEntryEnd : output.size();
            while (insertAt > desktopEntryHeader + 1 && output.at(insertAt - 1).trimmed().isEmpty()) {
                --insertAt;
            }
            for (int i = 0; i < missing.size(); ++i) {
                output.insert(insertAt + i, missing.at(i));
            }
        } else {
            if (!output.isEmpty() && !output.last().trimmed().isEmpty()) {
                output.append(QString());
            }
            output.append(QStringLiteral("[Desktop Entry]"));
            output.append(missing);
        }
    }

    QSaveFile outputFile(path);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    const QByteArray data = output.join(QLatin1Char('\n')).toUtf8();
    if (outputFile.write(data) != data.size()) {
        outputFile.cancelWriting();
        return false;
    }
    return outputFile.commit();
}

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

bool StartupApps::setEnabled(const QString &path, bool enabled)
{
    QString target = path;
    const QString fileName = QFileInfo(path).fileName();

    const bool isSystem = isSystemAutostartPath(path);
    if (isSystem) {
        QDir().mkpath(userAutostartDir());
        const QString userPath = QDir(userAutostartDir()).filePath(fileName);
        if (!QFile::exists(userPath) && !QFile::copy(path, userPath)) {
            return false;
        }
        target = userPath;
    }

    QHash<QString, QString> updates;
    updates.insert(QStringLiteral("Hidden"), enabled ? QStringLiteral("false") : QStringLiteral("true"));
    if (enabled) {
        updates.insert(QStringLiteral("X-GNOME-Autostart-enabled"), QStringLiteral("true"));
    }

    if (!writeKeys(target, updates)) {
        return false;
    }

    Q_EMIT changed();
    return true;
}

bool StartupApps::save(const QString &path, const QString &name, const QString &comment, const QString &exec, const QString &icon)
{
    if (isSystemAutostartPath(path)) {
        return false;
    }

    QHash<QString, QString> updates;
    updates.insert(QStringLiteral("Name"), sanitizeValue(name));
    updates.insert(QStringLiteral("Comment"), sanitizeValue(comment));
    updates.insert(QStringLiteral("Exec"), sanitizeValue(exec));
    updates.insert(QStringLiteral("Icon"), sanitizeValue(icon));

    if (!writeKeys(path, updates)) {
        return false;
    }

    Q_EMIT changed();
    return true;
}

QString StartupApps::create(const QString &name, const QString &comment, const QString &exec, const QString &icon)
{
    const QString directory = userAutostartDir();
    QDir().mkpath(directory);

    QString slug = name.trimmed().toLower().replace(slugPattern(), QStringLiteral("-"));
    slug.remove(slugTrimPattern());
    if (slug.isEmpty()) {
        slug = QStringLiteral("startup-app");
    }

    QFile file;
    QString path;
    for (int counter = 0; counter < 1000; ++counter) {
        QString fileName;
        if (counter == 0) {
            fileName = slug + QStringLiteral(".desktop");
        } else {
            fileName = slug + QLatin1Char('-') + QString::number(counter) + QStringLiteral(".desktop");
        }
        const QString candidate = QDir(directory).filePath(fileName);
        // NewOnly atomically claims the name and closes the check-then-create race.
        if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::NewOnly)) {
            path = candidate;
            break;
        }
    }
    if (path.isEmpty()) {
        return {};
    }

    QTextStream stream(&file);
    stream << "[Desktop Entry]\n";
    stream << "Type=Application\n";
    stream << "Name=" << sanitizeValue(name) << "\n";
    const QString cleanComment = sanitizeValue(comment);
    if (!cleanComment.isEmpty()) {
        stream << "Comment=" << cleanComment << "\n";
    }
    stream << "Exec=" << sanitizeValue(exec) << "\n";
    const QString cleanIcon = sanitizeValue(icon);
    if (!cleanIcon.isEmpty()) {
        stream << "Icon=" << cleanIcon << "\n";
    }
    stream << "Terminal=false\n";
    stream << "Hidden=false\n";
    stream.flush();

    const bool writeOk = stream.status() == QTextStream::Ok;
    file.close();
    if (!writeOk || file.error() != QFileDevice::NoError) {
        QFile::remove(path);
        return {};
    }

    Q_EMIT changed();
    return path;
}

bool StartupApps::remove(const QString &path)
{
    if (isSystemAutostartPath(path)) {
        return false;
    }

    if (!QFile::remove(path)) {
        return false;
    }

    Q_EMIT changed();
    return true;
}

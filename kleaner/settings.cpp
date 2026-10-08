// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "settings.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirListing>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QGuiApplication>
#include <QLocale>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>

#include <KConfig>
#include <KSharedConfig>

Settings::Settings(QObject *parent) :
    QObject(parent),
    m_group(KSharedConfig::openConfig(), QStringLiteral("General")),
    m_autoStartWatcher(new QFileSystemWatcher(this))
{
    setupAutoStartWatch();
    const auto watchChanged = [this] {
        setupAutoStartWatch();
        Q_EMIT autoStartChanged();
    };
    connect(m_autoStartWatcher, &QFileSystemWatcher::directoryChanged, this, watchChanged);
    connect(m_autoStartWatcher, &QFileSystemWatcher::fileChanged, this, watchChanged);
}

QString Settings::startPage() const
{
    return m_group.readEntry(QStringLiteral("StartPage"), QStringLiteral("dashboard"));
}

void Settings::setStartPage(const QString &startPage)
{
    if (this->startPage() == startPage) {
        return;
    }
    m_group.writeEntry(QStringLiteral("StartPage"), startPage);
    m_group.sync();
    Q_EMIT changed();
}

QString Settings::closeBehavior() const
{
    return m_group.readEntry(QStringLiteral("CloseBehavior"), QStringLiteral("ask"));
}

void Settings::setCloseBehavior(const QString &closeBehavior)
{
    if (this->closeBehavior() == closeBehavior) {
        return;
    }
    m_group.writeEntry(QStringLiteral("CloseBehavior"), closeBehavior);
    m_group.sync();
    Q_EMIT changed();
}

bool Settings::useTray() const
{
    return m_group.readEntry(QStringLiteral("UseTray"), true);
}

void Settings::setUseTray(bool useTray)
{
    if (this->useTray() == useTray) {
        return;
    }
    m_group.writeEntry(QStringLiteral("UseTray"), useTray);
    m_group.sync();
    Q_EMIT changed();
}

QString Settings::language() const
{
    return m_group.readEntry(QStringLiteral("Language"), QString());
}

void Settings::setLanguage(const QString &language)
{
    if (this->language() == language) {
        return;
    }
    m_group.writeEntry(QStringLiteral("Language"), language);
    m_group.sync();
    Q_EMIT languageChanged();
}

int Settings::windowWidth() const
{
    return m_group.readEntry(QStringLiteral("WindowWidth"), 1240);
}

void Settings::setWindowWidth(int width)
{
    if (this->windowWidth() == width) {
        return;
    }
    m_group.writeEntry(QStringLiteral("WindowWidth"), width);
    Q_EMIT changed();
}

int Settings::windowHeight() const
{
    return m_group.readEntry(QStringLiteral("WindowHeight"), 800);
}

void Settings::setWindowHeight(int height)
{
    if (this->windowHeight() == height) {
        return;
    }
    m_group.writeEntry(QStringLiteral("WindowHeight"), height);
    Q_EMIT changed();
}

bool Settings::autoStart() const
{
    const QString path = autoStartFilePath();
    if (!QFileInfo::exists(path)) {
        return false;
    }

    const KConfig entry(path, KConfig::SimpleConfig);
    const KConfigGroup group(&entry, QStringLiteral("Desktop Entry"));
    return !group.readEntry(QStringLiteral("Hidden"), false)
           && group.readEntry(QStringLiteral("X-GNOME-Autostart-enabled"), true);
}

void Settings::setAutoStart(bool enabled)
{
    if (autoStart() == enabled) {
        return;
    }

    const QString path = autoStartFilePath();
    if (enabled) {
        QDir().mkpath(QFileInfo(path).absolutePath());
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            file.write(autoStartFileContent());
        }
    } else {
        QFile::remove(path);
    }

    setupAutoStartWatch();
    Q_EMIT autoStartChanged();
}

QString Settings::autoStartFilePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
           + QStringLiteral("/autostart/" KLEANER_DESKTOP_NAME ".desktop");
}

QByteArray Settings::autoStartFileContent()
{
    QString command = QCoreApplication::applicationFilePath();
    if (command.contains(QLatin1Char(' '))) {
        command = QLatin1Char('"') + command + QLatin1Char('"');
    }

    QString name = QGuiApplication::applicationDisplayName();
    if (name.isEmpty()) {
        name = QCoreApplication::applicationName();
    }

    QString content;
    content += QLatin1String("[Desktop Entry]\n");
    content += QLatin1String("Type=Application\n");
    content += QLatin1String("Name=") + name + QLatin1Char('\n');
    content += QLatin1String("Exec=") + command + QLatin1String(" --hidden\n");
    content += QStringLiteral("Icon=" KLEANER_DESKTOP_NAME "\n");
    content += QLatin1String("Terminal=false\n");
    content += QLatin1String("X-GNOME-Autostart-enabled=true\n");
    return content.toUtf8();
}

void Settings::setupAutoStartWatch()
{
    const QString path = autoStartFilePath();
    const QString directory = QFileInfo(path).absolutePath();

    if (QFileInfo::exists(directory) && !m_autoStartWatcher->directories().contains(directory)) {
        m_autoStartWatcher->addPath(directory);
    }

    if (QFileInfo::exists(path)) {
        if (!m_autoStartWatcher->files().contains(path)) {
            m_autoStartWatcher->addPath(path);
        }
    } else if (m_autoStartWatcher->files().contains(path)) {
        m_autoStartWatcher->removePath(path);
    }
}

QVariantList Settings::availableLanguages() const
{
    QVariantList languages;
    QSet<QString> seen;

    const auto appendLanguage = [&languages, &seen](const QString &code) {
        if (code.isEmpty() || seen.contains(code)) {
            return;
        }
        seen.insert(code);

        const QLocale locale(QLocale::codeToLanguage(code));
        // nativeLanguageName() reports "American English" for plain "en";
        // users expect the neutral language name in the selector.
        QString name = locale.language() == QLocale::English
                           ? QLocale::languageToString(QLocale::English)
                           : locale.nativeLanguageName();
        if (name.isEmpty()) {
            name = QLocale::languageToString(locale.language());
        }
        if (name.isEmpty()) {
            name = code;
        }
        const QString displayName = name.left(1).toUpper() + name.mid(1);
        languages.append(QVariantMap { { QStringLiteral("code"), code }, { QStringLiteral("name"), displayName } });
    };

    const QStringList directories = translationDirectories();
    for (const QString &directory : directories) {
        for (const auto &entry : QDirListing(directory, { QStringLiteral("kleaner_*.qm") }, QDirListing::IteratorFlag::FilesOnly)) {
            QString code = entry.fileName();
            code.remove(QStringLiteral("kleaner_"));
            code.chop(3);
            appendLanguage(code);
        }
    }

    // English is the source language and ships no translation catalog, but it
    // must still be selectable when the system locale is something else.
    appendLanguage(QStringLiteral("en"));

    std::ranges::sort(languages, [](const QVariant &a, const QVariant &b) {
        return a.toMap().value(QStringLiteral("name")).toString().localeAwareCompare(b.toMap().value(QStringLiteral("name")).toString()) < 0;
    });

    return languages;
}

QStringList Settings::translationDirectories()
{
    QStringList directories;
    const QString appDir = QCoreApplication::applicationDirPath();

    const QString dataDir = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral(KLEANER_APP_ID "/translations"),
                                                   QStandardPaths::LocateDirectory);
    if (!dataDir.isEmpty()) {
        directories.append(dataDir);
    }
    directories.append(appDir);
    directories.append(appDir + QStringLiteral("/translations"));

    directories.removeDuplicates();
    return directories;
}

void Settings::sync()
{
    m_group.sync();
}

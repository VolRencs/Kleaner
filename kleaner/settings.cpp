// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "settings.h"

#include <QCoreApplication>
#include <QDir>
#include <QLocale>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>

#include <KSharedConfig>

Settings::Settings(QObject *parent) :
    QObject(parent),
    m_group(KSharedConfig::openConfig(QStringLiteral("kleanerrc")), QStringLiteral("General"))
{
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

QVariantList Settings::availableLanguages() const
{
    QVariantList languages;
    QSet<QString> seen;

    const QStringList directories = translationDirectories();
    for (const QString &directory : directories) {
        const QDir dir(directory);
        const QStringList files = dir.entryList({ QStringLiteral("kleaner_*.qm") }, QDir::Files);
        for (const QString &file : files) {
            QString code = file;
            code.remove(QStringLiteral("kleaner_"));
            code.chop(3);

            if (code.isEmpty() || seen.contains(code)) {
                continue;
            }
            seen.insert(code);

            const QLocale locale(QString(code).replace(QLatin1Char('-'), QLatin1Char('_')));
            QString name = locale.nativeLanguageName();
            if (name.isEmpty()) {
                name = QLocale::languageToString(locale.language());
            }
            if (name.isEmpty()) {
                name = code;
            }
            const QString displayName = name.left(1).toUpper() + name.mid(1);
            languages.append(QVariantMap { { QStringLiteral("code"), code }, { QStringLiteral("name"), displayName } });
        }
    }

    std::sort(languages.begin(), languages.end(), [](const QVariant &a, const QVariant &b) {
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
    directories.append(appDir + QStringLiteral("/../translations"));

    directories.removeDuplicates();
    return directories;
}

void Settings::sync()
{
    m_group.sync();
}

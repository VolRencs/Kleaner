#include "settings.h"

#include <KConfigGroup>
#include <KSharedConfig>

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    m_group = new KConfigGroup(config, QStringLiteral("General"));
}

Settings::~Settings()
{
    delete m_group;
}

QString Settings::startPage() const
{
    return m_group->readEntry(QStringLiteral("StartPage"), QStringLiteral("dashboard"));
}

void Settings::setStartPage(const QString &startPage)
{
    if (this->startPage() == startPage) {
        return;
    }
    m_group->writeEntry(QStringLiteral("StartPage"), startPage);
    Q_EMIT changed();
}

QString Settings::closeBehavior() const
{
    return m_group->readEntry(QStringLiteral("CloseBehavior"), QStringLiteral("ask"));
}

void Settings::setCloseBehavior(const QString &closeBehavior)
{
    if (this->closeBehavior() == closeBehavior) {
        return;
    }
    m_group->writeEntry(QStringLiteral("CloseBehavior"), closeBehavior);
    Q_EMIT changed();
}

bool Settings::useTray() const
{
    return m_group->readEntry(QStringLiteral("UseTray"), true);
}

void Settings::setUseTray(bool useTray)
{
    if (this->useTray() == useTray) {
        return;
    }
    m_group->writeEntry(QStringLiteral("UseTray"), useTray);
    Q_EMIT changed();
}

void Settings::sync()
{
    m_group->sync();
}

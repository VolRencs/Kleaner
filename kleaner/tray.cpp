// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "tray.h"

#include <QAction>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QIcon>
#include <QMenu>

#include <KStatusNotifierItem/kstatusnotifieritem.h>

namespace
{
constexpr auto watcherService = "org.kde.StatusNotifierWatcher";
}

Tray::Tray(QObject *parent) :
    QObject(parent)
{
    // The StatusNotifierWatcher may appear or restart after startup; track it
    // instead of freezing the availability at construction time.
    m_watcher = new QDBusServiceWatcher(QString::fromLatin1(watcherService), QDBusConnection::sessionBus(),
                                        QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration, this);
    connect(m_watcher, &QDBusServiceWatcher::serviceRegistered, this, [this] {
        setupItem();
    });
    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, [this] {
        setAvailable(false);
    });

    setupItem();
}

void Tray::setupItem()
{
    QDBusConnectionInterface *busInterface = QDBusConnection::sessionBus().interface();
    setAvailable(busInterface && busInterface->isServiceRegistered(QString::fromLatin1(watcherService)).value());

    if (m_item) {
        m_item->setStatus(m_enabled ? KStatusNotifierItem::Active : KStatusNotifierItem::Passive);
        return;
    }

    // The tray is small: always use the compact vector mark bundled with the app.
    const QIcon icon(QStringLiteral(":/kleaner-k.svg"));

    m_item = new KStatusNotifierItem(this);
    m_item->setIconByPixmap(icon);
    m_item->setTitle(QStringLiteral("Kleaner"));
    m_item->setToolTip(QStringLiteral(KLEANER_DESKTOP_NAME), QStringLiteral("Kleaner"), tr("Linux System Optimizer"));
    m_item->setStandardActionsEnabled(false);
    m_item->setStatus(m_enabled ? KStatusNotifierItem::Active : KStatusNotifierItem::Passive);

    auto *menu = new QMenu();
    QAction *showAction = menu->addAction(tr("Show Kleaner"));
    connect(showAction, &QAction::triggered, this, &Tray::showRequested);

    menu->addSeparator();

    QAction *quitAction = menu->addAction(tr("Quit"));
    connect(quitAction, &QAction::triggered, this, &Tray::quitRequested);

    // The item takes ownership of the menu.
    m_item->setContextMenu(menu);
    connect(m_item, &KStatusNotifierItem::activateRequested, this, [this] {
        Q_EMIT showRequested();
    });
}

bool Tray::available() const
{
    return m_available;
}

void Tray::setAvailable(bool available)
{
    if (m_available == available) {
        return;
    }
    m_available = available;
    Q_EMIT availableChanged();
}

bool Tray::enabled() const
{
    return m_enabled;
}

void Tray::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }
    m_enabled = enabled;

    if (m_item) {
        m_item->setStatus(enabled ? KStatusNotifierItem::Active : KStatusNotifierItem::Passive);
    }

    Q_EMIT enabledChanged();
}

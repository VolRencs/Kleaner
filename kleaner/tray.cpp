// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "tray.h"

#include <QAction>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QIcon>
#include <QMenu>

#include <KStatusNotifierItem/kstatusnotifieritem.h>

Tray::Tray(QObject *parent) :
    QObject(parent)
{
    auto *busInterface = QDBusConnection::sessionBus().interface();
    m_available = busInterface && busInterface->isServiceRegistered(QStringLiteral("org.kde.StatusNotifierWatcher"));

    if (!m_available) {
        return;
    }

    // The tray is small: always use the compact vector mark bundled with the app.
    const QIcon icon(QStringLiteral(":/kleaner-k.svg"));

    m_item = new KStatusNotifierItem(this);
    m_item->setIconByPixmap(icon);
    m_item->setTitle(QStringLiteral("Kleaner"));
    m_item->setToolTip(QStringLiteral("kleaner"), QStringLiteral("Kleaner"), tr("Linux System Optimizer"));
    m_item->setStatus(KStatusNotifierItem::Active);

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

Tray::~Tray()
{
    // KStatusNotifierItem owns its context menu and deletes it on destruction.
    delete m_item;
    m_item = nullptr;
}

bool Tray::available() const
{
    return m_available;
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

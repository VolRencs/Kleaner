#include "tray.h"

#include <QAction>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
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

    m_item = new KStatusNotifierItem(this);
    m_item->setIconByName(QStringLiteral("kleaner"));
    m_item->setTitle(QStringLiteral("Kleaner"));
    m_item->setToolTip(QStringLiteral("kleaner"), QStringLiteral("Kleaner"), tr("Linux System Optimizer"));
    m_item->setStatus(KStatusNotifierItem::Active);

    auto *menu = new QMenu();
    QAction *showAction = menu->addAction(tr("Show Kleaner"));
    connect(showAction, &QAction::triggered, this, &Tray::showRequested);

    menu->addSeparator();

    QAction *quitAction = menu->addAction(tr("Quit"));
    connect(quitAction, &QAction::triggered, this, &Tray::quitRequested);

    m_item->setContextMenu(menu);
    connect(m_item, &KStatusNotifierItem::activateRequested, this, [this] {
        Q_EMIT showRequested();
    });
}

Tray::~Tray()
{
    delete m_item;
}

bool Tray::available() const
{
    return m_available;
}

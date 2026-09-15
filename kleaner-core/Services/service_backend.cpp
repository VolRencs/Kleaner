// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "service_backend.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QFileInfo>
#include <QSet>

#include <algorithm>

namespace
{
constexpr auto systemdService = "org.freedesktop.systemd1";
constexpr auto systemdPath = "/org/freedesktop/systemd1";
constexpr auto systemdManager = "org.freedesktop.systemd1.Manager";

QDBusMessage systemdCall(const QString &method)
{
    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1StringView(systemdService), QLatin1StringView(systemdPath),
                                                          QLatin1StringView(systemdManager), method);
    // Let polkit prompt for authentication instead of failing with "access denied".
    message.setInteractiveAuthorizationAllowed(true);
    return message;
}
}

ServiceBackend::ServiceBackend(QObject *parent) :
    QObject(parent)
{
    m_available = detectSystemd();
}

bool ServiceBackend::detectSystemd()
{
    QDBusConnectionInterface *bus = QDBusConnection::systemBus().interface();
    return bus && bus->isServiceRegistered(QString::fromLatin1(systemdService)).value();
}

void ServiceBackend::refreshAvailability()
{
    const bool available = detectSystemd();
    if (available == m_available) {
        return;
    }
    m_available = available;
    Q_EMIT availableChanged();
}

bool ServiceBackend::available() const
{
    return m_available;
}

void ServiceBackend::reload()
{
    refreshAvailability();

    if (!m_available) {
        Q_EMIT loaded({});
        return;
    }

    const quint64 generation = ++m_generation;
    m_unitFilesLoaded = false;
    m_unitsLoaded = false;
    m_pendingServices.clear();

    auto *filesWatcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(systemdCall(QStringLiteral("ListUnitFiles"))), this);
    connect(filesWatcher, &QDBusPendingCallWatcher::finished, this, [this, generation](QDBusPendingCallWatcher *watcher) {
        handleListUnitFiles(watcher, generation);
    });

    auto *unitsWatcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(systemdCall(QStringLiteral("ListUnits"))), this);
    connect(unitsWatcher, &QDBusPendingCallWatcher::finished, this, [this, generation](QDBusPendingCallWatcher *watcher) {
        handleListUnits(watcher, generation);
    });
}

void ServiceBackend::handleListUnitFiles(QDBusPendingCallWatcher *watcher, quint64 generation)
{
    const QDBusMessage message = watcher->reply();
    watcher->deleteLater();

    if (generation != m_generation) {
        return;
    }

    if (message.type() == QDBusMessage::ErrorMessage) {
        Q_EMIT error(message.errorMessage());
        Q_EMIT loaded({});
        return;
    }

    const QVariantList arguments = message.arguments();
    if (arguments.isEmpty()) {
        m_unitFilesLoaded = true;
        finishReload(generation);
        return;
    }

    const QDBusArgument array = arguments.first().value<QDBusArgument>();
    QSet<QString> seenUnits;
    array.beginArray();
    while (!array.atEnd()) {
        QString path;
        QString state;
        array.beginStructure();
        array >> path >> state;
        array.endStructure();

        // Alias symlinks point at a canonical unit that is listed separately.
        if (state == QLatin1String("alias")) {
            continue;
        }

        // ListUnitFiles reports file paths; systemd methods need the unit name.
        const QString unit = QFileInfo(path).fileName();
        if (!unit.endsWith(QLatin1String(".service")) || unit.contains(QLatin1Char('@')) || seenUnits.contains(unit)) {
            continue;
        }
        seenUnits.insert(unit);

        // ListUnits may already have added this unit (the two D-Bus replies race).
        // Update it in place instead of appending a duplicate.
        const auto existing = std::ranges::find_if(m_pendingServices, [&unit](const QVariant &variant) {
            return variant.toMap().value(QStringLiteral("unit")).toString() == unit;
        });
        if (existing != m_pendingServices.end()) {
            QVariantMap service = existing->toMap();
            service.insert(QStringLiteral("enabled"), state == QLatin1String("enabled") || state == QLatin1String("enabled-runtime"));
            *existing = service;
            continue;
        }

        m_pendingServices.append(QVariantMap {
            { QStringLiteral("name"), unit.chopped(8) },
            { QStringLiteral("unit"), unit },
            { QStringLiteral("description"), QString() },
            { QStringLiteral("enabled"), state == QLatin1String("enabled") || state == QLatin1String("enabled-runtime") },
            { QStringLiteral("active"), false },
            { QStringLiteral("activeState"), QStringLiteral("inactive") },
        });
    }
    array.endArray();

    m_unitFilesLoaded = true;
    finishReload(generation);
}

void ServiceBackend::handleListUnits(QDBusPendingCallWatcher *watcher, quint64 generation)
{
    const QDBusMessage message = watcher->reply();
    watcher->deleteLater();

    if (generation != m_generation) {
        return;
    }

    if (message.type() == QDBusMessage::ErrorMessage) {
        Q_EMIT error(message.errorMessage());
        Q_EMIT loaded({});
        return;
    }

    const QVariantList arguments = message.arguments();
    if (arguments.isEmpty()) {
        m_unitsLoaded = true;
        finishReload(generation);
        return;
    }

    const QDBusArgument array = arguments.first().value<QDBusArgument>();
    array.beginArray();
    while (!array.atEnd()) {
        QString name;
        QString description;
        QString loadState;
        QString activeState;
        QString subState;
        QString following;
        QString unitPath;
        uint jobId = 0;
        QString jobType;
        QString jobPath;

        array.beginStructure();
        array >> name >> description >> loadState >> activeState >> subState >> following >> unitPath >> jobId >> jobType >> jobPath;
        array.endStructure();

        if (!name.endsWith(QLatin1String(".service")) || name.contains(QLatin1Char('@'))) {
            continue;
        }

        // Update the matching entry from ListUnitFiles, or add it if missing (runtime units)
        const auto existing = std::ranges::find_if(m_pendingServices, [&name](const QVariant &variant) {
            return variant.toMap().value(QStringLiteral("unit")).toString() == name;
        });

        if (existing != m_pendingServices.end()) {
            QVariantMap service = existing->toMap();
            service.insert(QStringLiteral("description"), description);
            service.insert(QStringLiteral("active"), activeState == QLatin1String("active") || activeState == QLatin1String("activating"));
            service.insert(QStringLiteral("activeState"), activeState);
            *existing = service;
        } else {
            m_pendingServices.append(QVariantMap {
                { QStringLiteral("name"), name.chopped(8) },
                { QStringLiteral("unit"), name },
                { QStringLiteral("description"), description },
                { QStringLiteral("enabled"), false },
                { QStringLiteral("active"), activeState == QLatin1String("active") || activeState == QLatin1String("activating") },
                { QStringLiteral("activeState"), activeState },
            });
        }
    }
    array.endArray();

    m_unitsLoaded = true;
    finishReload(generation);
}

void ServiceBackend::finishReload(quint64 generation)
{
    if (generation != m_generation || !m_unitFilesLoaded || !m_unitsLoaded) {
        return;
    }

    Q_EMIT loaded(m_pendingServices);
}

void ServiceBackend::setEnabled(const QString &unit, bool enabled)
{
    if (!m_available) {
        Q_EMIT error(tr("systemd is not available on this system."));
        return;
    }

    const QString method = enabled ? QStringLiteral("EnableUnitFiles") : QStringLiteral("DisableUnitFiles");

    QDBusMessage message = systemdCall(method);
    if (enabled) {
        message << QStringList { unit } << false << false;
    } else {
        message << QStringList { unit } << false;
    }

    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        const QDBusMessage message = self->reply();
        self->deleteLater();
        if (message.type() == QDBusMessage::ErrorMessage) {
            Q_EMIT error(message.errorMessage());
        }
        reload();
    });
}

void ServiceBackend::start(const QString &unit)
{
    runUnitMethod(QStringLiteral("StartUnit"), unit);
}

void ServiceBackend::stop(const QString &unit)
{
    runUnitMethod(QStringLiteral("StopUnit"), unit);
}

void ServiceBackend::restart(const QString &unit)
{
    runUnitMethod(QStringLiteral("RestartUnit"), unit);
}

void ServiceBackend::runUnitMethod(const QString &method, const QString &unit)
{
    if (!m_available) {
        Q_EMIT error(tr("systemd is not available on this system."));
        return;
    }

    QDBusMessage message = systemdCall(method);
    message << unit << QStringLiteral("replace");

    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        const QDBusMessage message = self->reply();
        self->deleteLater();
        if (message.type() == QDBusMessage::ErrorMessage) {
            Q_EMIT error(message.errorMessage());
        }
        reload();
    });
}

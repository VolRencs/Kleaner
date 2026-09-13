#include "service_backend.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>

namespace
{
constexpr auto systemdService = "org.freedesktop.systemd1";
constexpr auto systemdPath = "/org/freedesktop/systemd1";
constexpr auto systemdManager = "org.freedesktop.systemd1.Manager";
}

ServiceBackend::ServiceBackend(QObject *parent) :
    QObject(parent)
{
    m_interface = new QDBusInterface(QString::fromLatin1(systemdService), QString::fromLatin1(systemdPath), QString::fromLatin1(systemdManager),
                                     QDBusConnection::systemBus(), this);

    m_available = m_interface->isValid();
    if (!m_available) {
        delete m_interface;
        m_interface = nullptr;
    }
}

ServiceBackend::~ServiceBackend()
{
    delete m_interface;
}

bool ServiceBackend::available() const
{
    return m_available;
}

void ServiceBackend::reload()
{
    if (!m_available) {
        Q_EMIT loaded({});
        return;
    }

    m_unitFilesLoaded = false;
    m_unitsLoaded = false;
    m_pendingServices.clear();

    auto *filesWatcher = new QDBusPendingCallWatcher(m_interface->asyncCall(QStringLiteral("ListUnitFiles")), this);
    connect(filesWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        handleListUnitFiles(watcher);
    });

    auto *unitsWatcher = new QDBusPendingCallWatcher(m_interface->asyncCall(QStringLiteral("ListUnits")), this);
    connect(unitsWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        handleListUnits(watcher);
    });
}

void ServiceBackend::handleListUnitFiles(QDBusPendingCallWatcher *watcher)
{
    const QDBusMessage message = watcher->reply();
    watcher->deleteLater();

    if (message.type() == QDBusMessage::ErrorMessage) {
        Q_EMIT error(message.errorMessage());
        Q_EMIT loaded({});
        return;
    }

    const QVariantList arguments = message.arguments();
    if (arguments.isEmpty()) {
        m_unitFilesLoaded = true;
        finishReload();
        return;
    }

    const QDBusArgument array = arguments.first().value<QDBusArgument>();
    array.beginArray();
    while (!array.atEnd()) {
        QString name;
        QString state;
        array.beginStructure();
        array >> name >> state;
        array.endStructure();

        if (!name.endsWith(QLatin1String(".service")) || name.contains(QLatin1Char('@'))) {
            continue;
        }

        m_pendingServices.append(QVariantMap {
            { QStringLiteral("name"), name.chopped(8) },
            { QStringLiteral("unit"), name },
            { QStringLiteral("description"), QString() },
            { QStringLiteral("enabled"), state == QLatin1String("enabled") || state == QLatin1String("enabled-runtime") },
            { QStringLiteral("active"), false },
            { QStringLiteral("activeState"), QStringLiteral("inactive") },
            { QStringLiteral("unitFileState"), state },
        });
    }
    array.endArray();

    m_unitFilesLoaded = true;
    finishReload();
}

void ServiceBackend::handleListUnits(QDBusPendingCallWatcher *watcher)
{
    const QDBusMessage message = watcher->reply();
    watcher->deleteLater();

    if (message.type() == QDBusMessage::ErrorMessage) {
        Q_EMIT error(message.errorMessage());
        Q_EMIT loaded({});
        return;
    }

    const QVariantList arguments = message.arguments();
    if (arguments.isEmpty()) {
        m_unitsLoaded = true;
        finishReload();
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
        bool found = false;
        for (int i = 0; i < m_pendingServices.size(); ++i) {
            QVariantMap service = m_pendingServices.at(i).toMap();
            if (service.value(QStringLiteral("unit")).toString() == name) {
                service.insert(QStringLiteral("description"), description);
                service.insert(QStringLiteral("active"), activeState == QLatin1String("active") || activeState == QLatin1String("activating"));
                service.insert(QStringLiteral("activeState"), activeState);
                m_pendingServices[i] = service;
                found = true;
                break;
            }
        }

        if (!found) {
            m_pendingServices.append(QVariantMap {
                { QStringLiteral("name"), name.chopped(8) },
                { QStringLiteral("unit"), name },
                { QStringLiteral("description"), description },
                { QStringLiteral("enabled"), false },
                { QStringLiteral("active"), activeState == QLatin1String("active") || activeState == QLatin1String("activating") },
                { QStringLiteral("activeState"), activeState },
                { QStringLiteral("unitFileState"), QString() },
            });
        }
    }
    array.endArray();

    m_unitsLoaded = true;
    finishReload();
}

void ServiceBackend::finishReload()
{
    if (!m_unitFilesLoaded || !m_unitsLoaded) {
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

    QDBusPendingCall call = enabled
        ? m_interface->asyncCall(method, QStringList { unit }, false, false)
        : m_interface->asyncCall(method, QStringList { unit }, false);

    auto *watcher = new QDBusPendingCallWatcher(call, this);
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

    auto *watcher = new QDBusPendingCallWatcher(m_interface->asyncCall(method, unit, QStringLiteral("replace")), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *self) {
        const QDBusMessage message = self->reply();
        self->deleteLater();
        if (message.type() == QDBusMessage::ErrorMessage) {
            Q_EMIT error(message.errorMessage());
        }
        reload();
    });
}

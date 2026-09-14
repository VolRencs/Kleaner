// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "network_info.h"

#include "Utils/procfs.h"

#include <QDir>

#include <limits>

NetworkInfo::NetworkInfo(QObject *parent) :
    QObject(parent)
{
}

QString NetworkInfo::interface() const
{
    return m_interface;
}

double NetworkInfo::rxRate() const
{
    return m_rxRate;
}

double NetworkInfo::txRate() const
{
    return m_txRate;
}

bool NetworkInfo::connected() const
{
    return m_connected;
}

QString NetworkInfo::resolveDefaultInterface() const
{
    const QList<QByteArray> routes = Procfs::lines(QStringLiteral("/proc/net/route"));
    QString gatewayIface;
    quint32 gatewayMetric = std::numeric_limits<quint32>::max();
    QString upIface;
    quint32 upMetric = std::numeric_limits<quint32>::max();

    for (const QByteArray &line : routes) {
        const QList<QByteArray> fields = line.simplified().split(' ');
        if (fields.size() < 8 || fields.at(0) == "Iface") {
            continue;
        }
        if (fields.at(1) != "00000000") {
            continue;
        }

        bool flagsOk = false;
        const quint32 flags = fields.at(3).toUInt(&flagsOk, 16);
        if (!flagsOk || !(flags & 0x0001u)) {
            continue;
        }

        const QString iface = QString::fromLatin1(fields.at(0));
        const QByteArray state = Procfs::read(QStringLiteral("/sys/class/net/%1/operstate").arg(iface)).trimmed();
        if (!state.isEmpty() && state != "up" && state != "unknown") {
            continue;
        }

        bool metricOk = false;
        const quint32 metric = fields.at(6).toUInt(&metricOk, 10);
        const quint32 value = metricOk ? metric : std::numeric_limits<quint32>::max();

        if (flags & 0x0002u) {
            if (value < gatewayMetric) {
                gatewayMetric = value;
                gatewayIface = iface;
            }
        } else if (value < upMetric) {
            upMetric = value;
            upIface = iface;
        }
    }

    return gatewayIface.isEmpty() ? upIface : gatewayIface;
}

QString NetworkInfo::resolveFallbackInterface() const
{
    const QDir netDir(QStringLiteral("/sys/class/net"));
    const QStringList entries = netDir.entryList(QDir::Dirs);
    for (const QString &entry : entries) {
        if (entry == QLatin1String("lo")) {
            continue;
        }
        const QByteArray state = Procfs::read(netDir.filePath(entry + QStringLiteral("/operstate"))).trimmed();
        if (state == "up") {
            return entry;
        }
    }
    return {};
}

void NetworkInfo::update()
{
    QString iface = m_interface;

    bool ifaceValid = !iface.isEmpty() && QDir(QStringLiteral("/sys/class/net/") + iface).exists();
    if (ifaceValid) {
        const QByteArray state = Procfs::read(QStringLiteral("/sys/class/net/%1/operstate").arg(iface)).trimmed();
        if (state == "down") {
            ifaceValid = false;
        }
    }

    if (!ifaceValid) {
        iface = resolveDefaultInterface();
        if (iface.isEmpty()) {
            iface = resolveFallbackInterface();
        }
        if (iface != m_interface) {
            m_interface = iface;
            m_hasBaseline = false;
            m_rxRate = 0.0;
            m_txRate = 0.0;
        }
    }

    if (m_interface.isEmpty()) {
        m_connected = false;
        Q_EMIT changed();
        return;
    }

    bool okRx = false;
    bool okTx = false;
    const qulonglong rx = Procfs::readUInt64(QStringLiteral("/sys/class/net/%1/statistics/rx_bytes").arg(m_interface), &okRx);
    const qulonglong tx = Procfs::readUInt64(QStringLiteral("/sys/class/net/%1/statistics/tx_bytes").arg(m_interface), &okTx);

    m_connected = okRx && okTx;

    if (!m_hasBaseline || rx < m_previousRx || tx < m_previousTx) {
        m_previousRx = rx;
        m_previousTx = tx;
        m_hasBaseline = true;
        m_elapsed.restart();
        m_rxRate = 0.0;
        m_txRate = 0.0;
    } else {
        const qint64 elapsedMs = m_elapsed.elapsed();
        if (elapsedMs > 200) {
            m_rxRate = static_cast<double>(rx - m_previousRx) * 1000.0 / static_cast<double>(elapsedMs);
            m_txRate = static_cast<double>(tx - m_previousTx) * 1000.0 / static_cast<double>(elapsedMs);
            m_previousRx = rx;
            m_previousTx = tx;
            m_elapsed.restart();
        }
    }

    Q_EMIT changed();
}

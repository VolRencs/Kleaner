#include "network_info.h"

#include "Utils/procfs.h"

#include <QDir>

NetworkInfo::NetworkInfo(QObject *parent) :
    QObject(parent)
{
}

QString NetworkInfo::interface() const
{
    return m_interface;
}

qulonglong NetworkInfo::rxBytes() const
{
    return m_rxBytes;
}

qulonglong NetworkInfo::txBytes() const
{
    return m_txBytes;
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
    for (const QByteArray &line : routes) {
        const QList<QByteArray> fields = line.simplified().split(' ');
        if (fields.size() < 2 || fields.at(0) == "Iface") {
            continue;
        }
        if (fields.at(1) == "00000000") {
            const QString iface = QString::fromLatin1(fields.at(0));
            const QByteArray state = Procfs::read(QStringLiteral("/sys/class/net/%1/operstate").arg(iface)).trimmed();
            if (state.isEmpty() || state == "up" || state == "unknown") {
                return iface;
            }
        }
    }
    return {};
}

QString NetworkInfo::resolveFallbackInterface() const
{
    const QDir netDir(QStringLiteral("/sys/class/net"));
    const QStringList entries = netDir.entryList(QDir::Dirs | QDir::NoSymLinks);
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
        m_rxBytes = 0;
        m_txBytes = 0;
        Q_EMIT changed();
        return;
    }

    bool okRx = false;
    bool okTx = false;
    const qulonglong rx = Procfs::readUInt64(QStringLiteral("/sys/class/net/%1/statistics/rx_bytes").arg(m_interface), &okRx);
    const qulonglong tx = Procfs::readUInt64(QStringLiteral("/sys/class/net/%1/statistics/tx_bytes").arg(m_interface), &okTx);

    m_connected = okRx && okTx;
    m_rxBytes = okRx ? rx : 0;
    m_txBytes = okTx ? tx : 0;

    if (!m_hasBaseline || rx < m_previousRx || tx < m_previousTx) {
        m_previousRx = rx;
        m_previousTx = tx;
        m_hasBaseline = true;
        m_elapsed.restart();
        m_rxRate = 0.0;
        m_txRate = 0.0;
    } else {
        const qint64 elapsedMs = m_elapsed.isValid() ? m_elapsed.elapsed() : 0;
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

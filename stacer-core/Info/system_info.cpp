#include "system_info.h"

#include "Utils/helpers.h"
#include "Utils/procfs.h"

#include <QRegularExpression>
#include <QSysInfo>

#include <unistd.h>

SystemInfo::SystemInfo(QObject *parent) :
    QObject(parent)
{
    m_hostname = QSysInfo::machineHostName();
    m_platform = QSysInfo::prettyProductName();
    m_kernel = QSysInfo::kernelVersion();
    m_username = Helpers::userName(getuid());

    readOsRelease();
    readCpuModel();
    update();
}

QString SystemInfo::hostname() const
{
    return m_hostname;
}

QString SystemInfo::platform() const
{
    return m_platform;
}

QString SystemInfo::distribution() const
{
    return m_distribution;
}

QString SystemInfo::distributionId() const
{
    return m_distributionId;
}

QString SystemInfo::kernel() const
{
    return m_kernel;
}

QString SystemInfo::cpuModel() const
{
    return m_cpuModel;
}

QString SystemInfo::username() const
{
    return m_username;
}

qulonglong SystemInfo::uptimeSeconds() const
{
    return m_uptimeSeconds;
}

void SystemInfo::update()
{
    bool ok = false;
    const quint64 uptime = Procfs::readUInt64(QStringLiteral("/proc/uptime"), &ok);
    if (ok) {
        m_uptimeSeconds = uptime;
        Q_EMIT changed();
    }
}

void SystemInfo::readOsRelease()
{
    const QList<QByteArray> lines = Procfs::lines(QStringLiteral("/etc/os-release"));
    for (const QByteArray &line : lines) {
        const int separator = line.indexOf('=');
        if (separator < 0) {
            continue;
        }
        const QString key = QString::fromLatin1(line.left(separator));
        QString value = QString::fromUtf8(line.mid(separator + 1)).trimmed();
        if (value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"')) && value.size() >= 2) {
            value = value.mid(1, value.size() - 2);
        }

        if (key == QLatin1String("PRETTY_NAME")) {
            m_distribution = value;
        } else if (key == QLatin1String("NAME") && m_distribution.isEmpty()) {
            m_distribution = value;
        } else if (key == QLatin1String("ID")) {
            m_distributionId = value;
        }
    }
}

void SystemInfo::readCpuModel()
{
    const QList<QByteArray> lines = Procfs::lines(QStringLiteral("/proc/cpuinfo"));
    static const QRegularExpression re(QStringLiteral("^(model name|Processor|Hardware|cpu model)\\s*:\\s*(.+)$"),
                                       QRegularExpression::CaseInsensitiveOption);
    for (const QByteArray &line : lines) {
        const QRegularExpressionMatch match = re.match(QString::fromLatin1(line));
        if (match.hasMatch()) {
            QString model = match.captured(2).trimmed();
            model.remove(QRegularExpression(QStringLiteral("\\s*@\\s*[0-9.]+\\s*GHz$")));
            m_cpuModel = model;
            return;
        }
    }
}

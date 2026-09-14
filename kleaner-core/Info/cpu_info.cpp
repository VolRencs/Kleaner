// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "cpu_info.h"

#include "Utils/procfs.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>

CpuInfo::CpuInfo(QObject *parent) :
    QObject(parent)
{
}

int CpuInfo::coreCount() const
{
    return m_coreCount;
}

double CpuInfo::usage() const
{
    return m_usage;
}

QVariantList CpuInfo::coreUsages() const
{
    QVariantList list;
    list.reserve(m_coreUsages.size());
    for (const double value : m_coreUsages) {
        list.append(value);
    }
    return list;
}

double CpuInfo::load1() const
{
    return m_load1;
}

double CpuInfo::load5() const
{
    return m_load5;
}

double CpuInfo::load15() const
{
    return m_load15;
}

double CpuInfo::clock() const
{
    return m_clock;
}

void CpuInfo::update()
{
    const QList<QByteArray> lines = Procfs::lines(QStringLiteral("/proc/stat"));

    QList<QList<quint64>> current;
    for (const QByteArray &line : lines) {
        if (!line.startsWith("cpu")) {
            continue;
        }
        const QList<QByteArray> fields = line.simplified().split(' ');
        if (fields.size() < 5) {
            continue;
        }
        QList<quint64> values;
        values.reserve(fields.size() - 1);
        for (int i = 1; i < fields.size(); ++i) {
            values.append(fields.at(i).toULongLong());
        }
        current.append(values);
    }

    if (current.isEmpty()) {
        return;
    }

    const int count = current.size();

    if (!m_hasBaseline || m_previousTotals.size() != count) {
        m_previousTotals.clear();
        m_previousIdles.clear();
        for (const QList<quint64> &values : current) {
            quint64 total = 0;
            for (const quint64 value : values) {
                total += value;
            }
            m_previousTotals.append(total);
            m_previousIdles.append(values.value(3) + values.value(4));
        }
        m_hasBaseline = true;
        m_coreCount = qMax(1, count - 1);
        m_coreUsages = QList<double>(m_coreCount, 0.0);
        m_usage = 0.0;
        updateLoads();
        updateClocks();
        Q_EMIT changed();
        return;
    }

    m_coreUsages.clear();
    m_coreUsages.reserve(count - 1);

    for (int i = 0; i < count; ++i) {
        const QList<quint64> &values = current.at(i);

        quint64 total = 0;
        for (const quint64 value : values) {
            total += value;
        }
        const quint64 idle = values.value(3) + values.value(4);

        const quint64 deltaTotal = total >= m_previousTotals.at(i) ? total - m_previousTotals.at(i) : 0;
        const quint64 deltaIdle = idle >= m_previousIdles.at(i) ? idle - m_previousIdles.at(i) : 0;

        const double percent = deltaTotal > 0 ? 100.0 * static_cast<double>(deltaTotal - deltaIdle) / static_cast<double>(deltaTotal) : 0.0;

        if (i == 0) {
            m_usage = percent;
        } else {
            m_coreUsages.append(percent);
        }

        m_previousTotals[i] = total;
        m_previousIdles[i] = idle;
    }

    m_coreCount = qMax(1, count - 1);

    updateLoads();
    updateClocks();

    Q_EMIT changed();
}

void CpuInfo::updateLoads()
{
    const QByteArray content = Procfs::read(QStringLiteral("/proc/loadavg")).simplified();
    const QList<QByteArray> fields = content.split(' ');
    if (fields.size() >= 3) {
        m_load1 = fields.at(0).toDouble();
        m_load5 = fields.at(1).toDouble();
        m_load15 = fields.at(2).toDouble();
    }
}

void CpuInfo::updateClocks()
{
    const QDir cpuDir(QStringLiteral("/sys/devices/system/cpu"));
    const QStringList entries = cpuDir.entryList({ QStringLiteral("cpu[0-9]*") }, QDir::Dirs | QDir::NoSymLinks);

    double sumKHz = 0.0;
    int count = 0;
    for (const QString &entry : entries) {
        const QString path = cpuDir.filePath(entry + QStringLiteral("/cpufreq/scaling_cur_freq"));
        bool ok = false;
        const quint64 khz = Procfs::readUInt64(path, &ok);
        if (ok && khz > 0) {
            sumKHz += static_cast<double>(khz);
            ++count;
        }
    }

    if (count > 0) {
        m_clock = sumKHz / count / 1000.0;
        return;
    }

    // Fallback for platforms without cpufreq (mainly x86 /proc/cpuinfo)
    const QList<QByteArray> lines = Procfs::lines(QStringLiteral("/proc/cpuinfo"));
    const QRegularExpression re(QStringLiteral("^cpu MHz\\s*:\\s*([0-9.]+)"));
    for (const QByteArray &line : lines) {
        const QRegularExpressionMatch match = re.match(QString::fromLatin1(line));
        if (match.hasMatch()) {
            m_clock = match.captured(1).toDouble();
            return;
        }
    }

    m_clock = 0.0;
}

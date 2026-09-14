// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "memory_info.h"

#include "Utils/procfs.h"

#include <QHash>

void MemoryInfo::update()
{
    const QList<QByteArray> lines = Procfs::lines(QStringLiteral("/proc/meminfo"));

    QHash<QString, qulonglong> values;
    values.reserve(48);

    for (const QByteArray &line : lines) {
        const int separator = line.indexOf(':');
        if (separator < 0) {
            continue;
        }
        const QString key = QString::fromLatin1(line.left(separator));
        const QList<QByteArray> fields = line.mid(separator + 1).simplified().split(' ');
        if (fields.isEmpty()) {
            continue;
        }
        bool ok = false;
        const qulonglong kib = fields.at(0).toULongLong(&ok);
        if (ok) {
            values.insert(key, kib * 1024ULL);
        }
    }

    m_total = values.value(QStringLiteral("MemTotal"));
    const qulonglong freeMemory = values.value(QStringLiteral("MemFree"));

    qulonglong available = values.value(QStringLiteral("MemAvailable"));
    if (available == 0) {
        const qulonglong buffers = values.value(QStringLiteral("Buffers"));
        const qulonglong cachedAndReclaimable = values.value(QStringLiteral("Cached")) + values.value(QStringLiteral("SReclaimable"));
        const qulonglong shmem = values.value(QStringLiteral("Shmem"));
        const qulonglong cached = cachedAndReclaimable >= shmem ? cachedAndReclaimable - shmem : 0;
        available = freeMemory + buffers + cached;
    }

    m_used = m_total > available ? m_total - available : 0;
    m_swapTotal = values.value(QStringLiteral("SwapTotal"));
    const qulonglong swapFree = values.value(QStringLiteral("SwapFree"));
    m_swapUsed = m_swapTotal > swapFree ? m_swapTotal - swapFree : 0;

    Q_EMIT changed();
}

MemoryInfo::MemoryInfo(QObject *parent) :
    QObject(parent)
{
}

qulonglong MemoryInfo::total() const
{
    return m_total;
}

qulonglong MemoryInfo::used() const
{
    return m_used;
}

qulonglong MemoryInfo::swapTotal() const
{
    return m_swapTotal;
}

qulonglong MemoryInfo::swapUsed() const
{
    return m_swapUsed;
}

double MemoryInfo::usagePercent() const
{
    return m_total > 0 ? 100.0 * static_cast<double>(m_used) / static_cast<double>(m_total) : 0.0;
}

double MemoryInfo::swapPercent() const
{
    return m_swapTotal > 0 ? 100.0 * static_cast<double>(m_swapUsed) / static_cast<double>(m_swapTotal) : 0.0;
}

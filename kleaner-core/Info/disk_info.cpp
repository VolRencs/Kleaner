// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "disk_info.h"

#include "Utils/procfs.h"

#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QStorageInfo>

namespace
{
bool isPseudoFileSystem(const QByteArray &fsType)
{
    static const QSet<QByteArray> pseudoFileSystems = {
        "tmpfs", "devtmpfs", "devpts", "proc", "sysfs", "cgroup", "cgroup2", "securityfs", "debugfs",
        "tracefs", "configfs", "fusectl", "bpf", "autofs", "mqueue", "hugetlbfs", "ramfs", "pstore",
        "efivarfs", "overlay", "squashfs", "nsfs", "binfmt_misc", "rpc_pipefs", "fuse.portal",
        "fuse.gvfsd-fuse"
    };
    return pseudoFileSystems.contains(fsType);
}

bool isPhysicalBlockDevice(const QString &name)
{
    static const QStringList prefixes = {
        QStringLiteral("loop"), QStringLiteral("ram"), QStringLiteral("zram"),
        QStringLiteral("sr"), QStringLiteral("fd"), QStringLiteral("dm-"), QStringLiteral("md")
    };
    for (const QString &prefix : prefixes) {
        if (name.startsWith(prefix)) {
            return false;
        }
    }
    return true;
}
}

DiskInfo::DiskInfo(QObject *parent) :
    QObject(parent)
{
}

QVariantList DiskInfo::disks() const
{
    return m_disks;
}

qulonglong DiskInfo::readBytes() const
{
    return m_readBytes;
}

qulonglong DiskInfo::writeBytes() const
{
    return m_writeBytes;
}

double DiskInfo::readRate() const
{
    return m_readRate;
}

double DiskInfo::writeRate() const
{
    return m_writeRate;
}

void DiskInfo::update()
{
    // Mounted file systems
    QVariantList diskList;

    const QList<QStorageInfo> volumes = QStorageInfo::mountedVolumes();
    for (const QStorageInfo &volume : volumes) {
        if (!volume.isValid() || !volume.isReady() || volume.bytesTotal() <= 0) {
            continue;
        }
        if (volume.device().isEmpty() || isPseudoFileSystem(volume.fileSystemType())) {
            continue;
        }

        const QString mountPoint = volume.rootPath();
        const QString name = mountPoint == QLatin1String("/") ? QStringLiteral("root") : QFileInfo(mountPoint).fileName();

        const qulonglong total = static_cast<qulonglong>(volume.bytesTotal());
        const qulonglong free = static_cast<qulonglong>(volume.bytesFree());
        const qulonglong used = total > free ? total - free : 0;

        diskList.append(QVariantMap {
            { QStringLiteral("name"), name },
            { QStringLiteral("mountPoint"), mountPoint },
            { QStringLiteral("device"), QString::fromUtf8(volume.device()) },
            { QStringLiteral("fileSystemType"), QString::fromUtf8(volume.fileSystemType()) },
            { QStringLiteral("total"), total },
            { QStringLiteral("used"), used },
            { QStringLiteral("free"), free },
            { QStringLiteral("percent"), total > 0 ? 100.0 * static_cast<double>(used) / static_cast<double>(total) : 0.0 },
        });
    }

    m_disks = diskList;

    // Block device I/O
    qulonglong sectorsRead = 0;
    qulonglong sectorsWritten = 0;

    const QDir blockDir(QStringLiteral("/sys/block"));
    const QStringList devices = blockDir.entryList(QDir::Dirs | QDir::NoSymLinks);
    for (const QString &device : devices) {
        if (!isPhysicalBlockDevice(device)) {
            continue;
        }
        const QList<QByteArray> fields = Procfs::read(blockDir.filePath(device + QStringLiteral("/stat"))).simplified().split(' ');
        if (fields.size() < 7) {
            continue;
        }
        sectorsRead += fields.at(2).toULongLong();
        sectorsWritten += fields.at(6).toULongLong();
    }

    const qulonglong read = sectorsRead * 512ULL;
    const qulonglong write = sectorsWritten * 512ULL;

    m_readBytes = read;
    m_writeBytes = write;

    if (!m_hasBaseline || read < m_previousRead || write < m_previousWrite) {
        m_previousRead = read;
        m_previousWrite = write;
        m_hasBaseline = true;
        m_elapsed.restart();
        m_readRate = 0.0;
        m_writeRate = 0.0;
    } else {
        const qint64 elapsedMs = m_elapsed.isValid() ? m_elapsed.elapsed() : 0;
        if (elapsedMs > 200) {
            m_readRate = static_cast<double>(read - m_previousRead) * 1000.0 / static_cast<double>(elapsedMs);
            m_writeRate = static_cast<double>(write - m_previousWrite) * 1000.0 / static_cast<double>(elapsedMs);
            m_previousRead = read;
            m_previousWrite = write;
            m_elapsed.restart();
        }
    }

    Q_EMIT changed();
}

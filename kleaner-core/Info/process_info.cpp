// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "process_info.h"

#include "Utils/helpers.h"
#include "Utils/procfs.h"

#include <QDir>
#include <QMutexLocker>

#include <csignal>
#include <unistd.h>

ProcessInfo::ProcessInfo(QObject *parent) :
    QObject(parent)
{
}

QVector<Process> ProcessInfo::read()
{
    QMutexLocker locker(&m_mutex);

    const QList<QByteArray> statLines = Procfs::lines(QStringLiteral("/proc/stat"));
    quint64 totalJiffies = 0;
    if (!statLines.isEmpty() && statLines.first().startsWith("cpu ")) {
        const QList<QByteArray> fields = statLines.first().simplified().split(' ');
        for (int i = 1; i < fields.size(); ++i) {
            totalJiffies += fields.at(i).toULongLong();
        }
    }
    const quint64 deltaTotal = m_hasBaseline && totalJiffies > m_previousTotal ? totalJiffies - m_previousTotal : 0;

    if (m_memoryTotal == 0) {
        const QList<QByteArray> memoryLines = Procfs::lines(QStringLiteral("/proc/meminfo"));
        for (const QByteArray &line : memoryLines) {
            if (line.startsWith("MemTotal:")) {
                m_memoryTotal = line.mid(9).trimmed().split(' ').first().toULongLong() * 1024ULL;
                break;
            }
        }
    }

    QHash<int, quint64> currentCpu;

    QVector<Process> processes;
    processes.reserve(512);

    const QDir procDir(QStringLiteral("/proc"));
    const QStringList entries = procDir.entryList(QDir::Dirs | QDir::NoSymLinks);
    for (const QString &entry : entries) {
        bool isPid = false;
        const int pid = entry.toInt(&isPid);
        if (!isPid || pid <= 0) {
            continue;
        }

        const QString basePath = QStringLiteral("/proc/") + entry;
        const QByteArray stat = Procfs::read(basePath + QStringLiteral("/stat"));
        if (stat.isEmpty()) {
            continue;
        }

        const int openParen = stat.indexOf('(');
        const int closeParen = stat.lastIndexOf(')');
        if (openParen < 0 || closeParen <= openParen) {
            continue;
        }

        Process process;
        process.pid = pid;
        process.name = QString::fromUtf8(stat.mid(openParen + 1, closeParen - openParen - 1));

        const QList<QByteArray> fields = stat.mid(closeParen + 2).simplified().split(' ');
        if (fields.size() >= 22) {
            process.state = QChar::fromLatin1(fields.at(0).isEmpty() ? '?' : fields.at(0).at(0));
            const quint64 utime = fields.at(11).toULongLong();
            const quint64 stime = fields.at(12).toULongLong();
            process.nice = fields.at(16).toInt();
            process.vsize = fields.at(20).toULongLong();
            const qulonglong rssPages = fields.at(21).toULongLong();
            process.rss = rssPages * static_cast<qulonglong>(Helpers::pageSizeKiB()) * 1024ULL;
            const quint64 currentTicks = utime + stime;
            const quint64 previousTicks = m_previousCpu.value(pid);
            const quint64 deltaTicks = currentTicks >= previousTicks ? currentTicks - previousTicks : 0;
            process.cpu = deltaTotal > 0 ? 100.0 * static_cast<double>(deltaTicks) / static_cast<double>(deltaTotal) : 0.0;
            currentCpu.insert(pid, currentTicks);
        } else {
            process.state = QChar::fromLatin1('?');
        }

        // User name from the first Uid field in /proc/<pid>/status
        const QList<QByteArray> statusLines = Procfs::lines(basePath + QStringLiteral("/status"));
        for (const QByteArray &line : statusLines) {
            if (line.startsWith("Uid:")) {
                const QList<QByteArray> uidFields = line.mid(4).simplified().split(' ');
                if (!uidFields.isEmpty()) {
                    process.user = Helpers::userName(static_cast<uid_t>(uidFields.first().toUInt()));
                }
                break;
            }
        }

        const QByteArray cmdline = Procfs::read(basePath + QStringLiteral("/cmdline"));
        if (cmdline.isEmpty()) {
            process.cmd = QStringLiteral("[%1]").arg(process.name);
        } else {
            QString command = QString::fromUtf8(cmdline);
            command.replace(QChar::Null, QLatin1Char(' '));
            process.cmd = command.trimmed();
        }

        process.mem = m_memoryTotal > 0 ? 100.0 * static_cast<double>(process.rss) / static_cast<double>(m_memoryTotal) : 0.0;

        processes.append(process);
    }

    m_previousCpu = currentCpu;
    m_previousTotal = totalJiffies;
    m_hasBaseline = true;

    return processes;
}

bool ProcessInfo::killProcess(int pid, bool force)
{
    if (pid <= 1) {
        return false;
    }
    return ::kill(pid, force ? SIGKILL : SIGTERM) == 0;
}

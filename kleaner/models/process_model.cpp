// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "process_model.h"

#include <QFutureWatcher>
#include <QSet>
#include <QtConcurrent>

#include <algorithm>

namespace
{
bool sameProcess(const Process &a, const Process &b)
{
    return a.pid == b.pid && a.name == b.name && a.user == b.user && a.state == b.state && qFuzzyCompare(a.cpu + 1.0, b.cpu + 1.0)
        && qFuzzyCompare(a.mem + 1.0, b.mem + 1.0) && a.rss == b.rss && a.vsize == b.vsize && a.nice == b.nice && a.cmd == b.cmd;
}
}

ProcessModel::ProcessModel(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(&m_watcher, &QFutureWatcher<QList<Process>>::finished, this, [this] {
        m_all = m_watcher.result();
        m_fetching = false;
        if (m_loading) {
            m_loading = false;
            Q_EMIT loadingChanged();
        }
        // Re-sorts as well, so the busiest processes keep moving to the top.
        applyFilterAndSort();
    });
}

ProcessModel::~ProcessModel()
{
    // The worker captures this, so it must not outlive the model.
    m_watcher.cancel();
    m_watcher.waitForFinished();
}

int ProcessModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_view.size();
}

QVariant ProcessModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_view.size()) {
        return {};
    }

    const Process &process = m_view.at(index.row());
    switch (role) {
    case PidRole:
        return process.pid;
    case NameRole:
        return process.name;
    case UserRole:
        return process.user;
    case StateRole:
        return QString(process.state);
    case CpuRole:
        return process.cpu;
    case MemRole:
        return process.mem;
    case RssRole:
        return process.rss;
    case CmdRole:
        return process.cmd;
    default:
        return {};
    }
}

QHash<int, QByteArray> ProcessModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { PidRole, "pid" },
        { NameRole, "name" },
        { UserRole, "user" },
        { StateRole, "processState" },
        { CpuRole, "cpu" },
        { MemRole, "mem" },
        { RssRole, "rss" },
        { CmdRole, "cmd" },
    };
    return roles;
}

QString ProcessModel::filter() const
{
    return m_filter;
}

void ProcessModel::setFilter(const QString &filter)
{
    if (m_filter == filter) {
        return;
    }
    m_filter = filter;
    Q_EMIT filterChanged();
    applyFilterAndSort();
}

int ProcessModel::sortBy() const
{
    return m_sortBy;
}

void ProcessModel::setSortBy(int sortBy)
{
    if (m_sortBy == sortBy) {
        return;
    }
    m_sortBy = sortBy;
    Q_EMIT sortByChanged();
    applyFilterAndSort();
}

bool ProcessModel::reverse() const
{
    return m_reverse;
}

void ProcessModel::setReverse(bool reverse)
{
    if (m_reverse == reverse) {
        return;
    }
    m_reverse = reverse;
    Q_EMIT reverseChanged();
    applyFilterAndSort();
}

bool ProcessModel::loading() const
{
    return m_loading;
}

bool ProcessModel::paused() const
{
    return m_paused;
}

void ProcessModel::setPaused(bool paused)
{
    if (m_paused == paused) {
        return;
    }
    m_paused = paused;
    Q_EMIT pausedChanged();
}

void ProcessModel::update()
{
    // The automatic refresh re-sorts as well, so the busiest processes keep
    // moving to the top just like in a system monitor.
    fetchProcesses(false);
}

void ProcessModel::refresh()
{
    fetchProcesses(true);
}

void ProcessModel::fetchProcesses(bool showLoading)
{
    if (m_fetching || m_paused) {
        return;
    }

    m_fetching = true;
    if (showLoading) {
        m_loading = true;
        Q_EMIT loadingChanged();
    }

    m_watcher.setFuture(QtConcurrent::run([this] {
        return m_info.read();
    }));
}

void ProcessModel::applyFilterAndSort()
{
    QList<Process> filtered;
    filtered.reserve(m_all.size());

    if (m_filter.isEmpty()) {
        filtered = m_all;
    } else {
        for (const Process &process : std::as_const(m_all)) {
            if (process.name.contains(m_filter, Qt::CaseInsensitive) || process.cmd.contains(m_filter, Qt::CaseInsensitive)
                || process.user.contains(m_filter, Qt::CaseInsensitive) || QString::number(process.pid).contains(m_filter)) {
                filtered.append(process);
            }
        }
    }

    // Stable sorting keeps equal values in their previous relative order.
    std::stable_sort(filtered.begin(), filtered.end(), [this](const Process &a, const Process &b) {
        int comparison = 0;
        switch (m_sortBy) {
        case SortMemory:
            comparison = a.mem < b.mem ? -1 : (a.mem > b.mem ? 1 : 0);
            break;
        case SortName:
            comparison = QString::localeAwareCompare(a.name, b.name);
            break;
        case SortPid:
            comparison = a.pid < b.pid ? -1 : (a.pid > b.pid ? 1 : 0);
            break;
        case SortUser:
            comparison = QString::localeAwareCompare(a.user, b.user);
            break;
        case SortRss:
            comparison = a.rss < b.rss ? -1 : (a.rss > b.rss ? 1 : 0);
            break;
        case SortState:
            comparison = a.state < b.state ? -1 : (a.state > b.state ? 1 : 0);
            break;
        case SortCpu:
        default:
            comparison = a.cpu < b.cpu ? -1 : (a.cpu > b.cpu ? 1 : 0);
            break;
        }
        return m_reverse ? comparison > 0 : comparison < 0;
    });

    QHash<int, int> wanted;
    wanted.reserve(filtered.size());
    for (int i = 0; i < filtered.size(); ++i) {
        wanted.insert(filtered.at(i).pid, i);
    }

    // Remove processes that disappeared without resetting the model.
    for (int row = m_view.size() - 1; row >= 0; --row) {
        if (!wanted.contains(m_view.at(row).pid)) {
            beginRemoveRows(QModelIndex(), row, row);
            m_view.removeAt(row);
            endRemoveRows();
        }
    }

    // Append processes that appeared for the first time.
    QSet<int> currentPids;
    currentPids.reserve(m_view.size());
    for (const Process &current : std::as_const(m_view)) {
        currentPids.insert(current.pid);
    }
    for (const Process &process : std::as_const(filtered)) {
        if (!currentPids.contains(process.pid)) {
            beginInsertRows(QModelIndex(), m_view.size(), m_view.size());
            m_view.append(process);
            currentPids.insert(process.pid);
            endInsertRows();
        }
    }

    // Move rows into their sorted positions; the TableView keeps its viewport
    // stable while rows are moved, only the order of the processes changes.
    for (int target = 0; target < filtered.size(); ++target) {
        const int pid = filtered.at(target).pid;
        int source = target;
        while (source < m_view.size() && m_view.at(source).pid != pid) {
            ++source;
        }
        if (source == target || source >= m_view.size()) {
            continue;
        }
        beginMoveRows(QModelIndex(), source, source, QModelIndex(), target);
        m_view.move(source, target);
        endMoveRows();
    }

    // Refresh values of the rows that stayed in place.
    for (int i = 0; i < m_view.size(); ++i) {
        const Process &updated = filtered.at(i);
        if (!sameProcess(m_view.at(i), updated)) {
            m_view[i] = updated;
            const QModelIndex modelIndex = index(i, 0);
            Q_EMIT dataChanged(modelIndex, modelIndex);
        }
    }
}

bool ProcessModel::killPid(int pid, bool force)
{
    if (pid <= 1 || !hasPid(pid)) {
        return false;
    }
    if (!m_info.killProcess(pid, force)) {
        Q_EMIT error(tr("Could not terminate process %1. You may not have permission.").arg(pid));
        return false;
    }
    return true;
}

bool ProcessModel::hasPid(int pid) const
{
    for (const Process &process : m_view) {
        if (process.pid == pid) {
            return true;
        }
    }
    return false;
}

#include "process_model.h"

#include <QFutureWatcher>
#include <QtConcurrent>

#include <algorithm>

ProcessModel::ProcessModel(QObject *parent) :
    QAbstractListModel(parent)
{
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
    case VsizeRole:
        return process.vsize;
    case NiceRole:
        return process.nice;
    case CmdRole:
        return process.cmd;
    default:
        return {};
    }
}

QHash<int, QByteArray> ProcessModel::roleNames() const
{
    return {
        { PidRole, "pid" },
        { NameRole, "name" },
        { UserRole, "user" },
        { StateRole, "state" },
        { CpuRole, "cpu" },
        { MemRole, "mem" },
        { RssRole, "rss" },
        { VsizeRole, "vsize" },
        { NiceRole, "nice" },
        { CmdRole, "cmd" },
    };
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
    Q_EMIT sortByChanged();
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
    if (m_loading || m_paused) {
        return;
    }

    m_loading = true;
    Q_EMIT loadingChanged();

    auto *watcher = new QFutureWatcher<QVector<Process>>(this);
    connect(watcher, &QFutureWatcher<QVector<Process>>::finished, this, [this, watcher] {
        m_all = watcher->result();
        watcher->deleteLater();
        m_loading = false;
        Q_EMIT loadingChanged();
        applyFilterAndSort();
    });

    watcher->setFuture(QtConcurrent::run([this] {
        return m_info.read();
    }));
}

void ProcessModel::applyFilterAndSort()
{
    QVector<Process> filtered;
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

    std::sort(filtered.begin(), filtered.end(), [this](const Process &a, const Process &b) {
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
        case SortCpu:
        default:
            comparison = a.cpu < b.cpu ? -1 : (a.cpu > b.cpu ? 1 : 0);
            break;
        }
        return m_reverse ? comparison > 0 : comparison < 0;
    });

    beginResetModel();
    m_view = filtered;
    endResetModel();
}

bool ProcessModel::killPid(int pid, bool force)
{
    if (pid <= 1) {
        return false;
    }
    if (!m_info.killProcess(pid, force)) {
        Q_EMIT error(tr("Could not terminate process %1. You may not have permission.").arg(pid));
        return false;
    }
    return true;
}

int ProcessModel::pidAt(int row) const
{
    if (row < 0 || row >= m_view.size()) {
        return -1;
    }
    return m_view.at(row).pid;
}

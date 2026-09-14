// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "Info/process_info.h"

class ProcessModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(int sortBy READ sortBy WRITE setSortBy NOTIFY sortByChanged)
    Q_PROPERTY(bool reverse READ reverse WRITE setReverse NOTIFY reverseChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)

  public:
    enum Roles {
        PidRole = Qt::UserRole + 1,
        NameRole,
        UserRole,
        StateRole,
        CpuRole,
        MemRole,
        RssRole,
        CmdRole,
    };
    Q_ENUM(Roles)

    enum SortBy {
        SortCpu,
        SortMemory,
        SortName,
        SortPid,
        SortUser,
        SortRss,
        SortState,
    };
    Q_ENUM(SortBy)

    explicit ProcessModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filter() const;
    void setFilter(const QString &filter);

    int sortBy() const;
    void setSortBy(int sortBy);

    bool reverse() const;
    void setReverse(bool reverse);

    bool loading() const;

    bool paused() const;
    void setPaused(bool paused);

    Q_INVOKABLE void update();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool killPid(int pid, bool force);
    Q_INVOKABLE bool hasPid(int pid) const;

  Q_SIGNALS:
    void filterChanged();
    void sortByChanged();
    void reverseChanged();
    void loadingChanged();
    void pausedChanged();
    void error(const QString &message);

  private:
    void fetchProcesses(bool reorder, bool showLoading);
    void applyFilterAndSort(bool reorder);

    ProcessInfo m_info;
    QVector<Process> m_all;
    QVector<Process> m_view;
    QString m_filter;
    int m_sortBy = SortCpu;
    bool m_reverse = true;
    bool m_loading = false;
    bool m_fetching = false;
    bool m_paused = true;
};

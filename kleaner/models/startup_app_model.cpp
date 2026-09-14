// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "startup_app_model.h"

StartupAppModel::StartupAppModel(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(&m_apps, &StartupApps::changed, this, &StartupAppModel::reload);
    reload();
}

int StartupAppModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_view.size();
}

QVariant StartupAppModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_view.size()) {
        return {};
    }

    const QVariantMap app = m_view.at(index.row()).toMap();
    switch (role) {
    case NameRole:
        return app.value(QStringLiteral("name"));
    case CommentRole:
        return app.value(QStringLiteral("comment"));
    case ExecRole:
        return app.value(QStringLiteral("exec"));
    case IconRole:
        return app.value(QStringLiteral("icon"));
    case EnabledRole:
        return app.value(QStringLiteral("enabled"));
    case SystemRole:
        return app.value(QStringLiteral("system"));
    default:
        return {};
    }
}

QHash<int, QByteArray> StartupAppModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { CommentRole, "comment" },
        { ExecRole, "exec" },
        { IconRole, "icon" },
        { EnabledRole, "enabled" },
        { SystemRole, "system" },
    };
}

QString StartupAppModel::filter() const
{
    return m_filter;
}

void StartupAppModel::setFilter(const QString &filter)
{
    if (m_filter == filter) {
        return;
    }
    m_filter = filter;
    Q_EMIT filterChanged();
    applyFilter();
}

void StartupAppModel::reload()
{
    m_all = m_apps.load();
    applyFilter();
}

void StartupAppModel::setEnabled(int row, bool enabled)
{
    if (row < 0 || row >= m_view.size()) {
        return;
    }
    const QString path = m_view.at(row).toMap().value(QStringLiteral("path")).toString();
    if (!m_apps.setEnabled(path, enabled)) {
        Q_EMIT error(tr("Could not change the startup entry."));
    }
}

bool StartupAppModel::save(int row, const QString &name, const QString &comment, const QString &exec, const QString &icon)
{
    if (row < 0 || row >= m_view.size()) {
        return false;
    }
    const QString path = m_view.at(row).toMap().value(QStringLiteral("path")).toString();
    if (!m_apps.save(path, name, comment, exec, icon)) {
        Q_EMIT error(tr("Could not save the startup entry."));
        return false;
    }
    return true;
}

int StartupAppModel::add(const QString &name, const QString &comment, const QString &exec, const QString &icon)
{
    const QString path = m_apps.create(name, comment, exec, icon);
    if (path.isEmpty()) {
        Q_EMIT error(tr("Could not create the startup entry."));
        return -1;
    }

    for (int i = 0; i < m_view.size(); ++i) {
        if (m_view.at(i).toMap().value(QStringLiteral("path")).toString() == path) {
            return i;
        }
    }
    return -1;
}

bool StartupAppModel::remove(int row)
{
    if (row < 0 || row >= m_view.size()) {
        return false;
    }
    const QString path = m_view.at(row).toMap().value(QStringLiteral("path")).toString();
    if (!m_apps.remove(path)) {
        Q_EMIT error(tr("Could not remove the startup entry."));
        return false;
    }
    return true;
}

void StartupAppModel::applyFilter()
{
    QVariantList filtered;
    filtered.reserve(m_all.size());

    if (m_filter.isEmpty()) {
        filtered = m_all;
    } else {
        for (const QVariant &variant : std::as_const(m_all)) {
            const QVariantMap app = variant.toMap();
            if (app.value(QStringLiteral("name")).toString().contains(m_filter, Qt::CaseInsensitive)
                || app.value(QStringLiteral("exec")).toString().contains(m_filter, Qt::CaseInsensitive)) {
                filtered.append(variant);
            }
        }
    }

    beginResetModel();
    m_view = filtered;
    endResetModel();
}

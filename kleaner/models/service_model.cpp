// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "service_model.h"

#include <algorithm>

ServiceModel::ServiceModel(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(&m_backend, &ServiceBackend::loaded, this, [this](const QVariantList &services) {
        m_all = services;
        applyFilter();
    });
    connect(&m_backend, &ServiceBackend::error, this, &ServiceModel::error);
}

int ServiceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_view.size();
}

QVariant ServiceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_view.size()) {
        return {};
    }

    const QVariantMap service = m_view.at(index.row()).toMap();
    switch (role) {
    case NameRole:
        return service.value(QStringLiteral("name"));
    case DescriptionRole:
        return service.value(QStringLiteral("description"));
    case EnabledRole:
        return service.value(QStringLiteral("enabled"));
    case ActiveRole:
        return service.value(QStringLiteral("active"));
    case ActiveStateRole:
        return service.value(QStringLiteral("activeState"));
    default:
        return {};
    }
}

QHash<int, QByteArray> ServiceModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { DescriptionRole, "description" },
        { EnabledRole, "enabled" },
        { ActiveRole, "active" },
        { ActiveStateRole, "activeState" },
    };
}

QString ServiceModel::filter() const
{
    return m_filter;
}

void ServiceModel::setFilter(const QString &filter)
{
    if (m_filter == filter) {
        return;
    }
    m_filter = filter;
    Q_EMIT filterChanged();
    applyFilter();
}

bool ServiceModel::available() const
{
    return m_backend.available();
}

int ServiceModel::sortBy() const
{
    return m_sortBy;
}

void ServiceModel::setSortBy(int sortBy)
{
    if (m_sortBy == sortBy) {
        return;
    }
    m_sortBy = sortBy;
    Q_EMIT sortByChanged();
    applyFilter();
}

bool ServiceModel::reverse() const
{
    return m_reverse;
}

void ServiceModel::setReverse(bool reverse)
{
    if (m_reverse == reverse) {
        return;
    }
    m_reverse = reverse;
    Q_EMIT reverseChanged();
    applyFilter();
}

void ServiceModel::reload()
{
    m_backend.reload();
}

void ServiceModel::setEnabled(int row, bool enabled)
{
    if (row < 0 || row >= m_view.size()) {
        return;
    }
    m_backend.setEnabled(m_view.at(row).toMap().value(QStringLiteral("unit")).toString(), enabled);
}

void ServiceModel::start(int row)
{
    if (row < 0 || row >= m_view.size()) {
        return;
    }
    m_backend.start(m_view.at(row).toMap().value(QStringLiteral("unit")).toString());
}

void ServiceModel::stop(int row)
{
    if (row < 0 || row >= m_view.size()) {
        return;
    }
    m_backend.stop(m_view.at(row).toMap().value(QStringLiteral("unit")).toString());
}

void ServiceModel::restart(int row)
{
    if (row < 0 || row >= m_view.size()) {
        return;
    }
    m_backend.restart(m_view.at(row).toMap().value(QStringLiteral("unit")).toString());
}

void ServiceModel::applyFilter()
{
    QVariantList filtered;
    filtered.reserve(m_all.size());

    if (m_filter.isEmpty()) {
        filtered = m_all;
    } else {
        for (const QVariant &variant : std::as_const(m_all)) {
            const QVariantMap service = variant.toMap();
            if (service.value(QStringLiteral("name")).toString().contains(m_filter, Qt::CaseInsensitive)
                || service.value(QStringLiteral("description")).toString().contains(m_filter, Qt::CaseInsensitive)) {
                filtered.append(variant);
            }
        }
    }

    std::stable_sort(filtered.begin(), filtered.end(), [this](const QVariant &a, const QVariant &b) {
        const QVariantMap left = a.toMap();
        const QVariantMap right = b.toMap();
        int comparison = 0;
        switch (m_sortBy) {
        case SortState:
            comparison = left.value(QStringLiteral("active")).toBool() == right.value(QStringLiteral("active")).toBool()
                ? QString::localeAwareCompare(left.value(QStringLiteral("name")).toString(), right.value(QStringLiteral("name")).toString())
                : (left.value(QStringLiteral("active")).toBool() ? -1 : 1);
            break;
        case SortStartup:
            comparison = left.value(QStringLiteral("enabled")).toBool() == right.value(QStringLiteral("enabled")).toBool()
                ? QString::localeAwareCompare(left.value(QStringLiteral("name")).toString(), right.value(QStringLiteral("name")).toString())
                : (left.value(QStringLiteral("enabled")).toBool() ? -1 : 1);
            break;
        case SortName:
        default:
            comparison = QString::localeAwareCompare(left.value(QStringLiteral("name")).toString(), right.value(QStringLiteral("name")).toString());
            break;
        }
        return m_reverse ? comparison > 0 : comparison < 0;
    });

    beginResetModel();
    m_view = filtered;
    endResetModel();
}

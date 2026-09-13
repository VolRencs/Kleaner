#include "service_model.h"

ServiceModel::ServiceModel(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(&m_backend, &ServiceBackend::loaded, this, [this](const QVariantList &services) {
        m_all = services;
        applyFilter();
    });
    connect(&m_backend, &ServiceBackend::error, this, &ServiceModel::error);
    connect(&m_backend, &ServiceBackend::availableChanged, this, &ServiceModel::availableChanged);
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
    case UnitRole:
        return service.value(QStringLiteral("unit"));
    case DescriptionRole:
        return service.value(QStringLiteral("description"));
    case EnabledRole:
        return service.value(QStringLiteral("enabled"));
    case ActiveRole:
        return service.value(QStringLiteral("active"));
    case ActiveStateRole:
        return service.value(QStringLiteral("activeState"));
    case UnitFileStateRole:
        return service.value(QStringLiteral("unitFileState"));
    default:
        return {};
    }
}

QHash<int, QByteArray> ServiceModel::roleNames() const
{
    return {
        { NameRole, "name" },
        { UnitRole, "unit" },
        { DescriptionRole, "description" },
        { EnabledRole, "enabled" },
        { ActiveRole, "active" },
        { ActiveStateRole, "activeState" },
        { UnitFileStateRole, "unitFileState" },
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

    beginResetModel();
    m_view = filtered;
    endResetModel();
}

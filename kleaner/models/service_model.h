// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QVariantList>

#include "Services/service_backend.h"

class ServiceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(int sortBy READ sortBy WRITE setSortBy NOTIFY sortByChanged)
    Q_PROPERTY(bool reverse READ reverse WRITE setReverse NOTIFY reverseChanged)

  public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        DescriptionRole,
        EnabledRole,
        ActiveRole,
        ActiveStateRole,
    };
    Q_ENUM(Roles)
    enum SortBy {
        SortName,
        SortState,
        SortStartup,
    };
    Q_ENUM(SortBy)

    explicit ServiceModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filter() const;
    void setFilter(const QString &filter);

    bool available() const;

    int sortBy() const;
    void setSortBy(int sortBy);

    bool reverse() const;
    void setReverse(bool reverse);

    Q_INVOKABLE void reload();
    Q_INVOKABLE void setEnabled(int row, bool enabled);
    Q_INVOKABLE void start(int row);
    Q_INVOKABLE void stop(int row);
    Q_INVOKABLE void restart(int row);

  Q_SIGNALS:
    void filterChanged();
    void sortByChanged();
    void reverseChanged();
    void error(const QString &message);

  private:
    void applyFilter();

    ServiceBackend m_backend;
    QVariantList m_all;
    QVariantList m_view;
    QString m_filter;
    int m_sortBy = SortName;
    bool m_reverse = false;
};

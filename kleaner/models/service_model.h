#pragma once

#include <QAbstractListModel>
#include <QVariantList>

#include "Services/service_backend.h"

class ServiceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)

  public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        UnitRole,
        DescriptionRole,
        EnabledRole,
        ActiveRole,
        ActiveStateRole,
        UnitFileStateRole,
    };
    Q_ENUM(Roles)

    explicit ServiceModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filter() const;
    void setFilter(const QString &filter);

    bool available() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void setEnabled(int row, bool enabled);
    Q_INVOKABLE void start(int row);
    Q_INVOKABLE void stop(int row);
    Q_INVOKABLE void restart(int row);

  Q_SIGNALS:
    void filterChanged();
    void availableChanged();
    void error(const QString &message);

  private:
    void applyFilter();

    ServiceBackend m_backend;
    QVariantList m_all;
    QVariantList m_view;
    QString m_filter;
};

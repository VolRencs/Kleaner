#pragma once

#include <QAbstractListModel>
#include <QVariantList>

#include "Startup/startup_apps.h"

class StartupAppModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

  public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        CommentRole,
        ExecRole,
        IconRole,
        EnabledRole,
        SystemRole,
        PathRole,
    };
    Q_ENUM(Roles)

    explicit StartupAppModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filter() const;
    void setFilter(const QString &filter);

    Q_INVOKABLE void reload();
    Q_INVOKABLE void setEnabled(int row, bool enabled);
    Q_INVOKABLE bool save(int row, const QString &name, const QString &comment, const QString &exec, const QString &icon);
    Q_INVOKABLE int add(const QString &name, const QString &comment, const QString &exec, const QString &icon);
    Q_INVOKABLE bool remove(int row);

  Q_SIGNALS:
    void filterChanged();
    void error(const QString &message);

  private:
    void applyFilter();

    StartupApps m_apps;
    QVariantList m_all;
    QVariantList m_view;
    QString m_filter;
};

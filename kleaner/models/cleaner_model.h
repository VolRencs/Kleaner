// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QAbstractListModel>
#include <QSet>
#include <QVariantList>

#include "Cleaner/cleaner.h"

class CleanerModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(bool cleaning READ cleaning NOTIFY cleaningChanged)
    Q_PROPERTY(qulonglong checkedSize READ checkedSize NOTIFY checkedSizeChanged)
    Q_PROPERTY(bool hasCheckedItems READ hasCheckedItems NOTIFY hasCheckedItemsChanged)
    Q_PROPERTY(qulonglong lastFreedBytes READ lastFreedBytes NOTIFY lastResultChanged)
    Q_PROPERTY(int lastRemovedCount READ lastRemovedCount NOTIFY lastResultChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastResultChanged)

  public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        SizeRole,
        DepthRole,
        ExpandableRole,
        ExpandedRole,
        CheckedRole,
        RootRole,
        IsCategoryRole,
    };
    Q_ENUM(Roles)

    explicit CleanerModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool scanning() const;
    bool cleaning() const;
    qulonglong checkedSize() const;
    bool hasCheckedItems() const;
    qulonglong lastFreedBytes() const;
    int lastRemovedCount() const;
    QString lastError() const;

    Q_INVOKABLE void scan();
    Q_INVOKABLE void toggleExpand(int row);
    Q_INVOKABLE void setChecked(int row, bool checked);
    Q_INVOKABLE void clean();

  Q_SIGNALS:
    void scanningChanged();
    void cleaningChanged();
    void checkedSizeChanged();
    void hasCheckedItemsChanged();
    void lastResultChanged();
    void scanFinished();
    void cleanFinished(bool ok, const QString &message, int count, qulonglong freedBytes);

  private:
    struct Category {
        QString id;
        QString title;
        qulonglong size = 0;
        bool expanded = false;
        Qt::CheckState check = Qt::Unchecked;
        struct Entry {
            QString title;
            QString path;
            qulonglong size = 0;
            bool root = false;
            bool checked = false;
        };
        QVector<Entry> entries;
    };

    struct Row {
        int category = -1;
        int entry = -1;
    };

    void rebuild();
    void updateCheckedSize();
    void loadSelection();
    void saveSelection();

    Cleaner m_cleaner;
    QVector<Category> m_categories;
    QVector<Row> m_rows;
    QSet<QString> m_checkedPaths;
    qulonglong m_checkedSize = 0;
    qulonglong m_lastFreedBytes = 0;
    bool m_hasCheckedItems = false;
    bool m_cleaning = false;
    int m_lastRemovedCount = 0;
    QString m_lastError;
};

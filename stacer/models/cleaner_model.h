#pragma once

#include <QAbstractListModel>
#include <QVariantList>

#include "Cleaner/cleaner.h"

class CleanerModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(qulonglong checkedSize READ checkedSize NOTIFY checkedSizeChanged)

  public:
    enum Roles {
        TitleRole = Qt::UserRole + 1,
        PathRole,
        SizeRole,
        DepthRole,
        ExpandableRole,
        ExpandedRole,
        CheckedRole,
        RootRole,
        IsCategoryRole,
        CategoryIdRole,
    };
    Q_ENUM(Roles)

    explicit CleanerModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool scanning() const;
    qulonglong checkedSize() const;

    Q_INVOKABLE void scan();
    Q_INVOKABLE void toggleExpand(int row);
    Q_INVOKABLE void setChecked(int row, bool checked);
    Q_INVOKABLE void clean();

  Q_SIGNALS:
    void scanningChanged();
    void checkedSizeChanged();
    void scanFinished();
    void cleanFinished(bool ok, const QString &message, int count);

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

    Cleaner m_cleaner;
    QVector<Category> m_categories;
    QVector<Row> m_rows;
    qulonglong m_checkedSize = 0;
};

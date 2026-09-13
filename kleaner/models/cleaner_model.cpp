#include "cleaner_model.h"

CleanerModel::CleanerModel(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(&m_cleaner, &Cleaner::scanningChanged, this, &CleanerModel::scanningChanged);
    connect(&m_cleaner, &Cleaner::scanned, this, [this](const QVariantList &categories) {
        m_categories.clear();
        for (const QVariant &variant : categories) {
            const QVariantMap categoryMap = variant.toMap();

            Category category;
            category.id = categoryMap.value(QStringLiteral("id")).toString();
            category.title = categoryMap.value(QStringLiteral("title")).toString();
            category.size = categoryMap.value(QStringLiteral("size")).toULongLong();

            const QVariantList entries = categoryMap.value(QStringLiteral("entries")).toList();
            category.entries.reserve(entries.size());
            for (const QVariant &entryVariant : entries) {
                const QVariantMap entryMap = entryVariant.toMap();
                Category::Entry entry;
                entry.title = entryMap.value(QStringLiteral("title")).toString();
                entry.path = entryMap.value(QStringLiteral("path")).toString();
                entry.size = entryMap.value(QStringLiteral("size")).toULongLong();
                entry.root = entryMap.value(QStringLiteral("root")).toBool();
                category.entries.append(entry);
            }

            m_categories.append(category);
        }

        rebuild();
        updateCheckedSize();
        Q_EMIT scanFinished();
    });
    connect(&m_cleaner, &Cleaner::cleaned, this, [this](int count, const QString &error) {
        Q_EMIT cleanFinished(error.isEmpty(), error, count);
    });
}

int CleanerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_rows.size();
}

QVariant CleanerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
        return {};
    }

    const Row &row = m_rows.at(index.row());
    const Category &category = m_categories.at(row.category);

    if (role == TitleRole) {
        if (row.entry < 0) {
            return category.title;
        }
        return category.entries.at(row.entry).title;
    }
    if (role == PathRole) {
        return row.entry < 0 ? QString() : category.entries.at(row.entry).path;
    }
    if (role == SizeRole) {
        return row.entry < 0 ? category.size : category.entries.at(row.entry).size;
    }
    if (role == DepthRole) {
        return row.entry < 0 ? 0 : 1;
    }
    if (role == ExpandableRole) {
        return row.entry < 0 && !category.entries.isEmpty();
    }
    if (role == ExpandedRole) {
        return row.entry < 0 && category.expanded;
    }
    if (role == CheckedRole) {
        if (row.entry < 0) {
            return static_cast<int>(category.check);
        }
        return static_cast<int>(category.entries.at(row.entry).checked ? Qt::Checked : Qt::Unchecked);
    }
    if (role == RootRole) {
        return row.entry < 0 ? false : category.entries.at(row.entry).root;
    }
    if (role == IsCategoryRole) {
        return row.entry < 0;
    }
    if (role == CategoryIdRole) {
        return category.id;
    }
    return {};
}

QHash<int, QByteArray> CleanerModel::roleNames() const
{
    return {
        { TitleRole, "title" },
        { PathRole, "path" },
        { SizeRole, "size" },
        { DepthRole, "depth" },
        { ExpandableRole, "expandable" },
        { ExpandedRole, "expanded" },
        { CheckedRole, "checkState" },
        { RootRole, "root" },
        { IsCategoryRole, "isCategory" },
        { CategoryIdRole, "categoryId" },
    };
}

bool CleanerModel::scanning() const
{
    return m_cleaner.scanning();
}

qulonglong CleanerModel::checkedSize() const
{
    return m_checkedSize;
}

void CleanerModel::scan()
{
    m_cleaner.scan();
}

void CleanerModel::toggleExpand(int row)
{
    if (row < 0 || row >= m_rows.size()) {
        return;
    }
    const Row &r = m_rows.at(row);
    if (r.entry >= 0) {
        return;
    }

    m_categories[r.category].expanded = !m_categories.at(r.category).expanded;
    rebuild();
}

void CleanerModel::setChecked(int row, bool checked)
{
    if (row < 0 || row >= m_rows.size()) {
        return;
    }

    const Row &r = m_rows.at(row);
    Category &category = m_categories[r.category];

    if (r.entry < 0) {
        category.check = checked ? Qt::Checked : Qt::Unchecked;
        for (Category::Entry &entry : category.entries) {
            entry.checked = checked;
        }
    } else {
        category.entries[r.entry].checked = checked;

        bool allChecked = !category.entries.isEmpty();
        bool anyChecked = false;
        for (const Category::Entry &entry : std::as_const(category.entries)) {
            allChecked = allChecked && entry.checked;
            anyChecked = anyChecked || entry.checked;
        }
        category.check = allChecked ? Qt::Checked : (anyChecked ? Qt::PartiallyChecked : Qt::Unchecked);
    }

    rebuild();
    updateCheckedSize();
}

void CleanerModel::clean()
{
    QStringList paths;
    for (const Category &category : std::as_const(m_categories)) {
        if (category.check == Qt::Unchecked) {
            continue;
        }
        if (category.check == Qt::Checked) {
            for (const Category::Entry &entry : category.entries) {
                paths.append(entry.path);
            }
        } else {
            for (const Category::Entry &entry : category.entries) {
                if (entry.checked) {
                    paths.append(entry.path);
                }
            }
        }
    }

    m_cleaner.clean(paths);
}

void CleanerModel::rebuild()
{
    QVector<Row> rows;
    for (int i = 0; i < m_categories.size(); ++i) {
        rows.append(Row { i, -1 });
        if (!m_categories.at(i).expanded) {
            continue;
        }
        for (int j = 0; j < m_categories.at(i).entries.size(); ++j) {
            rows.append(Row { i, j });
        }
    }

    beginResetModel();
    m_rows = rows;
    endResetModel();
}

void CleanerModel::updateCheckedSize()
{
    qulonglong total = 0;
    for (const Category &category : std::as_const(m_categories)) {
        for (const Category::Entry &entry : category.entries) {
            if (entry.checked) {
                total += entry.size;
            }
        }
    }

    if (total == m_checkedSize) {
        return;
    }
    m_checkedSize = total;
    Q_EMIT checkedSizeChanged();
}

// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "cleaner_model.h"

#include <QCoreApplication>

#include <utility>

#include <KConfigGroup>
#include <KSharedConfig>

namespace
{
constexpr auto selectionGroup = "Cleaner";
constexpr auto selectionKey = "SelectedPaths";
}

CleanerModel::CleanerModel(QObject *parent) :
    QAbstractListModel(parent)
{
    loadSelection();

    if (QCoreApplication::instance()) {
        connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &CleanerModel::saveSelection);
    }

    connect(&m_cleaner, &Cleaner::scanningChanged, this, &CleanerModel::scanningChanged);
    connect(&m_cleaner, &Cleaner::scanned, this, [this](const QVariantList &categories) {
        m_categories.clear();
        for (const QVariant &variant : categories) {
            const QVariantMap categoryMap = variant.toMap();

            Category category;
            category.title = categoryMap.value(QStringLiteral("title")).toString();
            category.size = categoryMap.value(QStringLiteral("size")).toULongLong();
            category.action = categoryMap.value(QStringLiteral("action")).toString();

            const QVariantList entries = categoryMap.value(QStringLiteral("entries")).toList();
            category.entries.reserve(entries.size());
            for (const QVariant &entryVariant : entries) {
                const QVariantMap entryMap = entryVariant.toMap();
                Category::Entry entry;
                entry.title = entryMap.value(QStringLiteral("title")).toString();
                entry.path = entryMap.value(QStringLiteral("path")).toString();
                entry.size = entryMap.value(QStringLiteral("size")).toULongLong();
                entry.root = entryMap.value(QStringLiteral("root")).toBool();
                entry.checked = m_checkedPaths.contains(entry.path);
                category.entries.append(entry);
            }

            bool allChecked = !category.entries.isEmpty();
            bool anyChecked = false;
            for (const Category::Entry &entry : std::as_const(category.entries)) {
                allChecked = allChecked && entry.checked;
                anyChecked = anyChecked || entry.checked;
            }
            if (!category.action.isEmpty()) {
                // Whole-tree categories have no child entries; the remembered
                // selection is the only source of truth for their state.
                anyChecked = m_checkedPaths.contains(category.action);
                allChecked = anyChecked;
            }
            category.check = allChecked ? Qt::Checked : (anyChecked ? Qt::PartiallyChecked : Qt::Unchecked);

            m_categories.append(category);
        }

        rebuild();
        updateCheckedSize();
    });
    connect(&m_cleaner, &Cleaner::cleaned, this, [this](int count, qulonglong freedBytes, const QString &error) {
        m_cleaning = false;
        Q_EMIT cleaningChanged();
        m_lastFreedBytes = freedBytes;
        m_lastRemovedCount = count;
        m_lastError = error;
        Q_EMIT lastResultChanged();
        if (error.isEmpty()) {
            m_checkedPaths.clear();
            saveSelection();
        }
        if (error.isEmpty()) {
            scan();
        }
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
            return std::to_underlying(category.check);
        }
        const Qt::CheckState state = category.entries.at(row.entry).checked ? Qt::Checked : Qt::Unchecked;
        return std::to_underlying(state);
    }
    if (role == RootRole) {
        return row.entry < 0 ? false : category.entries.at(row.entry).root;
    }
    if (role == IsCategoryRole) {
        return row.entry < 0;
    }
    return {};
}

QHash<int, QByteArray> CleanerModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { TitleRole, "title" },
        { SizeRole, "size" },
        { DepthRole, "depth" },
        { ExpandableRole, "expandable" },
        { ExpandedRole, "expanded" },
        { CheckedRole, "checkState" },
        { RootRole, "root" },
        { IsCategoryRole, "isCategory" },
    };
    return roles;
}

bool CleanerModel::scanning() const
{
    return m_cleaner.scanning();
}

bool CleanerModel::cleaning() const
{
    return m_cleaning;
}

qulonglong CleanerModel::lastFreedBytes() const
{
    return m_lastFreedBytes;
}

int CleanerModel::lastRemovedCount() const
{
    return m_lastRemovedCount;
}

QString CleanerModel::lastError() const
{
    return m_lastError;
}

qulonglong CleanerModel::checkedSize() const
{
    return m_checkedSize;
}

bool CleanerModel::hasCheckedItems() const
{
    return m_hasCheckedItems;
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
    const Row r = m_rows.at(row);
    if (r.entry >= 0) {
        return;
    }

    Category &category = m_categories[r.category];
    const int entryCount = category.entries.size();

    if (category.expanded) {
        if (entryCount > 0) {
            beginRemoveRows(QModelIndex(), row + 1, row + entryCount);
            m_rows.remove(row + 1, entryCount);
            endRemoveRows();
        }
        category.expanded = false;
        Q_EMIT dataChanged(index(row), index(row), { ExpandedRole });
        return;
    }

    category.expanded = true;
    Q_EMIT dataChanged(index(row), index(row), { ExpandedRole });
    if (entryCount == 0) {
        return;
    }

    beginInsertRows(QModelIndex(), row + 1, row + entryCount);
    for (int i = 0; i < entryCount; ++i) {
        m_rows.insert(row + 1 + i, Row { r.category, i });
    }
    endInsertRows();
}

void CleanerModel::setChecked(int row, bool checked)
{
    if (row < 0 || row >= m_rows.size()) {
        return;
    }

    const Row r = m_rows.at(row);
    Category &category = m_categories[r.category];

    if (r.entry < 0) {
        category.check = checked ? Qt::Checked : Qt::Unchecked;
        for (Category::Entry &entry : category.entries) {
            entry.checked = checked;
            if (checked) {
                m_checkedPaths.insert(entry.path);
            } else {
                m_checkedPaths.remove(entry.path);
            }
        }
        if (!category.action.isEmpty()) {
            if (checked) {
                m_checkedPaths.insert(category.action);
            } else {
                m_checkedPaths.remove(category.action);
            }
        }
        if (category.expanded && !category.entries.isEmpty()) {
            Q_EMIT dataChanged(index(row + 1), index(row + category.entries.size()), { CheckedRole });
        }
    } else {
        Category::Entry &entry = category.entries[r.entry];
        entry.checked = checked;
        if (checked) {
            m_checkedPaths.insert(entry.path);
        } else {
            m_checkedPaths.remove(entry.path);
        }

        bool allChecked = !category.entries.isEmpty();
        bool anyChecked = false;
        for (const Category::Entry &current : std::as_const(category.entries)) {
            allChecked = allChecked && current.checked;
            anyChecked = anyChecked || current.checked;
        }
        category.check = allChecked ? Qt::Checked : (anyChecked ? Qt::PartiallyChecked : Qt::Unchecked);
    }

    Q_EMIT dataChanged(index(row), index(row), { CheckedRole });
    saveSelection();
    updateCheckedSize();
}

void CleanerModel::clean()
{
    if (m_cleaning) {
        return;
    }

    QStringList paths;
    QStringList orphanPackages;
    QVariantMap sizes;
    bool vacuumJournal = false;

    const auto collect = [&paths, &orphanPackages, &sizes, &vacuumJournal](const CleanerModel::Category::Entry &entry) {
        if (entry.path.startsWith(QLatin1String("journal:"))) {
            vacuumJournal = true;
        } else if (entry.path.startsWith(QLatin1String("pkg:"))) {
            const QString package = entry.path.mid(4).trimmed();
            if (!package.isEmpty()) {
                orphanPackages.append(package);
            }
        } else {
            paths.append(entry.path);
            sizes.insert(entry.path, QVariant::fromValue<qulonglong>(entry.size));
        }
    };

    for (const Category &category : std::as_const(m_categories)) {
        if (category.check == Qt::Unchecked) {
            continue;
        }

        if (!category.action.isEmpty()) {
            paths.append(category.action);
            sizes.insert(category.action, QVariant::fromValue<qulonglong>(category.size));
            continue;
        }

        if (category.check == Qt::Checked) {
            for (const Category::Entry &entry : category.entries) {
                collect(entry);
            }
        } else {
            for (const Category::Entry &entry : category.entries) {
                if (entry.checked) {
                    collect(entry);
                }
            }
        }
    }

    m_cleaning = true;
    Q_EMIT cleaningChanged();
    m_cleaner.clean(paths, sizes, orphanPackages, vacuumJournal);
}

void CleanerModel::rebuild()
{
    QList<Row> rows;
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
    bool hasChecked = false;
    for (const Category &category : std::as_const(m_categories)) {
        if (!category.action.isEmpty()) {
            if (category.check == Qt::Checked) {
                total += category.size;
                hasChecked = true;
            }
            continue;
        }
        for (const Category::Entry &entry : category.entries) {
            if (entry.checked) {
                total += entry.size;
                hasChecked = true;
            }
        }
    }

    if (total != m_checkedSize) {
        m_checkedSize = total;
        Q_EMIT checkedSizeChanged();
    }
    if (hasChecked != m_hasCheckedItems) {
        m_hasCheckedItems = hasChecked;
        Q_EMIT hasCheckedItemsChanged();
    }
}

void CleanerModel::loadSelection()
{
    const KConfigGroup group(KSharedConfig::openConfig(), QLatin1String(selectionGroup));
    const QStringList paths = group.readEntry(QLatin1String(selectionKey), QStringList());
    m_checkedPaths = QSet<QString>(paths.begin(), paths.end());
}

void CleanerModel::saveSelection()
{
    KConfigGroup group(KSharedConfig::openConfig(), QLatin1String(selectionGroup));
    group.writeEntry(QLatin1String(selectionKey), QStringList(m_checkedPaths.begin(), m_checkedPaths.end()));
    group.sync();
}

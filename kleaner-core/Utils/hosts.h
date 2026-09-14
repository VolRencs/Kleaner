// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QSet>
#include <QStringList>
#include <QVariantList>

class Hosts : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList entries READ entriesProperty NOTIFY entriesChanged)

  public:
    explicit Hosts(QObject *parent = nullptr);

    QVariantList entriesProperty() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE QVariantList entries() const;
    Q_INVOKABLE void setEntries(const QVariantList &entries);
    Q_INVOKABLE void save();

  Q_SIGNALS:
    void entriesChanged();
    void loaded(const QVariantList &entries);
    void saved(bool ok, const QString &error);

  private:
    QStringList m_lines;
    QSet<int> m_hostLines;
    QVariantList m_entries;
};

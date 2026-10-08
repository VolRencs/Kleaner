// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QSet>
#include <QStringList>
#include <QVariantList>

#include <QtQmlIntegration/qqmlintegration.h>

class Hosts : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Hosts)
    QML_SINGLETON
    QML_UNCREATABLE("Provided by the application")
    Q_PROPERTY(QVariantList entries READ entriesProperty NOTIFY entriesChanged)

  public:
    explicit Hosts(QObject *parent = nullptr);

    [[nodiscard]] QVariantList entriesProperty() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void setEntries(const QVariantList &entries);
    Q_INVOKABLE void save();

  Q_SIGNALS:
    void entriesChanged();
    void saved(bool ok, const QString &error);

  private:
    [[nodiscard]] QVariantList parseEntries() const;

    QStringList m_lines;
    QSet<int> m_hostLines;
    QVariantList m_entries;
    bool m_loaded = false;
    qint64 m_lastModified = 0;
};

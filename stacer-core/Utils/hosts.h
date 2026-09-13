#pragma once

#include <QObject>
#include <QSet>
#include <QStringList>
#include <QVariantList>

class Hosts : public QObject
{
    Q_OBJECT

  public:
    explicit Hosts(QObject *parent = nullptr);

    Q_INVOKABLE void reload();
    Q_INVOKABLE QVariantList entries() const;
    Q_INVOKABLE void save(const QVariantList &entries);

  Q_SIGNALS:
    void loaded(const QVariantList &entries);
    void saved(bool ok, const QString &error);

  private:
    QStringList m_lines;
    QSet<int> m_hostLines;
};

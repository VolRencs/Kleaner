#pragma once

#include <QObject>

class Format : public QObject
{
    Q_OBJECT

  public:
    explicit Format(QObject *parent = nullptr);

    Q_INVOKABLE QString bytes(qulonglong value) const;
    Q_INVOKABLE QString percent(double value, int decimals = 1) const;
    Q_INVOKABLE QString duration(qulonglong seconds) const;
};

#pragma once

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QVector>

struct Process {
    int pid = 0;
    QString name;
    QString user;
    QChar state;
    double cpu = 0.0;
    double mem = 0.0;
    qulonglong rss = 0;
    qulonglong vsize = 0;
    int nice = 0;
    QString cmd;
};

class ProcessInfo : public QObject
{
    Q_OBJECT

  public:
    explicit ProcessInfo(QObject *parent = nullptr);

    QVector<Process> read();

    Q_INVOKABLE bool killProcess(int pid, bool force);

  private:
    QMutex m_mutex;
    bool m_hasBaseline = false;
    quint64 m_previousTotal = 0;
    QHash<int, quint64> m_previousCpu;
    qulonglong m_memoryTotal = 0;
};

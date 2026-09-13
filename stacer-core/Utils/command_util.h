#ifndef COMMAND_UTIL_H
#define COMMAND_UTIL_H

#include <QStringList>

#include "stacer-core_global.h"

class STACERCORESHARED_EXPORT CommandUtil
{
  public:
    static QString sudoExec(const QString &cmd, QStringList args = QStringList(), QByteArray data = QByteArray());
    static QString exec(const QString &cmd, QStringList args = QStringList(), QByteArray data = QByteArray(), bool checkExitCode = false);
    static bool isExecutable(const QString &cmd);
    static bool isAptRpm();
};

#endif // COMMAND_UTIL_H

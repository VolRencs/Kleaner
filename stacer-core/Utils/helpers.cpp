#include "helpers.h"

#include <QHash>
#include <QMutex>

#include <pwd.h>
#include <unistd.h>

namespace
{
QMutex &userCacheMutex()
{
    static QMutex mutex;
    return mutex;
}

QHash<uid_t, QString> &userCache()
{
    static QHash<uid_t, QString> cache;
    return cache;
}
}

namespace Helpers
{

QString userName(uid_t uid)
{
    {
        QMutexLocker locker(&userCacheMutex());
        const auto it = userCache().constFind(uid);
        if (it != userCache().constEnd()) {
            return it.value();
        }
    }

    QString name = QString::number(uid);

    struct passwd pwd;
    struct passwd *result = nullptr;
    char buffer[4096];
    if (getpwuid_r(uid, &pwd, buffer, sizeof(buffer), &result) == 0 && result) {
        name = QString::fromLocal8Bit(result->pw_name);
    }

    QMutexLocker locker(&userCacheMutex());
    userCache().insert(uid, name);
    return name;
}

int pageSizeKiB()
{
    static const int size = static_cast<int>(sysconf(_SC_PAGESIZE) / 1024);
    return size > 0 ? size : 4;
}

double clockTicks()
{
    static const double ticks = [] {
        const long value = sysconf(_SC_CLK_TCK);
        return value > 0 ? static_cast<double>(value) : 100.0;
    }();
    return ticks;
}

}

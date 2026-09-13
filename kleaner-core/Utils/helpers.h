#pragma once

#include <QString>

#include <sys/types.h>

namespace Helpers
{
QString userName(uid_t uid);
int pageSizeKiB();
double clockTicks();
}

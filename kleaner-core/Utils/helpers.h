// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QString>

#include <sys/types.h>

namespace Helpers
{
QString userName(uid_t uid);
int pageSizeKiB();
double clockTicks();
}

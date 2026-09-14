// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QString>

#include <sys/types.h>

namespace Helpers
{
[[nodiscard]] QString userName(uid_t uid);
[[nodiscard]] int pageSizeKiB();
}

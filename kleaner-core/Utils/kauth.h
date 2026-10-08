// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QString>

class KJob;

namespace Kauth
{
// Human-readable text for a finished KAuth job. KAuth reports some failures,
// such as denied authorization, with an empty error text.
[[nodiscard]] QString errorText(const KJob *job);
}

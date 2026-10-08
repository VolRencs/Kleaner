// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

#include "kauth.h"

#include <QCoreApplication>

#include <KAuth/ActionReply>
#include <KJob>

namespace Kauth
{
QString errorText(const KJob *job)
{
    if (!job || job->error() == KJob::NoError) {
        return {};
    }

    const QString text = job->errorString().trimmed();
    if (!text.isEmpty()) {
        return text;
    }

    switch (job->error()) {
    case KAuth::ActionReply::AuthorizationDeniedError:
        return QCoreApplication::translate("KAuth", "Authentication was denied");
    case KAuth::ActionReply::UserCancelledError:
        return QCoreApplication::translate("KAuth", "Authentication was cancelled");
    default:
        return QCoreApplication::translate("KAuth", "The privileged operation failed");
    }
}
}

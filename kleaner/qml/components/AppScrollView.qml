// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import Kleaner

Controls.ScrollView {
    id: control

    clip: true
    Controls.ScrollBar.horizontal.policy: Controls.ScrollBar.AlwaysOff

    // The platform style only positions its own scrollbars, so a custom
    // vertical bar has to be placed explicitly. It is re-parented to the page
    // so that it can sit on the window edge instead of being clipped by the
    // ScrollView, whose right edge is inset by the page margins.
    Controls.ScrollBar.vertical: AppScrollBar {
        parent: control
        z: 1
        x: control.x + control.width - width + Design.pagePadding
        y: control.y + control.topPadding
        height: control.availableHeight

        Component.onCompleted: parent = control.parent
    }
}

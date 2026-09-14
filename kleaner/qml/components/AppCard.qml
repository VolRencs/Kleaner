// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import Kleaner

Controls.Pane {
    id: root

    padding: Design.cardPadding

    background: Rectangle {
        color: Design.surface
        radius: Design.radius
        border.width: 1
        border.color: Design.border
    }
}

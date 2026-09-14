// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import Kleaner

Controls.Label {
    id: root

    property color badgeColor: Design.textMuted

    leftPadding: 8
    rightPadding: 8
    topPadding: 2
    bottomPadding: 2
    font.pointSize: Design.tinyFontSize
    font.weight: Font.DemiBold
    color: root.badgeColor

    background: Rectangle {
        radius: height / 2
        color: Design.alpha(root.badgeColor, 0.14)
    }
}

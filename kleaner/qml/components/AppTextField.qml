// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls

Controls.TextField {
    id: root

    implicitHeight: Design.controlHeight
    leftPadding: 12
    rightPadding: 12
    color: root.enabled ? Design.text : Design.textFaint
    placeholderTextColor: Design.textFaint
    selectionColor: Design.accent
    selectedTextColor: Design.accentText
    font.pointSize: Design.baseFontSize

    background: Rectangle {
        radius: Design.radiusSmall
        color: Design.surface
        border.width: 1
        border.color: root.activeFocus ? Design.accent
                                       : root.hovered ? Design.borderStrong
                                                      : Design.border

        Behavior on border.color {
            ColorAnimation { duration: 120 }
        }
    }
}

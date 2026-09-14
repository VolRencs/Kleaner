// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls

Controls.Switch {
    id: root

    spacing: 8
    font.pointSize: Design.baseFontSize

    indicator: Rectangle {
        implicitWidth: 42
        implicitHeight: 24
        x: root.leftPadding
        y: parent.height / 2 - height / 2
        radius: height / 2
        color: root.checked ? (root.hovered ? Design.accentHover : Design.accent) : Design.surfaceActive
        border.width: root.checked ? 0 : 1
        border.color: root.hovered ? Design.borderStrong : Design.border

        Behavior on color {
            ColorAnimation { duration: 140 }
        }

        Rectangle {
            y: 3
            x: root.checked ? parent.width - width - 3 : 3
            width: 18
            height: 18
            radius: 9
            color: root.checked ? Design.accentText : Design.textMuted

            Behavior on x {
                NumberAnimation {
                    duration: 140
                    easing.type: Easing.OutCubic
                }
            }
            Behavior on color {
                ColorAnimation { duration: 140 }
            }
        }
    }

    contentItem: Controls.Label {
        leftPadding: root.indicator.width + root.spacing
        visible: root.text.length > 0
        text: root.text
        color: root.enabled ? Design.text : Design.textFaint
        font: root.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}

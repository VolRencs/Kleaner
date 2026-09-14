// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls

Controls.ScrollBar {
    id: root

    padding: 2
    minimumSize: 0.15

    background: Rectangle {
        color: "transparent"
    }

    contentItem: Rectangle {
        implicitWidth: 6
        implicitHeight: 6
        radius: Math.min(width, height) / 2
        color: root.pressed ? Design.accent
                            : root.hovered ? Design.borderStrong
                                           : Design.border
        opacity: root.active ? 1.0 : 0.6

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }
}

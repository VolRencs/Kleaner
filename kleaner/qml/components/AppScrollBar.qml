// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Templates as T
import Kleaner

// Standalone overlay scrollbar. It deliberately does not inherit the platform
// style implementation: the style hardcodes its own geometry, animations and
// visibility rules that fight with the app's overlay layout.
T.ScrollBar {
    id: root

    implicitWidth: 8
    implicitHeight: 8

    padding: 1
    // Keep the handle clear of the rounded corners of the surrounding card.
    topPadding: root.orientation === Qt.Vertical ? 6 : 1
    bottomPadding: root.orientation === Qt.Vertical ? 6 : 1
    minimumSize: 0.1
    policy: T.ScrollBar.AsNeeded

    // Only reserve space when there is something to scroll.
    visible: policy === T.ScrollBar.AlwaysOn || (size > 0.0 && size < 1.0)
    opacity: active || pressed ? 1.0 : 0.55

    background: null

    contentItem: Rectangle {
        implicitWidth: 6
        implicitHeight: 6
        radius: Math.min(width, height) / 2
        color: root.pressed ? Design.accent
                            : root.hovered ? Design.borderStrong
                                           : Design.border

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: 150 }
    }
}

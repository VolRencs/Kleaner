// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami
import Kleaner

// Round icon-only button with a Kirigami glyph. The stock Basic-style Button
// draws a rectangular background and focus frame, which looks out of place in
// the rounded Kleaner interface; this control keeps the highlight and the
// focus ring circular.
Controls.ToolButton {
    id: root

    property alias iconSource: glyph.source
    // Allows chevrons and arrows to rotate into their expanded direction.
    property real iconRotation: 0
    property color iconColor: Design.textMuted
    property color highlightColor: Design.accent

    implicitWidth: 28
    implicitHeight: 28
    display: Controls.AbstractButton.IconOnly
    hoverEnabled: true
    activeFocusOnTab: true

    Accessible.name: root.text
    Accessible.role: Accessible.Button

    background: Rectangle {
        radius: width / 2
        color: root.down ? Design.accentSoft
                         : root.hovered || root.visualFocus ? Design.surfaceHover
                                                            : Design.surfaceHoverClear
        border.width: root.visualFocus ? 2 : 0
        border.color: Design.accent

        Behavior on color {
            ColorAnimation { duration: Design.durationFast }
        }
    }

    contentItem: Kirigami.Icon {
        id: glyph

        implicitWidth: Design.iconMedium
        implicitHeight: Design.iconMedium
        color: !root.enabled ? Design.textFaint
                             : root.hovered || root.visualFocus ? root.highlightColor
                                                                : root.iconColor
        smooth: true
        rotation: root.iconRotation

        Behavior on rotation {
            NumberAnimation {
                duration: Design.durationNormal
                easing.type: Easing.OutCubic
            }
        }

        Behavior on color {
            ColorAnimation { duration: Design.durationFast }
        }
    }

    Controls.ToolTip.visible: root.hovered && root.text.length > 0
    Controls.ToolTip.text: root.text
    Controls.ToolTip.delay: 500
}

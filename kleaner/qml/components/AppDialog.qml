// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import Kleaner

Controls.Dialog {
    id: root

    modal: true
    anchors.centerIn: parent
    padding: Design.space20
    spacing: Design.space12

    // Fixed width: binding it to implicitWidth makes wrapping content items
    // feed their own width back into the dialog and produces a binding loop.
    width: 420
    height: Math.max(140, implicitHeight)

    header: Controls.Label {
        visible: root.title.length > 0
        leftPadding: Design.space20
        rightPadding: Design.space20
        topPadding: Design.space20
        bottomPadding: 0
        text: root.title
        color: Design.text
        font.pointSize: Design.baseFontSize * 1.2
        font.weight: Font.DemiBold
        elide: Text.ElideRight
    }

    background: Rectangle {
        color: Design.elevated
        radius: Design.radiusLarge
        border.width: 1
        border.color: Design.borderStrong
    }
}

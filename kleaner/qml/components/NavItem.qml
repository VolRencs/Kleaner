// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

// Focusable, keyboard-operable navigation entry.
Controls.ItemDelegate {
    id: root

    property string iconName
    property bool selected: false
    property bool compact: false

    implicitHeight: 40
    leftPadding: 12
    rightPadding: 12
    topPadding: 0
    bottomPadding: 0
    hoverEnabled: true
    activeFocusOnTab: true

    Accessible.name: root.text
    Accessible.role: Accessible.PageTab
    Accessible.checked: root.selected

    background: Rectangle {
        radius: Design.radiusItem
        color: root.selected ? Design.accentSoft
                             : (root.hovered || root.activeFocus) ? Design.surfaceHover
                                                                  : Design.surfaceHoverClear

        Behavior on color {
            ColorAnimation { duration: Design.durationNormal }
        }
    }

    contentItem: RowLayout {
        spacing: 12

        Kirigami.Icon {
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: Design.iconLarge
            implicitHeight: Design.iconLarge
            source: root.iconName
            color: root.selected ? Design.accent : Design.textMuted
        }

        Controls.Label {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            visible: !root.compact
            text: root.text
            color: root.selected ? Design.text : Design.textMuted
            font.pointSize: Design.baseFontSize
            font.weight: root.selected ? Font.DemiBold : Font.Normal
            elide: Text.ElideRight
        }
    }

    Controls.ToolTip.visible: root.compact && root.hovered
    Controls.ToolTip.text: root.text
    Controls.ToolTip.delay: 400
}

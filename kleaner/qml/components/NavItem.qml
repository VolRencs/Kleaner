// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami

Item {
    id: root

    property string iconName
    property string text
    property bool selected: false
    property bool compact: false

    signal clicked()

    implicitHeight: 40

    Rectangle {
        id: background

        anchors.fill: parent
        radius: Design.radiusSmall
        color: root.selected ? Design.accentSoft
                             : mouseArea.containsMouse ? Design.surfaceHover
                                                       : "transparent"

        Behavior on color {
            ColorAnimation { duration: 120 }
        }

        Row {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 12

            Kirigami.Icon {
                anchors.verticalCenter: parent.verticalCenter
                width: 20
                height: 20
                source: root.iconName
                color: root.selected ? Design.accent : Design.textMuted
            }

            Controls.Label {
                anchors.verticalCenter: parent.verticalCenter
                visible: !root.compact
                text: root.text
                color: root.selected ? Design.text : Design.textMuted
                font.pointSize: Design.baseFontSize
                font.weight: root.selected ? Font.DemiBold : Font.Normal
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    Controls.ToolTip.visible: root.compact && mouseArea.containsMouse
    Controls.ToolTip.text: root.text
    Controls.ToolTip.delay: 400
}

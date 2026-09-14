// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami
import Kleaner

Item {
    id: root

    property string text
    property bool sorted: false
    property bool descending: false
    property bool alignLeft: false
    property bool alignRight: false

    signal clicked()

    implicitWidth: row.implicitWidth
    implicitHeight: 22

    Rectangle {
        anchors.fill: parent
        radius: Design.radiusItem
        color: hoverHandler.hovered ? Design.surfaceHover : "transparent"
    }

    Row {
        id: row

        anchors.verticalCenter: parent.verticalCenter
        x: {
            if (root.alignLeft) {
                return 4;
            }
            if (root.alignRight) {
                return Math.max(0, root.width - implicitWidth - 4);
            }
            return Math.max(0, (root.width - implicitWidth) / 2);
        }
        spacing: 4

        Controls.Label {
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: root.sorted ? Design.accent : Design.textFaint
            font.pointSize: Design.tinyFontSize
            font.weight: Font.DemiBold
        }

        Kirigami.Icon {
            anchors.verticalCenter: parent.verticalCenter
            visible: root.sorted
            width: 12
            height: 12
            source: root.descending ? "view-sort-descending" : "view-sort-ascending"
            color: Design.accent
        }
    }

    HoverHandler {
        id: hoverHandler
    }

    TapHandler {
        onTapped: root.clicked()
    }
}

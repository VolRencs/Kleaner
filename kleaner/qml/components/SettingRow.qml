// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import Kleaner

Item {
    id: root

    property string text
    property string description
    default property alias controlData: slot.data

    implicitHeight: layout.implicitHeight

    RowLayout {
        id: layout

        anchors.fill: parent
        spacing: Design.space16

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 2

            Controls.Label {
                Layout.fillWidth: true
                text: root.text
                color: Design.text
            }

            Controls.Label {
                Layout.fillWidth: true
                visible: root.description.length > 0
                text: root.description
                color: Design.textMuted
                font.pointSize: Design.smallFontSize
                wrapMode: Text.WordWrap
            }
        }

        RowLayout {
            id: slot

            Layout.alignment: Qt.AlignVCenter
            spacing: Design.space8
        }
    }
}

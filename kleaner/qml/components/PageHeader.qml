// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import Kleaner

Item {
    id: root

    property string title
    property string subtitle
    default property alias actions: actionsRow.data

    implicitHeight: headerRow.implicitHeight

    RowLayout {
        id: headerRow

        anchors.fill: parent
        spacing: Design.space16

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true
            spacing: 2

            Controls.Label {
                Layout.fillWidth: true
                text: root.title
                color: Design.text
                font.pointSize: Design.titleFontSize
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            Controls.Label {
                Layout.fillWidth: true
                visible: root.subtitle.length > 0
                text: root.subtitle
                color: Design.textMuted
                font.pointSize: Design.subtitleFontSize
                elide: Text.ElideRight
            }
        }

        RowLayout {
            id: actionsRow

            Layout.alignment: Qt.AlignVCenter
            spacing: Design.space8
        }
    }
}

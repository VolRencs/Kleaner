// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

Item {
    id: root

    property string iconName
    property string label
    property string value
    property color valueColor: Design.text

    implicitHeight: row.implicitHeight

    RowLayout {
        id: row

        anchors.fill: parent
        spacing: Design.space12

        Kirigami.Icon {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            source: root.iconName
            color: Design.textFaint
        }

        Controls.Label {
            Layout.preferredWidth: 120
            text: root.label
            color: Design.textMuted
            font.pointSize: Design.smallFontSize
            elide: Text.ElideRight
        }

        Controls.Label {
            Layout.fillWidth: true
            text: root.value
            color: root.valueColor
            font.pointSize: Design.smallFontSize
            elide: Text.ElideRight
        }
    }
}

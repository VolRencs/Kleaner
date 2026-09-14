// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami
import Kleaner

Controls.TextField {
    id: root

    implicitHeight: Design.controlHeight
    leftPadding: 36
    rightPadding: root.text.length > 0 ? 36 : 12
    color: root.enabled ? Design.text : Design.textFaint
    placeholderTextColor: Design.textFaint
    selectionColor: Design.accent
    selectedTextColor: Design.accentText
    font.pointSize: Design.baseFontSize

    background: Rectangle {
        radius: Design.radiusSmall
        color: Design.surface
        border.width: 1
        border.color: root.activeFocus ? Design.accent
                                       : root.hovered ? Design.borderStrong
                                                      : Design.border

        Behavior on border.color {
            ColorAnimation { duration: 120 }
        }
    }

    Kirigami.Icon {
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        width: 16
        height: 16
        source: "search"
        color: root.activeFocus ? Design.accent : Design.textFaint
    }

    AppButton {
        anchors.right: parent.right
        anchors.rightMargin: 4
        anchors.verticalCenter: parent.verticalCenter
        width: 28
        height: 28
        visible: root.text.length > 0
        flat: true
        display: Controls.AbstractButton.IconOnly
        icon.name: "edit-clear"
        text: qsTr("Clear")
        leftPadding: 6
        rightPadding: 6
        onClicked: {
            root.text = "";
            root.forceActiveFocus();
        }
    }
}

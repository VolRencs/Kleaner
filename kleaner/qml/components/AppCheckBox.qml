// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls

Controls.CheckBox {
    id: root

    // The stock implicit height ignores a textless content item, which would
    // leave no clickable area for checkbox-only rows (e.g. the cleaner list).
    implicitWidth: Math.max(root.indicator ? root.indicator.implicitWidth : 0,
                            root.contentItem ? root.contentItem.implicitWidth : 0)
    implicitHeight: Math.max(root.indicator ? root.indicator.implicitHeight : 0,
                             root.contentItem ? root.contentItem.implicitHeight : 0)

    spacing: 8
    font.pointSize: Design.baseFontSize

    indicator: Rectangle {
        implicitWidth: 20
        implicitHeight: 20
        x: root.leftPadding
        y: parent.height / 2 - height / 2
        radius: Design.radiusSmall
        color: root.checkState !== Qt.Unchecked ? Design.accent : Design.surface
        border.width: 1
        border.color: root.checkState !== Qt.Unchecked ? Design.accent
                                                       : root.hovered ? Design.borderStrong
                                                                      : Design.border

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
        Behavior on border.color {
            ColorAnimation { duration: 120 }
        }

        Canvas {
            anchors.centerIn: parent
            width: 12
            height: 12
            visible: root.checkState === Qt.Checked

            onPaint: {
                const ctx = getContext("2d");
                ctx.clearRect(0, 0, width, height);
                ctx.strokeStyle = Design.accentText;
                ctx.lineWidth = 2;
                ctx.lineCap = "round";
                ctx.lineJoin = "round";
                ctx.beginPath();
                ctx.moveTo(2, 6.5);
                ctx.lineTo(4.8, 9.2);
                ctx.lineTo(10.2, 2.8);
                ctx.stroke();
            }
        }

        Rectangle {
            anchors.centerIn: parent
            visible: root.checkState === Qt.PartiallyChecked
            width: 10
            height: 2
            radius: 1
            color: Design.accentText
        }
    }

    contentItem: Controls.Label {
        leftPadding: root.indicator.width + root.spacing
        text: root.text
        color: root.enabled ? Design.text : Design.textFaint
        font: root.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}

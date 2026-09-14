// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami

Controls.ComboBox {
    id: root

    implicitHeight: Design.controlHeight
    implicitWidth: 180
    leftPadding: 12
    rightPadding: 34
    font.pointSize: Design.baseFontSize

    contentItem: Controls.Label {
        text: root.displayText
        color: root.enabled ? Design.text : Design.textFaint
        font: root.font
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Kirigami.Icon {
        x: root.mirrored ? 10 : root.width - width - 10
        y: (root.height - height) / 2
        width: 16
        height: 16
        source: "arrow-down"
        color: root.hovered || root.down ? Design.text : Design.textMuted
    }

    background: Rectangle {
        radius: Design.radiusSmall
        color: root.down ? Design.surfaceActive : Design.surface
        border.width: 1
        border.color: root.activeFocus ? Design.accent
                                       : root.hovered ? Design.borderStrong
                                                      : Design.border

        Behavior on border.color {
            ColorAnimation { duration: 120 }
        }
    }

    delegate: Controls.ItemDelegate {
        id: itemDelegate

        required property var model
        required property int index

        width: root.width - 8
        height: 34
        leftPadding: 12
        rightPadding: 12
        topPadding: 0
        bottomPadding: 0
        hoverEnabled: true
        highlighted: root.highlightedIndex === index

        contentItem: Controls.Label {
            text: root.textRole.length > 0 ? model[root.textRole] : modelData
            color: itemDelegate.highlighted ? Design.text : Design.textMuted
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            radius: Design.radiusSmall
            color: itemDelegate.highlighted ? Design.accentSoft
                                            : itemDelegate.hovered ? Design.surfaceHover
                                                                   : "transparent"
        }
    }

    popup: Controls.Popup {
        y: root.height + 4
        width: root.width
        padding: 4
        implicitHeight: Math.min(contentItem.implicitHeight + padding * 2, root.Window.height - 40)
        font: root.font

        background: Rectangle {
            radius: Design.radiusSmall
            color: Design.elevated
            border.width: 1
            border.color: Design.borderStrong
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex
            boundsBehavior: Flickable.StopAtBounds
        }
    }
}

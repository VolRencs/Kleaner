// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls

import org.kde.kirigami as Kirigami
import Kleaner

Controls.ComboBox {
    id: root

    implicitHeight: Design.controlHeight
    implicitWidth: 180
    leftPadding: 12
    rightPadding: 34
    font.pointSize: Design.baseFontSize

    // Index of the first entry whose `value` property matches, or -1. Used to
    // drive the current index from settings without breaking on user input.
    function indexOfValue(value) {
        const items = root.model;
        if (items === undefined || items === null) {
            return -1;
        }
        for (let i = 0; i < items.length; ++i) {
            const item = items[i];
            if (item !== null && item !== undefined && item.value === value) {
                return i;
            }
        }
        return -1;
    }

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
            ColorAnimation { duration: Design.durationNormal }
        }
    }

    delegate: Controls.ItemDelegate {
        id: itemDelegate

        required property int index

        // The popup pads its content, so the delegate must match the list
        // viewport instead of the ComboBox width, otherwise clip cuts off the
        // right edge of the rounded highlight.
        width: ListView.view ? ListView.view.width : root.width
        height: 34
        leftPadding: 12
        rightPadding: 12
        topPadding: 0
        bottomPadding: 0
        hoverEnabled: true
        highlighted: root.highlightedIndex === index

        contentItem: Controls.Label {
            text: {
                const entries = root.model;
                if (entries === undefined || entries === null) {
                    return "";
                }
                const entry = entries[itemDelegate.index];
                if (entry === undefined || entry === null) {
                    return "";
                }
                return root.textRole.length > 0 ? entry[root.textRole] : entry;
            }
            color: itemDelegate.highlighted ? Design.text : Design.textMuted
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            color: "transparent"

            Rectangle {
                anchors.fill: parent
                anchors.margins: Design.itemInset
                radius: Design.radiusItem
                color: itemDelegate.highlighted ? Design.accentSoft
                                                : itemDelegate.hovered ? Design.surfaceHover
                                                                       : Design.surfaceHoverClear

                Behavior on color {
                    ColorAnimation { duration: Design.durationNormal }
                }
            }
        }
    }

    popup: Controls.Popup {
        id: popup

        readonly property real popupHeight: Math.min((contentItem ? contentItem.implicitHeight : 0) + padding * 2, root.Window.height - 40)
        readonly property real sceneY: root.mapToItem(null, 0, 0).y
        // The popup position is relative to the combo box. Flip it above when
        // it would run past the bottom of the window and there is room on top;
        // otherwise clamp the height so it always fits on screen.
        readonly property bool openAbove: sceneY + root.height + 4 + popupHeight > root.Window.height
                                          && sceneY - popupHeight - 4 >= 0

        y: openAbove ? -popupHeight - 4 : root.height + 4
        height: popupHeight
        width: root.width
        padding: 4
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

            Controls.ScrollBar.vertical: AppScrollBar {}
        }
    }
}

// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

Controls.Button {
    id: root

    // Replaces the icon with a Kirigami busy indicator while an action runs.
    property bool spinning: false
    // Highlighted buttons turn red instead of blue for destructive actions.
    property bool destructive: false

    implicitHeight: Design.controlHeight
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    leftPadding: root.display === Controls.AbstractButton.IconOnly ? 8 : 14
    rightPadding: root.display === Controls.AbstractButton.IconOnly ? 8 : 14
    topPadding: 4
    bottomPadding: 4
    spacing: 8

    icon.width: 16
    icon.height: 16

    readonly property color highlightColor: root.destructive ? Design.negative : Design.accent
    // Tonal primary button: a tinted surface instead of a fully saturated fill,
    // which is too bright against the dark window.
    readonly property color highlightBase: Design.alpha(root.highlightColor, 0.16)
    readonly property color highlightHover: Design.alpha(root.highlightColor, 0.24)
    readonly property color highlightPressed: Design.alpha(root.highlightColor, 0.32)

    readonly property color foregroundColor: {
        if (!root.enabled) {
            return Design.textFaint;
        }
        if (root.highlighted) {
            return root.highlightColor;
        }
        if (root.flat) {
            return root.hovered || root.down ? Design.accent : Design.textMuted;
        }
        return Design.text;
    }

    readonly property color backgroundColor: {
        if (root.highlighted) {
            if (!root.enabled) {
                return Design.alpha(root.highlightColor, 0.08);
            }
            return root.down ? root.highlightPressed : root.hovered ? root.highlightHover : root.highlightBase;
        }
        if (root.flat) {
            return root.down ? Design.alpha(Design.accent, 0.22)
                             : root.hovered ? Design.alpha(Design.accent, 0.12)
                                            : Design.accentClear;
        }
        if (root.down) {
            return Design.surfaceActive;
        }
        if (root.hovered) {
            return Design.surfaceActive;
        }
        return Design.surfaceHover;
    }

    readonly property color borderColor: {
        if (root.highlighted || root.flat) {
            return "transparent";
        }
        if (root.activeFocus) {
            return Design.accent;
        }
        return root.hovered ? Design.borderStrong : Design.border;
    }

    contentItem: Item {
        implicitWidth: contentRow.implicitWidth
        implicitHeight: contentRow.implicitHeight

        RowLayout {
            id: contentRow

            // Anchored instead of filling: the platform style stretches the
            // content item to the whole button, which would left-align the row
            // and push icon-only buttons off-center.
            anchors.centerIn: parent
            width: Math.min(contentRow.implicitWidth, parent.width)
            spacing: root.spacing

            AppBusyIndicator {
                Layout.alignment: Qt.AlignVCenter
                visible: root.spinning
                         && root.display !== Controls.AbstractButton.TextOnly
                         && root.icon.name.length > 0
                implicitWidth: root.icon.width
                implicitHeight: root.icon.height
                color: root.foregroundColor
                running: root.spinning && root.visible
            }

            Kirigami.Icon {
                Layout.alignment: Qt.AlignVCenter
                visible: !root.spinning
                         && root.display !== Controls.AbstractButton.TextOnly
                         && root.icon.name.length > 0
                implicitWidth: root.icon.width
                implicitHeight: root.icon.height
                source: root.icon.name
                color: root.foregroundColor
                smooth: true
            }

            Controls.Label {
                Layout.alignment: Qt.AlignVCenter
                visible: root.display !== Controls.AbstractButton.IconOnly && root.text.length > 0
                text: root.text
                color: root.foregroundColor
                font: root.font
                elide: Text.ElideRight
            }
        }
    }

    background: Rectangle {
        radius: Design.radiusSmall
        color: root.backgroundColor
        border.width: root.highlighted || root.flat ? 0 : 1
        border.color: root.borderColor

        Behavior on color {
            ColorAnimation { duration: Design.durationFast }
        }
        Behavior on border.color {
            ColorAnimation { duration: Design.durationFast }
        }
    }

    Controls.ToolTip.visible: root.hovered && root.display === Controls.AbstractButton.IconOnly && root.text.length > 0
    Controls.ToolTip.text: root.text
    Controls.ToolTip.delay: 500
}

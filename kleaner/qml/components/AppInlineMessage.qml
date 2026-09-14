// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

// Inline message styled with the project's own tokens instead of the platform
// InlineMessage: rounded corners, Design colours and an AppButton close action.
// Positive and informational messages hide themselves, errors stay until the
// user closes them or triggers a new action.
Controls.Pane {
    id: root

    property int type: Kirigami.MessageType.Information
    property string text
    property int autoHideInterval: 6000

    readonly property bool persistent: root.type === Kirigami.MessageType.Error

    readonly property color accentColor: {
        switch (root.type) {
        case Kirigami.MessageType.Positive:
            return Design.positive;
        case Kirigami.MessageType.Warning:
            return Design.warning;
        case Kirigami.MessageType.Error:
            return Design.negative;
        default:
            return Design.accent;
        }
    }

    readonly property string iconName: {
        switch (root.type) {
        case Kirigami.MessageType.Positive:
            return "dialog-ok-apply";
        case Kirigami.MessageType.Warning:
            return "dialog-warning";
        case Kirigami.MessageType.Error:
            return "dialog-error";
        default:
            return "dialog-information";
        }
    }

    visible: false
    padding: Design.space8
    leftPadding: Design.space12
    rightPadding: Design.space4

    function show(message, messageType, autoHide) {
        root.type = messageType;
        root.text = message;
        root.visible = true;
        hideTimer.stop();
        const shouldAutoHide = autoHide !== undefined ? autoHide : !root.persistent;
        if (shouldAutoHide && root.autoHideInterval > 0) {
            hideTimer.restart();
        }
    }

    function showError(message) {
        root.show(message, Kirigami.MessageType.Error);
    }

    function showPositive(message) {
        root.show(message, Kirigami.MessageType.Positive);
    }

    function dismiss() {
        hideTimer.stop();
        root.visible = false;
    }

    Accessible.name: root.text
    Accessible.role: Accessible.Alert

    background: Rectangle {
        radius: Design.radiusSmall
        color: Qt.tint(Design.window, Design.alpha(root.accentColor, 0.16))
        border.width: 1
        border.color: Design.alpha(root.accentColor, 0.75)
    }

    contentItem: RowLayout {
        spacing: Design.space8

        Kirigami.Icon {
            Layout.alignment: Qt.AlignVCenter
            implicitWidth: Design.iconMedium
            implicitHeight: Design.iconMedium
            source: root.iconName
            color: root.accentColor
        }

        Controls.Label {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            text: root.text
            color: Design.text
            wrapMode: Text.WordWrap
            font.pointSize: Design.smallFontSize
        }

        AppButton {
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 28
            Layout.preferredHeight: 28
            flat: true
            display: Controls.AbstractButton.IconOnly
            icon.name: "dialog-close"
            text: qsTr("Close")
            leftPadding: 6
            rightPadding: 6
            onClicked: root.dismiss()
        }
    }

    Timer {
        id: hideTimer
        interval: root.autoHideInterval
        onTriggered: root.visible = false
    }
}

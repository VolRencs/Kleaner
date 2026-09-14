// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import Kleaner

// Confirmation dialog for destructive or irreversible actions. Built on the
// styled AppDialog so it keeps the exact Kleaner look; the primary action is
// only reachable after an explicit confirmation, Escape/close cancels.
AppDialog {
    id: root

    property string message
    property string confirmText: qsTr("Confirm")
    property string cancelText: qsTr("Cancel")
    property bool destructive: false

    signal confirmed()

    contentItem: Controls.Label {
        wrapMode: Text.Wrap
        text: root.message
        color: Design.text
    }

    footer: AppDialogFooter {
        AppButton {
            id: cancelButton
            Layout.fillWidth: true
            text: root.cancelText
            onClicked: root.close()
        }

        AppButton {
            Layout.fillWidth: true
            text: root.confirmText
            highlighted: true
            destructive: root.destructive
            onClicked: {
                root.close();
                root.confirmed();
            }
        }
    }

    onOpened: cancelButton.forceActiveFocus()
}

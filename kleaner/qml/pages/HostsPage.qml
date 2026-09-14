// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

pragma ComponentBehavior: Bound

Kirigami.Page {
    id: page

    padding: Design.pagePadding

    readonly property var hostEntries: Hosts.entries

    // Snapshot of the last loaded/saved state used to detect unsaved edits.
    property string savedState: ""

    readonly property bool dirty: page.savedState.length > 0
                                  && JSON.stringify(page.hostEntries) !== page.savedState

    function captureSavedState() {
        page.savedState = JSON.stringify(Hosts.entries);
    }

    function isValidAddress(text) {
        const value = text.trim();
        if (value.length === 0 || /\s/.test(value)) {
            return false;
        }
        const ipv4 = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
        const match = ipv4.exec(value);
        if (match) {
            for (let i = 1; i <= 4; ++i) {
                const octet = parseInt(match[i], 10);
                if (octet > 255) {
                    return false;
                }
            }
            return true;
        }
        return /^[0-9a-fA-F:]+$/.test(value) && value.includes(":");
    }

    function areValidNames(text) {
        const value = text.trim();
        if (value.length === 0) {
            return false;
        }
        const parts = value.split(/\s+/);
        for (let i = 0; i < parts.length; ++i) {
            if (!/^[A-Za-z0-9][A-Za-z0-9._-]*$/.test(parts[i])) {
                return false;
            }
        }
        return true;
    }

    Component.onCompleted: {
        Hosts.reload();
        page.captureSavedState();
    }

    Connections {
        target: Hosts

        function onSaved(ok, error) {
            if (ok) {
                page.captureSavedState();
                inlineMessage.showPositive(qsTr("/etc/hosts saved."));
            } else {
                inlineMessage.showError(error);
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Design.space16

        PageHeader {
            Layout.fillWidth: true
            title: qsTr("Hosts")
            subtitle: qsTr("Map hostnames to IP addresses in /etc/hosts")

            Badge {
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("%1 entries").arg(page.hostEntries.length)
                badgeColor: Design.textMuted
            }

            AppButton {
                text: qsTr("Add entry")
                icon.name: "list-add"
                highlighted: true
                onClicked: {
                    ipField.text = "";
                    namesField.text = "";
                    hostDialog.editingIndex = -1;
                    hostDialog.open();
                }
            }

            AppButton {
                text: qsTr("Save changes")
                icon.name: "document-save"
                highlighted: true
                enabled: page.dirty
                onClicked: Hosts.save()
            }
        }

        AppInlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
        }

        AppCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 0

            contentItem: Item {
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: Math.min(implicitWidth, parent.width - Design.space20 * 2)
                    visible: hostsList.count === 0
                    icon.name: "network-server"
                    text: qsTr("No host entries found")
                    explanation: qsTr("/etc/hosts has no hostname mappings yet.")

                    helpfulAction: Kirigami.Action {
                        text: qsTr("Add entry")
                        icon.name: "list-add"
                        onTriggered: {
                            ipField.text = "";
                            namesField.text = "";
                            hostDialog.editingIndex = -1;
                            hostDialog.open();
                        }
                    }
                }

                ListView {
                    id: hostsList

                    anchors.fill: parent
                    visible: count > 0
                    clip: true
                    model: page.hostEntries
                    boundsBehavior: Flickable.StopAtBounds

                    Controls.ScrollBar.vertical: AppScrollBar {}

                    delegate: Controls.ItemDelegate {
                        id: delegate

                        required property var modelData
                        required property int index

                        width: hostsList.width
                        height: Design.rowHeightNormal
                        leftPadding: Design.space16
                        rightPadding: Design.space16
                        topPadding: 0
                        bottomPadding: 0
                        hoverEnabled: true

                        background: Rectangle {
                            color: "transparent"

                            Rectangle {
                                anchors.fill: parent
                                anchors.leftMargin: Design.itemInset
                                anchors.topMargin: Design.itemInset
                                anchors.bottomMargin: Design.itemInset
                                anchors.rightMargin: Design.itemInset + (hostsList.contentHeight > hostsList.height ? Design.scrollBarGutter : 0)
                                radius: Design.radiusItem
                                color: delegate.hovered ? Design.surfaceHover : Design.surfaceHoverClear

                                Behavior on color {
                                    ColorAnimation { duration: Design.durationNormal }
                                }
                            }
                        }

                        contentItem: RowLayout {
                            spacing: Design.space16

                            Controls.Label {
                                Layout.preferredWidth: 170
                                text: delegate.modelData.ip
                                color: Design.accent
                                font.family: "monospace"
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }

                            Controls.Label {
                                Layout.fillWidth: true
                                text: delegate.modelData.names
                                color: Design.text
                                elide: Text.ElideRight
                            }

                            AppButton {
                                Layout.preferredWidth: 40
                                display: Controls.AbstractButton.IconOnly
                                icon.name: "document-edit"
                                text: qsTr("Edit")
                                onClicked: {
                                    ipField.text = delegate.modelData.ip;
                                    namesField.text = delegate.modelData.names;
                                    hostDialog.editingIndex = delegate.index;
                                    hostDialog.open();
                                }
                            }

                            AppButton {
                                Layout.preferredWidth: 40
                                display: Controls.AbstractButton.IconOnly
                                icon.name: "edit-delete"
                                text: qsTr("Delete")
                                onClicked: {
                                    deleteDialog.pendingIndex = delegate.index;
                                    deleteDialog.pendingNames = delegate.modelData.names;
                                    deleteDialog.open();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    AppDialog {
        id: hostDialog

        property int editingIndex: -1

        title: editingIndex >= 0 ? qsTr("Edit Host Entry") : qsTr("Add Host Entry")

        onOpened: ipField.forceActiveFocus()

        contentItem: Kirigami.FormLayout {
            wideMode: true

            AppTextField {
                id: ipField
                Layout.fillWidth: true
                Kirigami.FormData.label: qsTr("IP address")
                placeholderText: "192.168.1.10"
                font.family: "monospace"
            }

            AppTextField {
                id: namesField
                Layout.fillWidth: true
                Kirigami.FormData.label: qsTr("Hostnames (space separated)")
                placeholderText: qsTr("hostname alias")
            }
        }

        footer: AppDialogFooter {
            AppButton {
                Layout.fillWidth: true
                text: qsTr("Cancel")
                onClicked: hostDialog.close()
            }

            AppButton {
                Layout.fillWidth: true
                text: qsTr("Apply")
                highlighted: true
                enabled: page.isValidAddress(ipField.text) && page.areValidNames(namesField.text)
                onClicked: {
                    const entries = page.hostEntries.slice();
                    if (hostDialog.editingIndex >= 0) {
                        entries[hostDialog.editingIndex] = {
                            line: entries[hostDialog.editingIndex].line,
                            ip: ipField.text.trim(),
                            names: namesField.text.trim()
                        };
                    } else {
                        entries.push({ line: -1, ip: ipField.text.trim(), names: namesField.text.trim() });
                    }
                    Hosts.setEntries(entries);
                    hostDialog.close();
                }
            }
        }
    }

    ConfirmDialog {
        id: deleteDialog

        property int pendingIndex: -1
        property string pendingNames

        title: qsTr("Delete Host Entry")
        message: qsTr("Delete the host entry “%1”?").arg(pendingNames)
        confirmText: qsTr("Delete")
        destructive: true
        onConfirmed: {
            const entries = page.hostEntries.slice();
            entries.splice(deleteDialog.pendingIndex, 1);
            Hosts.setEntries(entries);
        }
    }
}

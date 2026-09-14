// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

pragma ComponentBehavior: Bound

Item {
    id: page

    readonly property var hostEntries: Hosts.entries

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

    Component.onCompleted: Hosts.reload()

    Connections {
        target: Hosts

        function onSaved(ok, error) {
            inlineMessage.type = ok ? Kirigami.MessageType.Positive : Kirigami.MessageType.Error;
            inlineMessage.text = ok ? qsTr("/etc/hosts saved.") : error;
            inlineMessage.visible = true;
            hideMessageTimer.restart();
        }
    }

    Timer {
        id: hideMessageTimer
        interval: 6000
        onTriggered: inlineMessage.visible = false
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Design.pagePadding
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
                onClicked: Hosts.save()
            }
        }

        Kirigami.InlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
            visible: false
        }

        AppCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 0

            contentItem: Item {
                Controls.Label {
                    anchors.centerIn: parent
                    visible: hostsList.count === 0
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("No host entries found.")
                    color: Design.textMuted
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
                        height: 52
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
                                    ColorAnimation { duration: 120 }
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
                                    const entries = page.hostEntries.slice();
                                    entries.splice(delegate.index, 1);
                                    Hosts.setEntries(entries);
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

        contentItem: ColumnLayout {
            spacing: Design.space8

            Controls.Label {
                text: qsTr("IP address")
                color: Design.textMuted
                font.pointSize: Design.smallFontSize
            }

            AppTextField {
                id: ipField
                Layout.fillWidth: true
                placeholderText: "192.168.1.10"
                font.family: "monospace"
            }

            Controls.Label {
                Layout.topMargin: Design.space4
                text: qsTr("Hostnames (space separated)")
                color: Design.textMuted
                font.pointSize: Design.smallFontSize
            }

            AppTextField {
                id: namesField
                Layout.fillWidth: true
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
}

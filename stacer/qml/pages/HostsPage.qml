import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: page

    title: qsTr("Hosts")

    property var hostEntries: []

    Component.onCompleted: Hosts.reload()

    Connections {
        target: Hosts
        function onLoaded(entries) {
            page.hostEntries = entries;
        }
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
        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
            visible: false
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Controls.Button {
                icon.name: "list-add"
                text: qsTr("Add entry")
                onClicked: {
                    ipField.text = "";
                    namesField.text = "";
                    hostDialog.editingIndex = -1;
                    hostDialog.open();
                }
            }

            Controls.Button {
                icon.name: "document-save"
                text: qsTr("Save changes")
                onClicked: Hosts.save(page.hostEntries)
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.Label {
                text: qsTr("Entries: %1").arg(page.hostEntries.length)
                color: Kirigami.Theme.disabledTextColor
            }
        }

        Controls.Label {
            Layout.fillWidth: true
            visible: page.hostEntries.length === 0
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("No host entries found.")
            color: Kirigami.Theme.disabledTextColor
        }

        ListView {
            id: hostsList

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: page.hostEntries
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Controls.ScrollBar {}

            delegate: Controls.ItemDelegate {
                width: hostsList.width

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Controls.Label {
                        Layout.preferredWidth: page.width * 0.3
                        text: modelData.ip
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        text: modelData.names
                        elide: Text.ElideRight
                    }

                    Controls.Button {
                        display: Controls.AbstractButton.IconOnly
                        icon.name: "document-edit"
                        text: qsTr("Edit")
                        onClicked: {
                            ipField.text = modelData.ip;
                            namesField.text = modelData.names;
                            hostDialog.editingIndex = index;
                            hostDialog.open();
                        }
                    }

                    Controls.Button {
                        display: Controls.AbstractButton.IconOnly
                        icon.name: "edit-delete"
                        text: qsTr("Delete")
                        onClicked: {
                            const entries = page.hostEntries.slice();
                            entries.splice(index, 1);
                            page.hostEntries = entries;
                        }
                    }
                }
            }
        }
    }

    Controls.Dialog {
        id: hostDialog

        property int editingIndex: -1

        title: editingIndex >= 0 ? qsTr("Edit Host Entry") : qsTr("Add Host Entry")
        modal: true
        anchors.centerIn: parent
        standardButtons: Controls.Dialog.NoButton

        contentItem: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            Controls.TextField {
                id: ipField
                Layout.fillWidth: true
                placeholderText: qsTr("IP address")
            }

            Controls.TextField {
                id: namesField
                Layout.fillWidth: true
                placeholderText: qsTr("Hostnames (space separated)")
            }
        }

        footer: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Controls.Button {
                Layout.fillWidth: true
                text: qsTr("Cancel")
                onClicked: hostDialog.close()
            }

            Controls.Button {
                Layout.fillWidth: true
                text: qsTr("Apply")
                highlighted: true
                enabled: ipField.text.length > 0 && namesField.text.length > 0
                onClicked: {
                    const entries = page.hostEntries.slice();
                    if (hostDialog.editingIndex >= 0) {
                        entries[hostDialog.editingIndex] = {
                            line: entries[hostDialog.editingIndex].line,
                            ip: ipField.text,
                            names: namesField.text
                        };
                    } else {
                        entries.push({ line: -1, ip: ipField.text, names: namesField.text });
                    }
                    page.hostEntries = entries;
                    hostDialog.close();
                }
            }
        }
    }
}

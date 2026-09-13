import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: page

    title: qsTr("Startup Apps")

    onVisibleChanged: {
        if (visible) {
            StartupApps.reload();
        }
    }

    Component.onCompleted: StartupApps.reload()

    Connections {
        target: StartupApps
        function onError(message) {
            inlineMessage.text = message;
            inlineMessage.visible = true;
            hideMessageTimer.restart();
        }
    }

    Timer {
        id: hideMessageTimer
        interval: 5000
        onTriggered: inlineMessage.visible = false
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
            visible: false
            type: Kirigami.MessageType.Error
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Kirigami.SearchField {
                Layout.fillWidth: true
                placeholderText: qsTr("Search startup apps…")
                onTextChanged: StartupApps.filter = text
            }

            Controls.Button {
                icon.name: "list-add"
                text: qsTr("Add")
                onClicked: {
                    nameField.text = "";
                    execField.text = "";
                    commentField.text = "";
                    iconField.text = "";
                    editDialog.editingRow = -1;
                    editDialog.open();
                }
            }

            Controls.Button {
                icon.name: "view-refresh"
                display: Controls.AbstractButton.IconOnly
                text: qsTr("Refresh")
                onClicked: StartupApps.reload()
            }
        }

        Controls.Label {
            Layout.fillWidth: true
            visible: startupList.count === 0
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("No startup applications found.")
            color: Kirigami.Theme.disabledTextColor
        }

        ListView {
            id: startupList

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: StartupApps
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Controls.ScrollBar {}

            delegate: Controls.ItemDelegate {
                width: startupList.width

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                        source: model.icon.length > 0 ? model.icon : "application-x-executable"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Controls.Label {
                            Layout.fillWidth: true
                            text: model.name
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            text: model.exec
                            color: Kirigami.Theme.disabledTextColor
                            elide: Text.ElideRight
                        }
                    }

                    Controls.Label {
                        visible: model.system
                        text: qsTr("System")
                        color: Kirigami.Theme.disabledTextColor
                    }

                    Controls.Switch {
                        checked: model.enabled
                        onClicked: StartupApps.setEnabled(index, checked)
                    }

                    Controls.Button {
                        visible: !model.system
                        display: Controls.AbstractButton.IconOnly
                        icon.name: "document-edit"
                        text: qsTr("Edit")
                        onClicked: {
                            nameField.text = model.name;
                            execField.text = model.exec;
                            commentField.text = model.comment;
                            iconField.text = model.icon;
                            editDialog.editingRow = index;
                            editDialog.open();
                        }
                    }

                    Controls.Button {
                        visible: !model.system
                        display: Controls.AbstractButton.IconOnly
                        icon.name: "edit-delete"
                        text: qsTr("Remove")
                        onClicked: StartupApps.remove(index)
                    }
                }
            }
        }
    }

    Controls.Dialog {
        id: editDialog

        property int editingRow: -1

        title: editingRow >= 0 ? qsTr("Edit Startup App") : qsTr("Add Startup App")
        modal: true
        anchors.centerIn: parent
        standardButtons: Controls.Dialog.NoButton

        contentItem: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            Controls.TextField {
                id: nameField
                Layout.fillWidth: true
                placeholderText: qsTr("Name")
            }

            Controls.TextField {
                id: execField
                Layout.fillWidth: true
                placeholderText: qsTr("Command")
            }

            Controls.TextField {
                id: commentField
                Layout.fillWidth: true
                placeholderText: qsTr("Comment")
            }

            Controls.TextField {
                id: iconField
                Layout.fillWidth: true
                placeholderText: qsTr("Icon name")
            }
        }

        footer: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Controls.Button {
                Layout.fillWidth: true
                text: qsTr("Cancel")
                onClicked: editDialog.close()
            }

            Controls.Button {
                Layout.fillWidth: true
                text: qsTr("Save")
                highlighted: true
                enabled: nameField.text.length > 0 && execField.text.length > 0
                onClicked: {
                    if (editDialog.editingRow >= 0) {
                        StartupApps.save(editDialog.editingRow, nameField.text, commentField.text, execField.text, iconField.text);
                    } else {
                        StartupApps.add(nameField.text, commentField.text, execField.text, iconField.text);
                    }
                    editDialog.close();
                }
            }
        }
    }
}

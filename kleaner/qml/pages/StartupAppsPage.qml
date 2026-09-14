// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Item {
    id: page

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
        anchors.margins: Design.pagePadding
        spacing: Design.space16

        PageHeader {
            Layout.fillWidth: true
            title: qsTr("Startup Apps")
            subtitle: qsTr("Applications launched automatically when you log in")

            Badge {
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("%1 entries").arg(startupList.count)
                badgeColor: Design.textMuted
            }

            AppButton {
                text: qsTr("Add")
                icon.name: "list-add"
                onClicked: {
                    nameField.text = "";
                    execField.text = "";
                    commentField.text = "";
                    iconField.text = "";
                    editDialog.editingRow = -1;
                    editDialog.open();
                }
            }

            AppButton {
                icon.name: "view-refresh"
                display: Controls.AbstractButton.IconOnly
                text: qsTr("Refresh")
                onClicked: StartupApps.reload()
            }
        }

        Kirigami.InlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
            visible: false
            type: Kirigami.MessageType.Error
        }

        AppSearchField {
            Layout.fillWidth: true
            placeholderText: qsTr("Search startup apps…")
            text: StartupApps.filter
            onTextChanged: StartupApps.filter = text
        }

        AppCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 0

            contentItem: Item {
                Controls.Label {
                    anchors.centerIn: parent
                    visible: startupList.count === 0
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("No startup applications found.")
                    color: Design.textMuted
                }

                ListView {
                    id: startupList

                    anchors.fill: parent
                    visible: count > 0
                    clip: true
                    model: StartupApps
                    boundsBehavior: Flickable.StopAtBounds

                    Controls.ScrollBar.vertical: AppScrollBar {}

                    delegate: Controls.ItemDelegate {
                        id: delegate

                        width: startupList.width
                        height: 64
                        leftPadding: Design.space16
                        rightPadding: Design.space16
                        topPadding: 0
                        bottomPadding: 0
                        hoverEnabled: true

                        background: Rectangle {
                            color: delegate.hovered ? Design.surfaceHover : "transparent"

                            Rectangle {
                                anchors.bottom: parent.bottom
                                width: parent.width
                                height: 1
                                color: Design.border
                                opacity: 0.6
                            }
                        }

                        contentItem: RowLayout {
                            spacing: Design.space12

                            Rectangle {
                                Layout.alignment: Qt.AlignVCenter
                                Layout.preferredWidth: 38
                                Layout.preferredHeight: 38
                                radius: Design.radiusSmall
                                color: Design.surfaceHover

                                Kirigami.Icon {
                                    anchors.centerIn: parent
                                    width: 22
                                    height: 22
                                    source: model.icon.length > 0 ? model.icon : "application-x-executable"
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Controls.Label {
                                    Layout.fillWidth: true
                                    text: model.name
                                    color: Design.text
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                }

                                Controls.Label {
                                    Layout.fillWidth: true
                                    text: model.comment.length > 0 ? model.comment : model.exec
                                    color: Design.textMuted
                                    font.pointSize: Design.smallFontSize
                                    elide: Text.ElideRight
                                }
                            }

                            Badge {
                                Layout.alignment: Qt.AlignVCenter
                                visible: model.system
                                text: qsTr("System")
                                badgeColor: Design.textMuted
                            }

                            Item {
                                Layout.preferredWidth: 72
                                implicitHeight: 32

                                AppSwitch {
                                    anchors.centerIn: parent
                                    checked: model.enabled
                                    onClicked: StartupApps.setEnabled(index, checked)
                                }
                            }

                            AppButton {
                                visible: !model.system
                                Layout.preferredWidth: 40
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

                            AppButton {
                                visible: !model.system
                                Layout.preferredWidth: 40
                                display: Controls.AbstractButton.IconOnly
                                icon.name: "edit-delete"
                                text: qsTr("Remove")
                                onClicked: StartupApps.remove(index)
                            }

                            Item {
                                visible: model.system
                                Layout.preferredWidth: 80
                            }
                        }
                    }
                }
            }
        }
    }

    AppDialog {
        id: editDialog

        property int editingRow: -1

        title: editingRow >= 0 ? qsTr("Edit Startup App") : qsTr("Add Startup App")

        contentItem: ColumnLayout {
            spacing: Design.space8

            Controls.Label {
                text: qsTr("Name")
                color: Design.textMuted
                font.pointSize: Design.smallFontSize
            }

            AppTextField {
                id: nameField
                Layout.fillWidth: true
                placeholderText: qsTr("Name")
            }

            Controls.Label {
                Layout.topMargin: Design.space4
                text: qsTr("Command")
                color: Design.textMuted
                font.pointSize: Design.smallFontSize
            }

            AppTextField {
                id: execField
                Layout.fillWidth: true
                placeholderText: qsTr("Command")
            }

            Controls.Label {
                Layout.topMargin: Design.space4
                text: qsTr("Comment")
                color: Design.textMuted
                font.pointSize: Design.smallFontSize
            }

            AppTextField {
                id: commentField
                Layout.fillWidth: true
                placeholderText: qsTr("Comment")
            }

            Controls.Label {
                Layout.topMargin: Design.space4
                text: qsTr("Icon name")
                color: Design.textMuted
                font.pointSize: Design.smallFontSize
            }

            AppTextField {
                id: iconField
                Layout.fillWidth: true
                placeholderText: qsTr("Icon name")
            }
        }

        footer: AppDialogFooter {
            AppButton {
                Layout.fillWidth: true
                text: qsTr("Cancel")
                onClicked: editDialog.close()
            }

            AppButton {
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

// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

pragma ComponentBehavior: Bound

Kirigami.Page {
    padding: Design.pagePadding

    onVisibleChanged: {
        if (visible) {
            StartupApps.reload();
        }
    }

    Component.onCompleted: StartupApps.reload()

    Connections {
        target: StartupApps

        function onError(message) {
            inlineMessage.showError(message);
        }
    }

    ColumnLayout {
        anchors.fill: parent
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
                highlighted: true
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
                spinning: StartupApps.loading
                enabled: !StartupApps.loading
                onClicked: StartupApps.reload()
            }
        }

        AppInlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
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
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: Math.min(implicitWidth, parent.width - Design.space20 * 2)
                    visible: startupList.count === 0
                    icon.name: "system-run"
                    text: qsTr("No startup applications found")
                    explanation: StartupApps.filter.length > 0
                                 ? qsTr("Try a different search term.")
                                 : qsTr("Applications that launch on login will appear here.")
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

                        required property int index
                        required property string name
                        required property string comment
                        required property string exec
                        required property string iconName
                        required property bool autostart
                        required property bool system

                        width: startupList.width
                        height: Design.rowHeightLarge
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
                                anchors.rightMargin: Design.itemInset + (startupList.contentHeight > startupList.height ? Design.scrollBarGutter : 0)
                                radius: Design.radiusItem
                                color: delegate.hovered ? Design.surfaceHover : Design.surfaceHoverClear

                                Behavior on color {
                                    ColorAnimation { duration: Design.durationNormal }
                                }
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
                                    width: Design.iconXLarge
                                    height: Design.iconXLarge
                                    source: delegate.iconName.length > 0 ? delegate.iconName : "application-x-executable"
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Controls.Label {
                                    Layout.fillWidth: true
                                    text: delegate.name
                                    color: Design.text
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                }

                                Controls.Label {
                                    Layout.fillWidth: true
                                    text: delegate.comment.length > 0 ? delegate.comment : delegate.exec
                                    color: Design.textMuted
                                    font.pointSize: Design.smallFontSize
                                    elide: Text.ElideRight
                                }
                            }

                            Badge {
                                Layout.alignment: Qt.AlignVCenter
                                visible: delegate.system
                                text: qsTr("System")
                                badgeColor: Design.textMuted
                            }

                            Item {
                                Layout.preferredWidth: 72
                                implicitHeight: 32

                                AppSwitch {
                                    id: autostartSwitch

                                    anchors.centerIn: parent
                                    checked: delegate.autostart
                                    onClicked: StartupApps.setEnabled(delegate.index, checked)

                                    Binding {
                                        target: autostartSwitch
                                        property: "checked"
                                        value: delegate.autostart
                                        restoreMode: Binding.RestoreBindingOrValue
                                    }
                                }
                            }

                            AppButton {
                                visible: !delegate.system
                                Layout.preferredWidth: 40
                                display: Controls.AbstractButton.IconOnly
                                icon.name: "document-edit"
                                text: qsTr("Edit")
                                onClicked: {
                                    nameField.text = delegate.name;
                                    execField.text = delegate.exec;
                                    commentField.text = delegate.comment;
                                    iconField.text = delegate.iconName;
                                    editDialog.editingRow = delegate.index;
                                    editDialog.open();
                                }
                            }

                            AppButton {
                                visible: !delegate.system
                                Layout.preferredWidth: 40
                                display: Controls.AbstractButton.IconOnly
                                icon.name: "edit-delete"
                                text: qsTr("Remove")
                                onClicked: {
                                    removeDialog.pendingIndex = delegate.index;
                                    removeDialog.pendingName = delegate.name;
                                    removeDialog.open();
                                }
                            }

                            Item {
                                // Reserve the same space as the two row actions so
                                // the switch keeps a single column.
                                visible: delegate.system
                                Layout.preferredWidth: 2 * 40 + Design.space12
                            }
                        }
                    }
                }
            }
        }
    }

    ConfirmDialog {
        id: removeDialog

        property int pendingIndex: -1
        property string pendingName

        title: qsTr("Remove Startup App")
        message: qsTr("Remove “%1” from the list of startup applications?").arg(pendingName)
        confirmText: qsTr("Remove")
        destructive: true
        onConfirmed: StartupApps.remove(pendingIndex)
    }

    AppDialog {
        id: editDialog

        property int editingRow: -1

        title: editingRow >= 0 ? qsTr("Edit Startup App") : qsTr("Add Startup App")

        onOpened: nameField.forceActiveFocus()

        contentItem: Kirigami.FormLayout {
            wideMode: true

            AppTextField {
                id: nameField
                Layout.fillWidth: true
                Kirigami.FormData.label: qsTr("Name")
                placeholderText: qsTr("Name")
            }

            AppTextField {
                id: execField
                Layout.fillWidth: true
                Kirigami.FormData.label: qsTr("Command")
                placeholderText: qsTr("Command")
            }

            AppTextField {
                id: commentField
                Layout.fillWidth: true
                Kirigami.FormData.label: qsTr("Comment")
                placeholderText: qsTr("Comment")
            }

            AppTextField {
                id: iconField
                Layout.fillWidth: true
                Kirigami.FormData.label: qsTr("Icon name")
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

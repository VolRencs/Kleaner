// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Item {
    id: page

    Component.onCompleted: Cleaner.scan()

    Connections {
        target: Cleaner

        function onCleanFinished(ok, message, count) {
            inlineMessage.type = ok ? Kirigami.MessageType.Positive : Kirigami.MessageType.Error;
            inlineMessage.text = ok
                ? qsTr("Removed %1 items.").arg(count)
                : message;
            inlineMessage.visible = true;
            hideMessageTimer.restart();
            if (ok) {
                Cleaner.scan();
            }
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
            title: qsTr("System Cleaner")
            subtitle: qsTr("Remove caches, logs and other files you no longer need")
        }

        Kirigami.InlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
            visible: false
        }

        AppCard {
            Layout.fillWidth: true

            contentItem: RowLayout {
                spacing: Design.space16

                Rectangle {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: 56
                    Layout.preferredHeight: 56
                    radius: 28
                    color: Design.accentSoft

                    Kirigami.Icon {
                        anchors.centerIn: parent
                        width: 26
                        height: 26
                        source: Cleaner.scanning ? "search" : "edit-clear"
                        color: Design.accent
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Controls.Label {
                        Layout.fillWidth: true
                        text: Cleaner.scanning ? qsTr("Scanning for removable files…") : qsTr("Ready to clean")
                        color: Design.text
                        font.pointSize: Design.baseFontSize * 1.2
                        font.weight: Font.DemiBold
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        text: Cleaner.scanning
                              ? qsTr("This may take a moment.")
                              : qsTr("Select the categories you want to remove, then clean them up.")
                        color: Design.textMuted
                        font.pointSize: Design.smallFontSize
                        wrapMode: Text.WordWrap
                    }
                }

                AppSpinner {
                    Layout.alignment: Qt.AlignVCenter
                    running: Cleaner.scanning
                    visible: running
                    implicitWidth: 28
                    implicitHeight: 28
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 0

                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        text: Format.bytes(Cleaner.checkedSize)
                        color: Design.accent
                        font.pointSize: Design.baseFontSize * 1.6
                        font.weight: Font.Bold
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        text: qsTr("selected")
                        color: Design.textFaint
                        font.pointSize: Design.tinyFontSize
                    }
                }

                AppButton {
                    Layout.alignment: Qt.AlignVCenter
                    text: qsTr("Scan")
                    icon.name: "search"
                    enabled: !Cleaner.scanning
                    onClicked: Cleaner.scan()
                }

                AppButton {
                    Layout.alignment: Qt.AlignVCenter
                    text: qsTr("Clean")
                    icon.name: "edit-delete"
                    highlighted: true
                    enabled: !Cleaner.scanning && Cleaner.hasCheckedItems
                    onClicked: Cleaner.clean()
                }
            }
        }

        AppCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 0

            contentItem: Item {
                Controls.Label {
                    anchors.centerIn: parent
                    visible: cleanerList.count === 0 && !Cleaner.scanning
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Press Scan to look for removable files.")
                    color: Design.textMuted
                }

                ListView {
                    id: cleanerList

                    anchors.fill: parent
                    visible: count > 0
                    clip: true
                    model: Cleaner
                    boundsBehavior: Flickable.StopAtBounds

                    Controls.ScrollBar.vertical: AppScrollBar {}

                    delegate: Controls.ItemDelegate {
                        id: delegate

                        width: cleanerList.width
                        height: 46
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
                            spacing: Design.space8

                            Item {
                                Layout.preferredWidth: model.depth * 20
                            }

                            Item {
                                Layout.alignment: Qt.AlignVCenter
                                // Always reserve the expander column so rows without a
                                // chevron (e.g. orphan packages) stay aligned.
                                Layout.preferredWidth: 28
                                Layout.preferredHeight: 28

                                Controls.Button {
                                    anchors.fill: parent
                                    visible: model.expandable
                                    flat: true
                                    display: Controls.AbstractButton.IconOnly
                                    icon.name: model.expanded ? "go-down" : "go-next"
                                    text: model.expanded ? qsTr("Collapse") : qsTr("Expand")
                                    onClicked: Cleaner.toggleExpand(index)
                                }
                            }

                            AppCheckBox {
                                Layout.alignment: Qt.AlignVCenter
                                tristate: model.isCategory
                                checkState: model.checkState
                                // With tristate the first click yields PartiallyChecked, so use
                                // the state instead of `checked` to actually select the category.
                                onClicked: Cleaner.setChecked(index, checkState !== Qt.Unchecked)
                            }

                            Controls.Label {
                                Layout.fillWidth: true
                                text: model.title
                                color: Design.text
                                font.weight: model.isCategory ? Font.DemiBold : Font.Normal
                                elide: Text.ElideRight
                            }

                            Badge {
                                Layout.alignment: Qt.AlignVCenter
                                visible: model.root
                                text: qsTr("root")
                                badgeColor: Design.warning
                            }

                            Controls.Label {
                                Layout.preferredWidth: 100
                                horizontalAlignment: Text.AlignRight
                                text: model.size > 0 ? Format.bytes(model.size) : "—"
                                color: Design.textMuted
                                font.pointSize: Design.smallFontSize
                            }
                        }
                    }
                }
            }
        }
    }
}

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

    Component.onCompleted: Cleaner.scan()

    Connections {
        target: Cleaner

        function onCleaningChanged() {
            if (Cleaner.cleaning) {
                statusMessage.show(qsTr("Cleaning…"), Kirigami.MessageType.Information, false);
            }
        }

        function onLastResultChanged() {
            if (Cleaner.cleaning) {
                return;
            }
            if (Cleaner.lastError.length > 0) {
                statusMessage.showError(Cleaner.lastError);
            } else if (Cleaner.lastFreedBytes > 0 || Cleaner.lastRemovedCount > 0) {
                statusMessage.showPositive(qsTr("Freed %1 · %2 items removed")
                                           .arg(Format.bytes(Cleaner.lastFreedBytes))
                                           .arg(Cleaner.lastRemovedCount));
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Design.space16

        PageHeader {
            Layout.fillWidth: true
            title: qsTr("System Cleaner")
            subtitle: qsTr("Remove caches, logs and other files you no longer need")
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
                        source: Cleaner.scanning ? "search" : "edit-clear-all"
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

                AppBusyIndicator {
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
                    enabled: !Cleaner.scanning && !Cleaner.cleaning
                    onClicked: Cleaner.scan()
                }

                AppButton {
                    Layout.alignment: Qt.AlignVCenter
                    text: qsTr("Clean")
                    icon.name: "edit-clear-all"
                    highlighted: true
                    enabled: !Cleaner.scanning && !Cleaner.cleaning && Cleaner.hasCheckedItems
                    onClicked: cleanDialog.open()
                }
            }
        }

        AppCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 0

            contentItem: Item {
                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: Math.min(implicitWidth, parent.width - Design.space20 * 2)
                    visible: cleanerList.count === 0 && !Cleaner.scanning
                    icon.name: "edit-clear-all"
                    text: qsTr("Nothing to clean yet")
                    explanation: qsTr("Scan the system to look for caches, logs and other removable files.")

                    helpfulAction: Kirigami.Action {
                        text: qsTr("Scan")
                        icon.name: "search"
                        onTriggered: Cleaner.scan()
                    }
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

                        required property int index
                        required property string title
                        required property real size
                        required property int depth
                        required property bool expandable
                        required property bool expanded
                        required property int checkState
                        required property bool root
                        required property bool isCategory

                        width: cleanerList.width
                        height: Design.rowHeightTree
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
                                anchors.rightMargin: Design.itemInset + (cleanerList.contentHeight > cleanerList.height ? Design.scrollBarGutter : 0)
                                radius: Design.radiusItem
                                color: delegate.hovered ? Design.surfaceHover : Design.surfaceHoverClear

                                Behavior on color {
                                    ColorAnimation { duration: Design.durationNormal }
                                }
                            }
                        }

                        contentItem: RowLayout {
                            spacing: Design.space8

                            Item {
                                Layout.preferredWidth: delegate.depth * 20
                            }

                            Item {
                                Layout.alignment: Qt.AlignVCenter
                                // Always reserve the expander column so rows without a
                                // chevron (e.g. orphan packages) stay aligned.
                                Layout.preferredWidth: 28
                                Layout.preferredHeight: 28

                                AppIconButton {
                                    anchors.fill: parent
                                    visible: delegate.expandable
                                    iconSource: "go-next"
                                    iconRotation: delegate.expanded ? 90 : 0
                                    text: delegate.expanded ? qsTr("Collapse") : qsTr("Expand")
                                    onClicked: Cleaner.toggleExpand(delegate.index)
                                }
                            }

                            AppCheckBox {
                                id: cleanerCheck

                                Layout.alignment: Qt.AlignVCenter
                                tristate: delegate.isCategory
                                checkState: delegate.checkState
                                // With tristate the first click yields PartiallyChecked, so use
                                // the state instead of `checked` to actually select the category.
                                onClicked: Cleaner.setChecked(delegate.index, checkState !== Qt.Unchecked)

                                Binding {
                                    target: cleanerCheck
                                    property: "checkState"
                                    value: delegate.checkState
                                    restoreMode: Binding.RestoreBindingOrValue
                                }
                            }

                            Controls.Label {
                                Layout.fillWidth: true
                                text: delegate.title
                                color: Design.text
                                font.weight: delegate.isCategory ? Font.DemiBold : Font.Normal
                                elide: Text.ElideRight
                            }

                            Badge {
                                Layout.alignment: Qt.AlignVCenter
                                visible: delegate.root
                                text: qsTr("root")
                                badgeColor: Design.warning
                            }

                            Controls.Label {
                                Layout.preferredWidth: 100
                                horizontalAlignment: Text.AlignRight
                                text: delegate.size > 0 ? Format.bytes(delegate.size) : "—"
                                color: Design.textMuted
                                font.pointSize: Design.smallFontSize
                            }
                        }
                    }
                }
            }
        }

        AppInlineMessage {
            id: statusMessage
            Layout.fillWidth: true
        }
    }

    ConfirmDialog {
        id: cleanDialog

        title: qsTr("Clean Selected Files")
        message: qsTr("Remove the selected files (%1)? This cannot be undone.").arg(Format.bytes(Cleaner.checkedSize))
        confirmText: qsTr("Clean")
        destructive: true
        onConfirmed: Cleaner.clean()
    }
}

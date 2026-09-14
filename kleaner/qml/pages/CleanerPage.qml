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

    Component.onCompleted: Cleaner.scan()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Design.pagePadding
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
                    enabled: !Cleaner.scanning && !Cleaner.cleaning
                    onClicked: Cleaner.scan()
                }

                AppButton {
                    Layout.alignment: Qt.AlignVCenter
                    text: qsTr("Clean")
                    icon.name: "edit-delete"
                    highlighted: true
                    enabled: !Cleaner.scanning && !Cleaner.cleaning && Cleaner.hasCheckedItems
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
                        height: 46
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
                                    ColorAnimation { duration: 120 }
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

                                Controls.Button {
                                    anchors.fill: parent
                                    visible: delegate.expandable
                                    flat: true
                                    display: Controls.AbstractButton.IconOnly
                                    icon.name: delegate.expanded ? "go-down" : "go-next"
                                    text: delegate.expanded ? qsTr("Collapse") : qsTr("Expand")
                                    onClicked: Cleaner.toggleExpand(delegate.index)
                                }
                            }

                            AppCheckBox {
                                Layout.alignment: Qt.AlignVCenter
                                tristate: delegate.isCategory
                                checkState: delegate.checkState
                                // With tristate the first click yields PartiallyChecked, so use
                                // the state instead of `checked` to actually select the category.
                                onClicked: Cleaner.setChecked(delegate.index, checkState !== Qt.Unchecked)
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

        RowLayout {
            Layout.fillWidth: true
            spacing: Design.space8
            visible: Cleaner.cleaning || Cleaner.lastError.length > 0
                     || Cleaner.lastRemovedCount > 0 || Cleaner.lastFreedBytes > 0

            Kirigami.Icon {
                Layout.alignment: Qt.AlignVCenter
                implicitWidth: 16
                implicitHeight: 16
                source: Cleaner.cleaning ? "edit-clear"
                      : Cleaner.lastError.length > 0 ? "dialog-error"
                                                     : "dialog-ok-apply"
                color: Cleaner.lastError.length > 0 ? Design.negative : Design.positive
            }

            Controls.Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: Cleaner.lastError.length > 0 ? Design.negative : Design.textMuted
                font.pointSize: Design.smallFontSize
                text: {
                    if (Cleaner.cleaning) {
                        return qsTr("Cleaning…");
                    }
                    if (Cleaner.lastError.length > 0) {
                        return Cleaner.lastError;
                    }
                    return qsTr("Freed %1 · %2 items removed").arg(Format.bytes(Cleaner.lastFreedBytes)).arg(Cleaner.lastRemovedCount);
                }
            }
        }
    }
}

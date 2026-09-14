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

    onVisibleChanged: {
        if (visible) {
            Services.reload();
        }
    }

    Component.onCompleted: Services.reload()

    Connections {
        target: Services

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
            title: qsTr("Services")
            subtitle: qsTr("Manage systemd units on this machine")

            Badge {
                Layout.alignment: Qt.AlignVCenter
                visible: Services.available
                text: qsTr("%1 units").arg(serviceList.count)
                badgeColor: Design.textMuted
            }
        }

        Kirigami.InlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
            visible: false
            type: Kirigami.MessageType.Error
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: !Services.available
            type: Kirigami.MessageType.Warning
            text: qsTr("systemd is not available on this system.")
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Design.space8

            AppSearchField {
                Layout.fillWidth: true
                placeholderText: qsTr("Search services…")
                text: Services.filter
                onTextChanged: Services.filter = text
            }

            AppButton {
                icon.name: "view-refresh"
                display: Controls.AbstractButton.IconOnly
                text: qsTr("Refresh")
                spinning: Services.loading
                enabled: !Services.loading
                onClicked: Services.reload()
            }
        }

        AppCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 0

            contentItem: ColumnLayout {
                spacing: 0

                RowLayout {
                    Layout.fillWidth: true
                    Layout.leftMargin: Design.space16
                    Layout.rightMargin: Design.space16
                    Layout.topMargin: Design.space12
                    Layout.bottomMargin: Design.space8
                    spacing: Design.space12

                    SortHeader {
                        Layout.fillWidth: true
                        alignLeft: true
                        text: qsTr("Service")
                        sorted: Services.sortBy === Services.SortName
                        descending: Services.reverse
                        onClicked: {
                            if (Services.sortBy === Services.SortName) {
                                Services.reverse = !Services.reverse;
                            } else {
                                Services.sortBy = Services.SortName;
                                Services.reverse = false;
                            }
                        }
                    }

                    SortHeader {
                        Layout.preferredWidth: 110
                        text: qsTr("State")
                        sorted: Services.sortBy === Services.SortState
                        descending: Services.reverse
                        onClicked: {
                            if (Services.sortBy === Services.SortState) {
                                Services.reverse = !Services.reverse;
                            } else {
                                Services.sortBy = Services.SortState;
                                Services.reverse = false;
                            }
                        }
                    }

                    SortHeader {
                        Layout.preferredWidth: 110
                        text: qsTr("Autostart")
                        sorted: Services.sortBy === Services.SortStartup
                        descending: Services.reverse
                        onClicked: {
                            if (Services.sortBy === Services.SortStartup) {
                                Services.reverse = !Services.reverse;
                            } else {
                                Services.sortBy = Services.SortStartup;
                                Services.reverse = false;
                            }
                        }
                    }

                    Controls.Label {
                        Layout.preferredWidth: 96
                        horizontalAlignment: Text.AlignHCenter
                        text: qsTr("Actions")
                        color: Design.textFaint
                        font.pointSize: Design.tinyFontSize
                        font.weight: Font.DemiBold
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: Design.border
                }

                Controls.Label {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: serviceList.count === 0
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("No services found.")
                    color: Design.textMuted
                }

                ListView {
                    id: serviceList

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: count > 0
                    clip: true
                    model: Services
                    boundsBehavior: Flickable.StopAtBounds

                    Controls.ScrollBar.vertical: AppScrollBar {}

                    delegate: Controls.ItemDelegate {
                        id: delegate

                        required property int index
                        required property string name
                        required property string description
                        required property bool autostart
                        required property bool active
                        required property string activeState

                        width: serviceList.width
                        height: 60
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
                                anchors.rightMargin: Design.itemInset + (serviceList.contentHeight > serviceList.height ? Design.scrollBarGutter : 0)
                                radius: Design.radiusItem
                                color: delegate.hovered ? Design.surfaceHover : Design.surfaceHoverClear

                                Behavior on color {
                                    ColorAnimation { duration: 120 }
                                }
                            }
                        }

                        contentItem: RowLayout {
                            spacing: Design.space12

                            Rectangle {
                                Layout.alignment: Qt.AlignVCenter
                                Layout.preferredWidth: 36
                                Layout.preferredHeight: 36
                                radius: Design.radiusSmall
                                color: Design.surfaceHover

                                Kirigami.Icon {
                                    anchors.centerIn: parent
                                    width: 18
                                    height: 18
                                    source: "preferences-system-services"
                                    color: delegate.active ? Design.positive : Design.textFaint
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
                                    text: delegate.description
                                    color: Design.textMuted
                                    font.pointSize: Design.smallFontSize
                                    elide: Text.ElideRight
                                }
                            }

                            Item {
                                Layout.preferredWidth: 110
                                implicitHeight: stateBadge.implicitHeight

                                Badge {
                                    id: stateBadge
                                    anchors.centerIn: parent
                                    text: delegate.activeState === "failed" ? qsTr("Failed")
                                                                             : delegate.active ? qsTr("Running")
                                                                                               : qsTr("Stopped")
                                    badgeColor: delegate.activeState === "failed" ? Design.negative
                                                                                  : delegate.active ? Design.positive
                                                                                                     : Design.textMuted
                                }
                            }

                            Item {
                                Layout.preferredWidth: 110
                                implicitHeight: 32

                                AppSwitch {
                                    anchors.centerIn: parent
                                    checked: delegate.autostart
                                    onClicked: Services.setEnabled(delegate.index, checked)
                                }
                            }

                            RowLayout {
                                Layout.preferredWidth: 96
                                Layout.alignment: Qt.AlignVCenter
                                spacing: Design.space8

                                AppButton {
                                    Layout.preferredWidth: 44
                                    display: Controls.AbstractButton.IconOnly
                                    icon.name: delegate.active ? "media-playback-stop" : "media-playback-start"
                                    text: delegate.active ? qsTr("Stop") : qsTr("Start")
                                    onClicked: delegate.active ? Services.stop(delegate.index) : Services.start(delegate.index)
                                }

                                AppButton {
                                    Layout.preferredWidth: 44
                                    display: Controls.AbstractButton.IconOnly
                                    icon.name: "view-refresh"
                                    text: qsTr("Restart")
                                    onClicked: Services.restart(delegate.index)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

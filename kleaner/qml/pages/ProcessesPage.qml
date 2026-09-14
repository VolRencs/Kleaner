// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Item {
    id: page

    property int selectedPid: -1

    onVisibleChanged: {
        Processes.paused = !visible;
        if (visible) {
            Processes.refresh();
        }
    }

    Component.onCompleted: {
        Processes.paused = !visible;
        if (visible) {
            Processes.refresh();
        }
    }

    Timer {
        interval: 2000
        repeat: true
        running: page.visible
        onTriggered: Processes.update()
    }

    function toggleSort(sortBy) {
        if (Processes.sortBy === sortBy) {
            Processes.reverse = !Processes.reverse;
        } else {
            Processes.sortBy = sortBy;
            Processes.reverse = false;
        }
    }

    function stateColor(state) {
        switch (state) {
        case "R":
            return Design.positive;
        case "D":
            return Design.warning;
        case "Z":
            return Design.negative;
        case "T":
            return Design.violet;
        default:
            return Design.textMuted;
        }
    }

    Connections {
        target: Processes

        function onError(message) {
            inlineMessage.text = message;
            inlineMessage.visible = true;
            hideMessageTimer.restart();
        }

        function onLoadingChanged() {
            if (!Processes.loading && page.selectedPid > 1 && !Processes.hasPid(page.selectedPid)) {
                page.selectedPid = -1;
            }
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
            title: qsTr("Processes")
            subtitle: qsTr("Monitor and control everything running on your system")

            Badge {
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("%1 processes").arg(processList.count)
                badgeColor: Design.textMuted
            }
        }

        Kirigami.InlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
            visible: false
            type: Kirigami.MessageType.Error
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Design.space8

            AppSearchField {
                Layout.fillWidth: true
                placeholderText: qsTr("Search processes…")
                text: Processes.filter
                onTextChanged: Processes.filter = text
            }

            AppButton {
                text: qsTr("End Process")
                icon.name: "process-stop"
                enabled: page.selectedPid > 1
                onClicked: Processes.killPid(page.selectedPid, false)
            }

            AppButton {
                text: qsTr("Force Kill")
                icon.name: "application-exit"
                enabled: page.selectedPid > 1
                onClicked: Processes.killPid(page.selectedPid, true)
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.Label {
                visible: page.selectedPid > 1
                text: qsTr("PID %1 selected").arg(page.selectedPid)
                color: Design.textMuted
                font.pointSize: Design.smallFontSize
            }

            AppSpinner {
                running: Processes.loading
                visible: running
                implicitWidth: Kirigami.Units.gridUnit * 1.5
                implicitHeight: Kirigami.Units.gridUnit * 1.5
            }

            AppButton {
                icon.name: "view-refresh"
                display: Controls.AbstractButton.IconOnly
                text: qsTr("Refresh")
                onClicked: Processes.refresh()
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
                        Layout.preferredWidth: processList.width * 0.22
                        alignLeft: true
                        text: qsTr("Name")
                        sorted: Processes.sortBy === Processes.SortName
                        descending: Processes.reverse
                        onClicked: page.toggleSort(Processes.SortName)
                    }

                    SortHeader {
                        Layout.preferredWidth: processList.width * 0.06
                        alignLeft: true
                        text: qsTr("PID")
                        sorted: Processes.sortBy === Processes.SortPid
                        descending: Processes.reverse
                        onClicked: page.toggleSort(Processes.SortPid)
                    }

                    SortHeader {
                        Layout.preferredWidth: processList.width * 0.11
                        alignLeft: true
                        text: qsTr("User")
                        sorted: Processes.sortBy === Processes.SortUser
                        descending: Processes.reverse
                        onClicked: page.toggleSort(Processes.SortUser)
                    }

                    SortHeader {
                        Layout.preferredWidth: processList.width * 0.07
                        alignRight: true
                        text: qsTr("CPU")
                        sorted: Processes.sortBy === Processes.SortCpu
                        descending: Processes.reverse
                        onClicked: page.toggleSort(Processes.SortCpu)
                    }

                    SortHeader {
                        Layout.preferredWidth: processList.width * 0.08
                        alignRight: true
                        text: qsTr("Memory")
                        sorted: Processes.sortBy === Processes.SortMemory
                        descending: Processes.reverse
                        onClicked: page.toggleSort(Processes.SortMemory)
                    }

                    SortHeader {
                        Layout.preferredWidth: processList.width * 0.09
                        alignRight: true
                        text: qsTr("RSS")
                        sorted: Processes.sortBy === Processes.SortRss
                        descending: Processes.reverse
                        onClicked: page.toggleSort(Processes.SortRss)
                    }

                    SortHeader {
                        Layout.preferredWidth: processList.width * 0.06
                        text: qsTr("State")
                        sorted: Processes.sortBy === Processes.SortState
                        descending: Processes.reverse
                        onClicked: page.toggleSort(Processes.SortState)
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        text: qsTr("Command")
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
                    visible: processList.count === 0
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("No processes found.")
                    color: Design.textMuted
                }

                ListView {
                    id: processList

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: count > 0
                    clip: true
                    model: Processes
                    boundsBehavior: Flickable.StopAtBounds

                    Controls.ScrollBar.vertical: AppScrollBar {}

                    delegate: Controls.ItemDelegate {
                        id: delegate

                        width: processList.width
                        height: 42
                        leftPadding: Design.space16
                        rightPadding: Design.space16
                        topPadding: 0
                        bottomPadding: 0
                        highlighted: page.selectedPid === model.pid
                        hoverEnabled: true
                        onClicked: page.selectedPid = model.pid

                        background: Rectangle {
                            color: delegate.highlighted ? Design.accentSoft
                                                        : delegate.hovered ? Design.surfaceHover
                                                                           : "transparent"

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

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.22
                                text: model.name
                                color: Design.text
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.06
                                text: model.pid
                                color: Design.textMuted
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.11
                                text: model.user
                                color: Design.textMuted
                                elide: Text.ElideRight
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.07
                                horizontalAlignment: Text.AlignRight
                                text: model.cpu.toFixed(1) + "%"
                                color: model.cpu > 80 ? Design.negative : model.cpu > 40 ? Design.warning : Design.text
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.08
                                horizontalAlignment: Text.AlignRight
                                text: model.mem.toFixed(1) + "%"
                                color: model.mem > 80 ? Design.negative : model.mem > 40 ? Design.warning : Design.text
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.09
                                horizontalAlignment: Text.AlignRight
                                text: Format.bytes(model.rss)
                                color: Design.textMuted
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.06
                                horizontalAlignment: Text.AlignHCenter
                                text: model.state
                                color: page.stateColor(model.state)
                                font.weight: Font.DemiBold
                            }

                            Controls.Label {
                                Layout.fillWidth: true
                                text: model.cmd
                                color: Design.textMuted
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }
}

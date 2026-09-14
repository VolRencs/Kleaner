// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

pragma ComponentBehavior: Bound

Kirigami.Page {
    id: page

    padding: Design.pagePadding

    property int selectedPid: -1

    readonly property bool compactToolbar: width < 900

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
            inlineMessage.showError(message);
        }

        function onLoadingChanged() {
            if (!Processes.loading && page.selectedPid > 1 && !Processes.hasPid(page.selectedPid)) {
                page.selectedPid = -1;
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
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

        AppInlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
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
                display: page.compactToolbar ? Controls.AbstractButton.IconOnly : Controls.AbstractButton.TextBesideIcon
                enabled: page.selectedPid > 1
                onClicked: {
                    killDialog.pendingForce = false;
                    killDialog.open();
                }
            }

            AppButton {
                text: qsTr("Force Kill")
                icon.name: "edit-delete"
                destructive: true
                display: page.compactToolbar ? Controls.AbstractButton.IconOnly : Controls.AbstractButton.TextBesideIcon
                enabled: page.selectedPid > 1
                onClicked: {
                    killDialog.pendingForce = true;
                    killDialog.open();
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.Label {
                visible: page.selectedPid > 1 && !page.compactToolbar
                text: qsTr("PID %1 selected").arg(page.selectedPid)
                color: Design.textMuted
                font.pointSize: Design.smallFontSize
            }

            AppButton {
                icon.name: "view-refresh"
                display: Controls.AbstractButton.IconOnly
                text: qsTr("Refresh")
                spinning: Processes.loading
                enabled: !Processes.loading
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

                AppSeparator {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: processList.count === 0

                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        width: Math.min(implicitWidth, parent.width - Design.space20 * 2)
                        icon.name: "view-list-details"
                        text: qsTr("No processes found")
                        explanation: Processes.filter.length > 0
                                     ? qsTr("Try a different search term.")
                                     : qsTr("No running processes were reported by the system.")
                    }
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

                        required property int pid
                        required property string name
                        required property string user
                        required property string processState
                        required property real cpu
                        required property real mem
                        required property real rss
                        required property string cmd

                        width: processList.width
                        height: Design.rowHeightCompact
                        leftPadding: Design.space16
                        rightPadding: Design.space16
                        topPadding: 0
                        bottomPadding: 0
                        highlighted: page.selectedPid === pid
                        hoverEnabled: true
                        onClicked: page.selectedPid = pid

                        background: Rectangle {
                            color: "transparent"

                            Rectangle {
                                anchors.fill: parent
                                anchors.leftMargin: Design.itemInset
                                anchors.topMargin: Design.itemInset
                                anchors.bottomMargin: Design.itemInset
                                anchors.rightMargin: Design.itemInset + (processList.contentHeight > processList.height ? Design.scrollBarGutter : 0)
                                radius: Design.radiusItem
                                color: delegate.highlighted ? Design.accentSoft
                                                            : delegate.hovered ? Design.surfaceHover
                                                                               : Design.surfaceHoverClear

                                Behavior on color {
                                    ColorAnimation { duration: Design.durationNormal }
                                }
                            }
                        }

                        contentItem: RowLayout {
                            spacing: Design.space12

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.22
                                text: delegate.name
                                color: Design.text
                                font.weight: Font.DemiBold
                                elide: Text.ElideRight
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.06
                                text: delegate.pid
                                color: Design.textMuted
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.11
                                text: delegate.user
                                color: Design.textMuted
                                elide: Text.ElideRight
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.07
                                horizontalAlignment: Text.AlignRight
                                text: qsTr("%1%").arg(Format.number(delegate.cpu, 1))
                                color: delegate.cpu > 80 ? Design.negative : delegate.cpu > 40 ? Design.warning : Design.text
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.08
                                horizontalAlignment: Text.AlignRight
                                text: qsTr("%1%").arg(Format.number(delegate.mem, 1))
                                color: delegate.mem > 80 ? Design.negative : delegate.mem > 40 ? Design.warning : Design.text
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.09
                                horizontalAlignment: Text.AlignRight
                                text: Format.bytes(delegate.rss)
                                color: Design.textMuted
                            }

                            Controls.Label {
                                Layout.preferredWidth: processList.width * 0.06
                                horizontalAlignment: Text.AlignHCenter
                                text: delegate.processState
                                color: page.stateColor(delegate.processState)
                                font.weight: Font.DemiBold
                            }

                            Controls.Label {
                                Layout.fillWidth: true
                                text: delegate.cmd
                                color: Design.textMuted
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }

    ConfirmDialog {
        id: killDialog

        property bool pendingForce: false

        title: pendingForce ? qsTr("Force Kill Process") : qsTr("End Process")
        message: pendingForce
                 ? qsTr("Force kill process %1? Unsaved data in the application will be lost.").arg(page.selectedPid)
                 : qsTr("End process %1? Unsaved data in the application may be lost.").arg(page.selectedPid)
        confirmText: pendingForce ? qsTr("Force Kill") : qsTr("End Process")
        destructive: pendingForce
        onConfirmed: Processes.killPid(page.selectedPid, pendingForce)
    }
}

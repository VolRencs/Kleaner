import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: page

    title: qsTr("Processes")

    property int selectedPid: -1

    onVisibleChanged: {
        Processes.paused = !visible;
        if (visible) {
            Processes.update();
        }
    }

    Component.onCompleted: Processes.paused = !visible

    Connections {
        target: Processes
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
                placeholderText: qsTr("Search processes…")
                onTextChanged: Processes.filter = text
            }

            Controls.Label {
                text: qsTr("Sort by")
                color: Kirigami.Theme.disabledTextColor
            }

            Controls.ComboBox {
                id: sortComboBox
                textRole: "text"
                valueRole: "value"
                model: [
                    { text: qsTr("CPU"), value: Processes.SortCpu },
                    { text: qsTr("Memory"), value: Processes.SortMemory },
                    { text: qsTr("Name"), value: Processes.SortName },
                    { text: qsTr("PID"), value: Processes.SortPid },
                    { text: qsTr("User"), value: Processes.SortUser }
                ]
                onActivated: Processes.sortBy = currentValue
            }

            Controls.Button {
                icon.name: Processes.reverse ? "view-sort-descending" : "view-sort-ascending"
                display: Controls.AbstractButton.IconOnly
                text: qsTr("Reverse sort order")
                onClicked: Processes.reverse = !Processes.reverse
            }

            Controls.Button {
                icon.name: "view-refresh"
                display: Controls.AbstractButton.IconOnly
                text: qsTr("Refresh")
                onClicked: Processes.update()
            }

            Controls.BusyIndicator {
                running: Processes.loading
                visible: running
                implicitWidth: Kirigami.Units.gridUnit * 1.5
                implicitHeight: Kirigami.Units.gridUnit * 1.5
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Controls.Button {
                text: qsTr("End Process")
                icon.name: "process-stop"
                enabled: page.selectedPid > 1
                onClicked: Processes.killPid(page.selectedPid, false)
            }

            Controls.Button {
                text: qsTr("Force Kill")
                icon.name: "application-exit"
                enabled: page.selectedPid > 1
                onClicked: Processes.killPid(page.selectedPid, true)
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.Label {
                text: processList.count
                color: Kirigami.Theme.disabledTextColor
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Controls.Label {
                Layout.preferredWidth: page.width * 0.2
                text: qsTr("Name")
                font.bold: true
            }
            Controls.Label {
                Layout.preferredWidth: page.width * 0.07
                text: qsTr("PID")
                font.bold: true
            }
            Controls.Label {
                Layout.preferredWidth: page.width * 0.11
                text: qsTr("User")
                font.bold: true
            }
            Controls.Label {
                Layout.preferredWidth: page.width * 0.07
                text: qsTr("CPU")
                font.bold: true
                horizontalAlignment: Text.AlignRight
            }
            Controls.Label {
                Layout.preferredWidth: page.width * 0.07
                text: qsTr("Memory")
                font.bold: true
                horizontalAlignment: Text.AlignRight
            }
            Controls.Label {
                Layout.preferredWidth: page.width * 0.09
                text: qsTr("RSS")
                font.bold: true
                horizontalAlignment: Text.AlignRight
            }
            Controls.Label {
                Layout.preferredWidth: page.width * 0.05
                text: qsTr("State")
                font.bold: true
            }
            Controls.Label {
                Layout.fillWidth: true
                text: qsTr("Command")
                font.bold: true
            }
        }

        Controls.Label {
            Layout.fillWidth: true
            visible: processList.count === 0
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("No processes found.")
            color: Kirigami.Theme.disabledTextColor
        }

        ListView {
            id: processList

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: Processes
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Controls.ScrollBar {}

            delegate: Controls.ItemDelegate {
                width: processList.width
                highlighted: page.selectedPid === model.pid
                onClicked: page.selectedPid = model.pid

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Controls.Label {
                        Layout.preferredWidth: page.width * 0.2
                        text: model.name
                        elide: Text.ElideRight
                    }
                    Controls.Label {
                        Layout.preferredWidth: page.width * 0.07
                        text: model.pid
                    }
                    Controls.Label {
                        Layout.preferredWidth: page.width * 0.11
                        text: model.user
                        elide: Text.ElideRight
                    }
                    Controls.Label {
                        Layout.preferredWidth: page.width * 0.07
                        text: model.cpu.toFixed(1) + "%"
                        horizontalAlignment: Text.AlignRight
                    }
                    Controls.Label {
                        Layout.preferredWidth: page.width * 0.07
                        text: model.mem.toFixed(1) + "%"
                        horizontalAlignment: Text.AlignRight
                    }
                    Controls.Label {
                        Layout.preferredWidth: page.width * 0.09
                        text: Format.bytes(model.rss)
                        horizontalAlignment: Text.AlignRight
                    }
                    Controls.Label {
                        Layout.preferredWidth: page.width * 0.05
                        text: model.state
                    }
                    Controls.Label {
                        Layout.fillWidth: true
                        text: model.cmd
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }
}

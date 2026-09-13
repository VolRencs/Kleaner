import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: page

    title: qsTr("Services")

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
        spacing: Kirigami.Units.smallSpacing

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
            spacing: Kirigami.Units.smallSpacing

            Kirigami.SearchField {
                Layout.fillWidth: true
                placeholderText: qsTr("Search services…")
                onTextChanged: Services.filter = text
            }

            Controls.Button {
                icon.name: "view-refresh"
                display: Controls.AbstractButton.IconOnly
                text: qsTr("Refresh")
                onClicked: Services.reload()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Controls.Label {
                Layout.fillWidth: true
                text: qsTr("Service")
                font.bold: true
            }
            Controls.Label {
                Layout.preferredWidth: page.width * 0.3
                text: qsTr("Description")
                font.bold: true
                elide: Text.ElideRight
            }
            Controls.Label {
                Layout.preferredWidth: page.width * 0.12
                text: qsTr("Startup")
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }
            Controls.Label {
                Layout.preferredWidth: page.width * 0.16
                text: qsTr("State")
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Controls.Label {
            Layout.fillWidth: true
            visible: serviceList.count === 0
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("No services found.")
            color: Kirigami.Theme.disabledTextColor
        }

        ListView {
            id: serviceList

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: Services
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Controls.ScrollBar {}

            delegate: Controls.ItemDelegate {
                width: serviceList.width

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Controls.Label {
                        Layout.fillWidth: true
                        text: model.name
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Controls.Label {
                        Layout.preferredWidth: page.width * 0.3
                        text: model.description
                        color: Kirigami.Theme.disabledTextColor
                        elide: Text.ElideRight
                    }

                    Controls.Switch {
                        Layout.preferredWidth: page.width * 0.12
                        checked: model.enabled
                        onClicked: Services.setEnabled(index, checked)
                    }

                    RowLayout {
                        Layout.preferredWidth: page.width * 0.16
                        spacing: 0

                        Controls.Button {
                            Layout.fillWidth: true
                            display: Controls.AbstractButton.IconOnly
                            icon.name: model.active ? "media-playback-stop" : "media-playback-start"
                            text: model.active ? qsTr("Stop") : qsTr("Start")
                            onClicked: model.active ? Services.stop(index) : Services.start(index)
                        }

                        Controls.Button {
                            display: Controls.AbstractButton.IconOnly
                            icon.name: "view-refresh"
                            text: qsTr("Restart")
                            onClicked: Services.restart(index)
                        }
                    }
                }
            }
        }
    }
}

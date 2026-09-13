import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: page

    title: qsTr("Settings")

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.AbstractCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Controls.Label {
                    Layout.fillWidth: true
                    text: qsTr("Window")
                    font.bold: true
                }

                Controls.Label {
                    Layout.fillWidth: true
                    text: qsTr("Page shown on startup")
                    color: Kirigami.Theme.disabledTextColor
                }

                Controls.ComboBox {
                    id: startPageCombo
                    Layout.fillWidth: true
                    textRole: "text"
                    valueRole: "value"
                    model: [
                        { text: qsTr("Dashboard"), value: "dashboard" },
                        { text: qsTr("Resources"), value: "resources" },
                        { text: qsTr("Processes"), value: "processes" },
                        { text: qsTr("Services"), value: "services" },
                        { text: qsTr("Startup Apps"), value: "startup" },
                        { text: qsTr("System Cleaner"), value: "cleaner" },
                        { text: qsTr("Hosts"), value: "hosts" }
                    ]
                    Component.onCompleted: currentIndex = indexOfValue(Settings.startPage)
                    onActivated: Settings.startPage = currentValue
                }

                Controls.Label {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.smallSpacing
                    text: qsTr("When closing the window")
                    color: Kirigami.Theme.disabledTextColor
                }

                Controls.ComboBox {
                    id: closeBehaviorCombo
                    Layout.fillWidth: true
                    textRole: "text"
                    valueRole: "value"
                    model: [
                        { text: qsTr("Ask every time"), value: "ask" },
                        { text: qsTr("Keep running in the tray"), value: "tray" },
                        { text: qsTr("Quit"), value: "quit" }
                    ]
                    Component.onCompleted: currentIndex = indexOfValue(Settings.closeBehavior)
                    onActivated: Settings.closeBehavior = currentValue
                }
            }
        }

        Kirigami.AbstractCard {
            Layout.fillWidth: true
            visible: Tray.available

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Controls.Label {
                    Layout.fillWidth: true
                    text: qsTr("System Tray")
                    font.bold: true
                }

                Controls.Switch {
                    text: qsTr("Use the system tray icon")
                    checked: Settings.useTray
                    onClicked: Settings.useTray = checked
                }
            }
        }
    }
}

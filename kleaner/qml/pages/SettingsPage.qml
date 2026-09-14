// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

Kirigami.ScrollablePage {
    padding: Design.pagePadding

    ColumnLayout {
        width: parent.width
        spacing: Design.space16

        PageHeader {
            Layout.fillWidth: true
            Layout.bottomMargin: Design.space4
            title: qsTr("Settings")
            subtitle: qsTr("Configure how Kleaner starts and behaves")
        }

        AppCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: Design.space12

                Controls.Label {
                    Layout.fillWidth: true
                    text: qsTr("Startup")
                    color: Design.text
                    font.weight: Font.DemiBold
                }

                SettingRow {
                    Layout.fillWidth: true
                    text: qsTr("Page shown on startup")
                    description: qsTr("Which page Kleaner opens when launched.")

                    AppComboBox {
                        id: startPageCombo
                        Layout.preferredWidth: 220
                        textRole: "text"

                        property var options: [
                            { text: qsTr("Dashboard"), value: "dashboard" },
                            { text: qsTr("Resources"), value: "resources" },
                            { text: qsTr("Processes"), value: "processes" },
                            { text: qsTr("Services"), value: "services" },
                            { text: qsTr("Startup Apps"), value: "startup" },
                            { text: qsTr("System Cleaner"), value: "cleaner" },
                            { text: qsTr("Hosts"), value: "hosts" }
                        ]

                        model: options
                        onActivated: Settings.startPage = options[currentIndex].value

                        // A plain currentIndex binding would be destroyed by
                        // the combo box writing to it on activation.
                        Binding {
                            target: startPageCombo
                            property: "currentIndex"
                            value: Math.max(0, startPageCombo.indexOfValue(Settings.startPage))
                            restoreMode: Binding.RestoreBindingOrValue
                        }
                    }
                }
            }
        }

        AppCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: Design.space12

                Controls.Label {
                    Layout.fillWidth: true
                    text: qsTr("Window")
                    color: Design.text
                    font.weight: Font.DemiBold
                }

                SettingRow {
                    Layout.fillWidth: true
                    text: qsTr("When closing the window")
                    description: Tray.available && Settings.useTray
                    ? qsTr("Choose whether Kleaner keeps running in the system tray.")
                    : qsTr("Choose whether Kleaner asks, minimizes to the tray or quits.")

                    AppComboBox {
                        id: closeBehaviorCombo
                        Layout.preferredWidth: 220
                        textRole: "text"

                        property var options: {
                            const items = [
                                { text: qsTr("Ask every time"), value: "ask" },
                                { text: qsTr("Quit"), value: "quit" }
                            ];
                            if (Tray.available) {
                                items.splice(1, 0, { text: qsTr("Keep running in the tray"), value: "tray" });
                            }
                            return items;
                        }

                        model: options
                        onActivated: Settings.closeBehavior = options[currentIndex].value

                        Binding {
                            target: closeBehaviorCombo
                            property: "currentIndex"
                            value: Math.max(0, closeBehaviorCombo.indexOfValue(Settings.closeBehavior))
                            restoreMode: Binding.RestoreBindingOrValue
                        }
                    }
                }

                AppSeparator {
                    Layout.fillWidth: true
                    visible: Tray.available
                    Layout.preferredHeight: 1
                }

                SettingRow {
                    Layout.fillWidth: true
                    visible: Tray.available
                    text: qsTr("System tray")
                    description: qsTr("Show an icon in the system tray while Kleaner is running.")

                    AppSwitch {
                        id: traySwitch

                        checked: Settings.useTray
                        onClicked: Settings.useTray = checked

                        Binding {
                            target: traySwitch
                            property: "checked"
                            value: Settings.useTray
                            restoreMode: Binding.RestoreBindingOrValue
                        }
                    }
                }
            }
        }

        AppCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: Design.space12

                Controls.Label {
                    Layout.fillWidth: true
                    text: qsTr("Language")
                    color: Design.text
                    font.weight: Font.DemiBold
                }

                SettingRow {
                    Layout.fillWidth: true
                    text: qsTr("Interface language")
                    description: qsTr("Changes the language of the application interface.")

                    AppComboBox {
                        id: languageCombo
                        Layout.preferredWidth: 220
                        textRole: "text"

                        property var options: {
                            const items = [{ text: qsTr("System language"), value: "" }];
                            const languages = Settings.availableLanguages();
                            for (let i = 0; i < languages.length; ++i) {
                                items.push({ text: languages[i].name, value: languages[i].code });
                            }
                            return items;
                        }

                        model: options
                        onActivated: Settings.language = options[currentIndex].value

                        Binding {
                            target: languageCombo
                            property: "currentIndex"
                            value: Math.max(0, languageCombo.indexOfValue(Settings.language))
                            restoreMode: Binding.RestoreBindingOrValue
                        }
                    }
                }
            }
        }
    }
}
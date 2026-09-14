// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Item {
    id: page

    readonly property string projectUrl: "https://volrencs.github.io/Kleaner/"
    readonly property string issuesUrl: "https://github.com/VolRencs/Kleaner/issues"

    Controls.ScrollView {
        id: scroll

        anchors.fill: parent
        anchors.margins: Design.pagePadding
        clip: true
        Controls.ScrollBar.horizontal.policy: Controls.ScrollBar.AlwaysOff
        Controls.ScrollBar.vertical: AppScrollBar {}

        ColumnLayout {
            width: scroll.availableWidth
            spacing: Design.space20

            PageHeader {
                Layout.fillWidth: true
                title: qsTr("About")
                subtitle: qsTr("Information about this application")
            }

            Item {
                Layout.fillWidth: true
                Layout.topMargin: Design.space20
                implicitHeight: centeredColumn.implicitHeight

                ColumnLayout {
                    id: centeredColumn

                    anchors.horizontalCenter: parent.horizontalCenter
                    width: Math.min(implicitWidth, parent.width)
                    spacing: Design.space12

                    Image {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 96
                        Layout.preferredHeight: 96
                        source: "qrc:/kleaner-circle.svg"
                        sourceSize: Qt.size(96, 96)
                        smooth: true
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: "Kleaner"
                        color: Design.text
                        font.pointSize: Design.baseFontSize * 2.1
                        font.weight: Font.Bold
                    }

                    Badge {
                        Layout.alignment: Qt.AlignHCenter
                        text: Qt.application.version
                        badgeColor: Design.accent
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: Design.space4
                        text: qsTr("Linux System Optimizer and Monitoring")
                        color: Design.textMuted
                        font.pointSize: Design.subtitleFontSize
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: Design.space8
                        Layout.preferredWidth: Math.min(560, page.width - Design.pagePadding * 2)
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        text: qsTr("Kleaner is a Qt 6 and KDE Frameworks 6 fork of Stacer with a modern dark interface. It monitors system resources, manages services and startup entries, and cleans up unneeded files.")
                        color: Design.textMuted
                        font.pointSize: Design.smallFontSize
                    }

                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: Design.space12
                        spacing: Design.space8

                        AppButton {
                            text: qsTr("Project website")
                            icon.name: "globe"
                            onClicked: Qt.openUrlExternally(page.projectUrl)
                        }

                        AppButton {
                            text: qsTr("Report an issue")
                            icon.name: "tools-report-bug"
                            onClicked: Qt.openUrlExternally(page.issuesUrl)
                        }
                    }
                }
            }

            AppCard {
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: Design.space12
                Layout.preferredWidth: Math.min(640, scroll.availableWidth)

                contentItem: ColumnLayout {
                    spacing: Design.space12

                    Controls.Label {
                        Layout.fillWidth: true
                        text: qsTr("Credits")
                        color: Design.text
                        font.weight: Font.DemiBold
                    }

                    InfoRow {
                        Layout.fillWidth: true
                        iconName: "user-identity"
                        label: qsTr("Developer")
                        value: "VolRen"
                    }

                    InfoRow {
                        Layout.fillWidth: true
                        iconName: "user"
                        label: qsTr("Original project")
                        value: "Stacer by Quentin Lienhardt"
                    }

                    InfoRow {
                        Layout.fillWidth: true
                        iconName: "application-x-executable"
                        label: qsTr("Framework")
                        value: "Qt 6 · KDE Frameworks 6"
                    }

                    InfoRow {
                        Layout.fillWidth: true
                        iconName: "license"
                        label: qsTr("License")
                        value: "GPL-3.0"
                    }

                    InfoRow {
                        Layout.fillWidth: true
                        iconName: "clock"
                        label: qsTr("Copyright")
                        value: "© 2026 VolRen"
                    }
                }
            }
        }
    }
}

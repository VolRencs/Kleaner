// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Item {
    id: page

    readonly property var rootDisk: {
        const list = Disks.disks;
        for (let i = 0; i < list.length; ++i) {
            if (list[i].mountPoint === "/") {
                return list[i];
            }
        }
        return list.length > 0 ? list[0] : null;
    }

    readonly property bool wide: width > 820

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
                title: qsTr("Dashboard")
                subtitle: qsTr("Live overview of your system")

                Badge {
                    Layout.alignment: Qt.AlignVCenter
                    text: qsTr("Uptime %1").arg(Format.duration(SystemInformation.uptimeSeconds))
                    badgeColor: Design.textMuted
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: page.wide ? 3 : 1
                columnSpacing: Design.space16
                rowSpacing: Design.space16

                AppCard {
                    Layout.fillWidth: true

                    contentItem: ColumnLayout {
                        spacing: Design.space12

                        Gauge {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 156
                            Layout.preferredHeight: 156
                            title: qsTr("CPU")
                            value: Cpu.usage
                            valueText: Format.percent(Cpu.usage)
                            accent: Design.accent
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            text: qsTr("%1 cores").arg(Cpu.coreCount) + (Cpu.clock > 0 ? " · " + Cpu.clock.toFixed(0) + " MHz" : "")
                            color: Design.textMuted
                            font.pointSize: Design.smallFontSize
                        }
                    }
                }

                AppCard {
                    Layout.fillWidth: true

                    contentItem: ColumnLayout {
                        spacing: Design.space12

                        Gauge {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 156
                            Layout.preferredHeight: 156
                            title: qsTr("Memory")
                            value: Memory.usagePercent
                            valueText: Format.percent(Memory.usagePercent)
                            accent: Design.violet
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            text: Format.bytes(Memory.used) + " / " + Format.bytes(Memory.total)
                            color: Design.textMuted
                            font.pointSize: Design.smallFontSize
                        }
                    }
                }

                AppCard {
                    Layout.fillWidth: true

                    contentItem: ColumnLayout {
                        spacing: Design.space12

                        Gauge {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 156
                            Layout.preferredHeight: 156
                            title: qsTr("Disk")
                            value: page.rootDisk ? page.rootDisk.percent : 0
                            valueText: page.rootDisk ? Format.percent(page.rootDisk.percent) : "—"
                            accent: Design.orange
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            text: page.rootDisk
                                  ? Format.bytes(page.rootDisk.used) + " / " + Format.bytes(page.rootDisk.total)
                                  : qsTr("No disk detected")
                            color: Design.textMuted
                            font.pointSize: Design.smallFontSize
                        }
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                columns: page.width > 900 ? 2 : 1
                columnSpacing: Design.space16
                rowSpacing: Design.space16

                AppCard {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop

                    contentItem: ColumnLayout {
                        spacing: Design.space12

                        Controls.Label {
                            Layout.fillWidth: true
                            text: qsTr("System Information")
                            color: Design.text
                            font.weight: Font.DemiBold
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "computer"
                            label: qsTr("Hostname")
                            value: SystemInformation.hostname
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "computer"
                            label: qsTr("Distribution")
                            value: SystemInformation.distribution
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "application-x-executable"
                            label: qsTr("Kernel")
                            value: SystemInformation.kernel
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "cpu"
                            label: qsTr("CPU")
                            value: SystemInformation.cpuModel + " (" + Cpu.coreCount + ")"
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "user"
                            label: qsTr("User")
                            value: SystemInformation.username
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "clock"
                            label: qsTr("Uptime")
                            value: Format.duration(SystemInformation.uptimeSeconds)
                        }
                    }
                }

                AppCard {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop

                    contentItem: ColumnLayout {
                        spacing: Design.space12

                        Controls.Label {
                            Layout.fillWidth: true
                            text: qsTr("Live Metrics")
                            color: Design.text
                            font.weight: Font.DemiBold
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "speedometer"
                            label: qsTr("Load average")
                            value: Cpu.load1.toFixed(2) + " / " + Cpu.load5.toFixed(2) + " / " + Cpu.load15.toFixed(2)
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "cpu"
                            label: qsTr("CPU clock")
                            value: Cpu.clock > 0 ? Cpu.clock.toFixed(0) + " MHz" : "—"
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "memory"
                            label: qsTr("Swap")
                            value: Memory.swapTotal > 0
                                   ? Format.bytes(Memory.swapUsed) + " / " + Format.bytes(Memory.swapTotal)
                                   : qsTr("No swap")
                            valueColor: Memory.swapTotal > 0 && Memory.swapPercent > 75 ? Design.warning : Design.text
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "network-wired"
                            label: qsTr("Interface")
                            value: Network.connected && Network.interface.length > 0 ? Network.interface : qsTr("Disconnected")
                            valueColor: Network.connected ? Design.text : Design.negative
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "arrow-down"
                            label: qsTr("Network")
                            value: "↓ " + Format.bytes(Network.rxRate) + "/s    ↑ " + Format.bytes(Network.txRate) + "/s"
                            valueColor: Design.positive
                        }

                        InfoRow {
                            Layout.fillWidth: true
                            iconName: "drive-harddisk"
                            label: qsTr("Disk I/O")
                            value: "R " + Format.bytes(Disks.readRate) + "/s    W " + Format.bytes(Disks.writeRate) + "/s"
                        }
                    }
                }
            }

            AppCard {
                Layout.fillWidth: true

                contentItem: ColumnLayout {
                    spacing: Design.space16

                    Controls.Label {
                        Layout.fillWidth: true
                        text: qsTr("Storage")
                        color: Design.text
                        font.weight: Font.DemiBold
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        visible: Disks.disks.length === 0
                        text: qsTr("No mounted disks found.")
                        color: Design.textMuted
                    }

                    Repeater {
                        model: Disks.disks

                        delegate: ColumnLayout {
                            Layout.fillWidth: true
                            spacing: Design.space8

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: Design.space12

                                Controls.Label {
                                    text: modelData.name === "root" ? "/" : modelData.mountPoint
                                    color: Design.text
                                    font.weight: Font.DemiBold
                                }

                                Badge {
                                    Layout.alignment: Qt.AlignVCenter
                                    text: modelData.fileSystemType
                                    badgeColor: Design.textMuted
                                }

                                Item {
                                    Layout.fillWidth: true
                                }

                                Controls.Label {
                                    text: Format.bytes(modelData.used) + " / " + Format.bytes(modelData.total)
                                    color: Design.textMuted
                                    font.pointSize: Design.smallFontSize
                                }

                                Controls.Label {
                                    Layout.preferredWidth: 56
                                    horizontalAlignment: Text.AlignRight
                                    text: Format.percent(modelData.percent)
                                    color: modelData.percent > 90 ? Design.negative
                                                                  : modelData.percent > 75 ? Design.warning
                                                                                           : Design.text
                                    font.weight: Font.DemiBold
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 6
                                radius: 3
                                color: Design.track

                                Rectangle {
                                    width: parent.width * Math.min(1, modelData.percent / 100)
                                    height: parent.height
                                    radius: 3
                                    color: modelData.percent > 90 ? Design.negative
                                                                  : modelData.percent > 75 ? Design.warning
                                                                                           : Design.accent

                                    Behavior on width {
                                        NumberAnimation {
                                            duration: 400
                                            easing.type: Easing.OutCubic
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

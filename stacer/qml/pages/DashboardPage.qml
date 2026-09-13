import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: page

    title: qsTr("Dashboard")

    readonly property var rootDisk: {
        const list = Disks.disks;
        for (let i = 0; i < list.length; ++i) {
            if (list[i].mountPoint === "/") {
                return list[i];
            }
        }
        return list.length > 0 ? list[0] : null;
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing

            Donut {
                Layout.fillWidth: true
                Layout.preferredHeight: 210
                title: qsTr("CPU")
                value: Cpu.usage
                valueText: Format.percent(Cpu.usage)
                chartColor: Kirigami.Theme.positiveTextColor
            }

            Donut {
                Layout.fillWidth: true
                Layout.preferredHeight: 210
                title: qsTr("Memory")
                value: Memory.usagePercent
                valueText: Format.bytes(Memory.used)
                chartColor: Kirigami.Theme.neutralTextColor
            }

            Donut {
                Layout.fillWidth: true
                Layout.preferredHeight: 210
                title: qsTr("Disk")
                value: page.rootDisk ? page.rootDisk.percent : 0
                valueText: page.rootDisk ? Format.bytes(page.rootDisk.used) : "—"
                chartColor: Kirigami.Theme.negativeTextColor
            }
        }

        Kirigami.AbstractCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Controls.Label {
                    Layout.fillWidth: true
                    text: qsTr("System Information")
                    font.bold: true
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: Kirigami.Units.largeSpacing
                    rowSpacing: Kirigami.Units.smallSpacing

                    Controls.Label {
                        text: qsTr("Hostname")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        Layout.fillWidth: true
                        text: SystemInformation.hostname
                    }

                    Controls.Label {
                        text: qsTr("Distribution")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        Layout.fillWidth: true
                        text: SystemInformation.distribution
                    }

                    Controls.Label {
                        text: qsTr("Kernel")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        Layout.fillWidth: true
                        text: SystemInformation.kernel
                    }

                    Controls.Label {
                        text: qsTr("CPU")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        Layout.fillWidth: true
                        text: SystemInformation.cpuModel + " (" + Cpu.coreCount + ")"
                        elide: Text.ElideRight
                    }

                    Controls.Label {
                        text: qsTr("User")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        Layout.fillWidth: true
                        text: SystemInformation.username
                    }

                    Controls.Label {
                        text: qsTr("Uptime")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        Layout.fillWidth: true
                        text: Format.duration(SystemInformation.uptimeSeconds)
                    }
                }
            }
        }

        Kirigami.AbstractCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Controls.Label {
                    Layout.fillWidth: true
                    text: qsTr("Live")
                    font.bold: true
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 4
                    columnSpacing: Kirigami.Units.largeSpacing
                    rowSpacing: Kirigami.Units.smallSpacing

                    Controls.Label {
                        text: qsTr("Load average")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        text: Cpu.load1.toFixed(2) + " / " + Cpu.load5.toFixed(2) + " / " + Cpu.load15.toFixed(2)
                    }

                    Controls.Label {
                        text: qsTr("CPU clock")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        text: Cpu.clock > 0 ? Cpu.clock.toFixed(0) + " MHz" : "—"
                    }

                    Controls.Label {
                        text: qsTr("Memory swap")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        text: Memory.swapTotal > 0 ? Format.percent(Memory.swapPercent) : qsTr("No swap")
                    }

                    Controls.Label {
                        text: qsTr("Interface")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        text: Network.interface.length > 0 ? Network.interface : qsTr("Disconnected")
                    }

                    Controls.Label {
                        text: qsTr("Network")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        text: "↓ " + Format.bytes(Network.rxRate) + "/s  ↑ " + Format.bytes(Network.txRate) + "/s"
                    }

                    Controls.Label {
                        text: qsTr("Disk I/O")
                        color: Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        text: "R " + Format.bytes(Disks.readRate) + "/s  W " + Format.bytes(Disks.writeRate) + "/s"
                    }
                }
            }
        }
    }
}

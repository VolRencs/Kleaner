// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Item {
    id: page

    readonly property bool wide: width > 900

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
                title: qsTr("Resources")
                subtitle: qsTr("60-second history of CPU, memory, disk and network activity")
            }

            GridLayout {
                Layout.fillWidth: true
                columns: page.wide ? 2 : 1
                columnSpacing: Design.space16
                rowSpacing: Design.space16

                ChartCard {
                    Layout.fillWidth: true
                    Layout.columnSpan: page.wide ? 2 : 1
                    Layout.preferredHeight: 280
                    title: qsTr("CPU Usage")
                    valueText: Format.percent(Cpu.usage)
                    yMax: 100
                    names: [qsTr("Usage")]
                    seriesColors: [Design.accent]
                    valueSources: [History.cpuUsage]
                }

                ChartCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 240
                    title: qsTr("CPU Load Average")
                    valueText: Cpu.load1.toFixed(2)
                    automaticYRange: true
                    yMax: Math.max(1, Cpu.load1 * 1.2)
                    names: [qsTr("1 min"), qsTr("5 min"), qsTr("15 min")]
                    seriesColors: [Design.accent, Design.positive, Design.warning]
                    valueSources: [History.load1, History.load5, History.load15]
                }

                ChartCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 240
                    title: qsTr("Memory and Swap")
                    valueText: Format.percent(Memory.usagePercent)
                    yMax: 100
                    names: [qsTr("Memory"), qsTr("Swap")]
                    seriesColors: [Design.violet, Design.cyan]
                    valueSources: [History.memoryUsage, History.swapUsage]
                }

                ChartCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 240
                    title: qsTr("Disk Read / Write")
                    valueText: "R " + Format.bytes(Disks.readRate) + "/s"
                    automaticYRange: true
                    yMax: Math.max(1, Disks.readRate * 1.2, Disks.writeRate * 1.2)
                    names: [qsTr("Read"), qsTr("Write")]
                    seriesColors: [Design.cyan, Design.orange]
                    valueSources: [History.diskRead, History.diskWrite]
                }

                ChartCard {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 240
                    title: qsTr("Network Download / Upload")
                    valueText: "↓ " + Format.bytes(Network.rxRate) + "/s"
                    automaticYRange: true
                    yMax: Math.max(1, Network.rxRate * 1.2, Network.txRate * 1.2)
                    names: [qsTr("Download"), qsTr("Upload")]
                    seriesColors: [Design.positive, Design.accent]
                    valueSources: [History.networkRx, History.networkTx]
                }
            }
        }
    }
}

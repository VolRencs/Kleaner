// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

Item {
    id: page

    // Distinct colour per core, cycling through the application palette.
    readonly property var coreColors: [
        Design.accent, Design.positive, Design.violet,
        Design.cyan, Design.orange, Design.warning
    ]

    function coreColor(index) {
        return page.coreColors[index % page.coreColors.length];
    }

    AppScrollView {
        id: scroll

        anchors.fill: parent
        anchors.margins: Design.pagePadding

        ColumnLayout {
            width: scroll.availableWidth
            spacing: Design.space20

            PageHeader {
                Layout.fillWidth: true
                title: qsTr("Resources")
                subtitle: qsTr("60-second history of CPU, memory, disk and network activity")
            }

            ChartCard {
                Layout.fillWidth: true
                title: qsTr("CPU Usage")
                yMax: 100
                fillStrength: 0.14
                lineWidth: 1.5
                headerValues: History.cpuUsage
                valueFormatter: function(value) { return Format.percent(value); }
                series: {
                    const cores = [];
                    for (let i = 0; i < History.cpuCores.length; ++i) {
                        cores.push({
                            name: qsTr("Core %1").arg(i),
                            color: page.coreColor(i),
                            values: History.cpuCores[i]
                        });
                    }
                    return cores;
                }
            }

            ChartCard {
                Layout.fillWidth: true
                title: qsTr("CPU Load Average")
                automaticYRange: true
                valueFormatter: function(value) { return value.toFixed(2); }
                series: [
                    { name: qsTr("1 min"), color: Design.accent, values: History.load1 },
                    { name: qsTr("5 min"), color: Design.positive, values: History.load5 },
                    { name: qsTr("15 min"), color: Design.warning, values: History.load15 }
                ]
            }

            ChartCard {
                Layout.fillWidth: true
                title: qsTr("Memory and Swap")
                yMax: 100
                valueFormatter: function(value) { return Format.percent(value); }
                series: [
                    { name: qsTr("Memory"), color: Design.violet, values: History.memoryUsage },
                    { name: qsTr("Swap"), color: Design.cyan, values: History.swapUsage }
                ]
            }

            ChartCard {
                Layout.fillWidth: true
                title: qsTr("Disk Read / Write")
                automaticYRange: true
                valueFormatter: function(value) { return Format.bytes(value) + "/s"; }
                series: [
                    { name: qsTr("Read"), color: Design.cyan, values: History.diskRead },
                    { name: qsTr("Write"), color: Design.orange, values: History.diskWrite }
                ]
            }

            ChartCard {
                Layout.fillWidth: true
                title: qsTr("Network Download / Upload")
                automaticYRange: true
                valueFormatter: function(value) { return Format.bytes(value) + "/s"; }
                series: [
                    { name: qsTr("Download"), color: Design.positive, values: History.networkRx },
                    { name: qsTr("Upload"), color: Design.accent, values: History.networkTx }
                ]
            }
        }
    }
}

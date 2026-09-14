// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

Kirigami.ScrollablePage {
    id: page

    padding: Design.pagePadding

    // The history sampler only needs to run while the charts are visible.
    onVisibleChanged: History.active = visible
    Component.onCompleted: History.active = visible

    // Distinct colour per core, evenly spread over the hue wheel around the
    // accent colour: the same algorithm KDE System Monitor uses, so no two
    // cores share a colour.
    function coreColor(index, count) {
        if (count <= 0) {
            return Design.accent;
        }
        const hue = (Design.accent.hsvHue + index / count) % 1.0;
        return Qt.hsva(hue, Design.accent.hsvSaturation, Design.accent.hsvValue, 1.0);
    }

    ColumnLayout {
        width: parent.width
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
            lineWidth: 1.5
            legendValueWidth: 48
            headerValues: History.cpuUsage
            valueFormatter: function(value) { return Format.percent(value); }
            series: {
                const cores = [];
                for (let i = 0; i < History.cpuCores.length; ++i) {
                    cores.push({
                        name: qsTr("Core %1").arg(i),
                        color: page.coreColor(i, History.cpuCores.length),
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
            legendValueWidth: 52
            valueFormatter: function(value) { return Format.number(value, 2); }
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
            legendValueWidth: 48
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
            legendValueWidth: 76
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
            legendValueWidth: 76
            valueFormatter: function(value) { return Format.bytes(value) + "/s"; }
            series: [
                { name: qsTr("Download"), color: Design.positive, values: History.networkRx },
                { name: qsTr("Upload"), color: Design.accent, values: History.networkTx }
            ]
        }
    }
}
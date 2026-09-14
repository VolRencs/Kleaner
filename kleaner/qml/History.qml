// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

pragma Singleton

import QtQuick

import org.kde.quickcharts as Charts

// Keeps 60-second histories for the resource charts. Living in a singleton
// means the data survives page switches: reopening Resources never resets
// the graphs.
QtObject {
    id: history

    readonly property var cpuUsage: Charts.HistoryProxySource {
        source: Charts.ArraySource { array: [Cpu.usage] }
        interval: 1000
        maximumHistory: 60
    }

    readonly property var load1: Charts.HistoryProxySource {
        source: Charts.ArraySource { array: [Cpu.load1] }
        interval: 1000
        maximumHistory: 60
    }

    readonly property var load5: Charts.HistoryProxySource {
        source: Charts.ArraySource { array: [Cpu.load5] }
        interval: 1000
        maximumHistory: 60
    }

    readonly property var load15: Charts.HistoryProxySource {
        source: Charts.ArraySource { array: [Cpu.load15] }
        interval: 1000
        maximumHistory: 60
    }

    readonly property var memoryUsage: Charts.HistoryProxySource {
        source: Charts.ArraySource { array: [Memory.usagePercent] }
        interval: 1000
        maximumHistory: 60
    }

    readonly property var swapUsage: Charts.HistoryProxySource {
        source: Charts.ArraySource { array: [Memory.swapPercent] }
        interval: 1000
        maximumHistory: 60
    }

    readonly property var diskRead: Charts.HistoryProxySource {
        source: Charts.ArraySource { array: [Disks.readRate] }
        interval: 1000
        maximumHistory: 60
    }

    readonly property var diskWrite: Charts.HistoryProxySource {
        source: Charts.ArraySource { array: [Disks.writeRate] }
        interval: 1000
        maximumHistory: 60
    }

    readonly property var networkRx: Charts.HistoryProxySource {
        source: Charts.ArraySource { array: [Network.rxRate] }
        interval: 1000
        maximumHistory: 60
    }

    readonly property var networkTx: Charts.HistoryProxySource {
        source: Charts.ArraySource { array: [Network.txRate] }
        interval: 1000
        maximumHistory: 60
    }
}

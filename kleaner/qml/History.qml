// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

pragma Singleton

import QtQuick
import Kleaner

// Keeps 60-second histories for the resource charts. Living in a singleton
// means the data survives page switches: reopening Resources never resets
// the graphs. Plain arrays are used instead of chart proxy sources so the
// page can also compute min/avg/max for the statistics.
QtObject {
    id: history

    readonly property int maximumHistory: 60

    property var cpuUsage: []
    property var cpuCores: []
    property var load1: []
    property var load5: []
    property var load15: []
    property var memoryUsage: []
    property var swapUsage: []
    property var diskRead: []
    property var diskWrite: []
    property var networkRx: []
    property var networkTx: []

    function push(series, value) {
        const next = series.slice();
        next.push(value);
        if (next.length > maximumHistory) {
            next.shift();
        }
        return next;
    }

    function minimum(series) {
        let result = 0;
        for (let i = 0; i < series.length; ++i) {
            if (i === 0 || series[i] < result) {
                result = series[i];
            }
        }
        return result;
    }

    function maximum(series) {
        let result = 0;
        for (let i = 0; i < series.length; ++i) {
            if (i === 0 || series[i] > result) {
                result = series[i];
            }
        }
        return result;
    }

    function average(series) {
        let total = 0;
        for (let i = 0; i < series.length; ++i) {
            total += series[i];
        }
        return series.length > 0 ? total / series.length : 0;
    }

    readonly property Timer sampler: Timer {
        interval: 1000
        running: true
        repeat: true
        triggeredOnStart: true

        onTriggered: {
            history.cpuUsage = history.push(history.cpuUsage, Cpu.usage);

            const coreReadings = Cpu.coreUsages;
            const coreSeries = [];
            for (let i = 0; i < coreReadings.length; ++i) {
                const previous = i < history.cpuCores.length ? history.cpuCores[i] : [];
                coreSeries.push(history.push(previous, coreReadings[i]));
            }
            history.cpuCores = coreSeries;
            history.load1 = history.push(history.load1, Cpu.load1);
            history.load5 = history.push(history.load5, Cpu.load5);
            history.load15 = history.push(history.load15, Cpu.load15);
            history.memoryUsage = history.push(history.memoryUsage, Memory.usagePercent);
            history.swapUsage = history.push(history.swapUsage, Memory.swapPercent);
            history.diskRead = history.push(history.diskRead, Disks.readRate);
            history.diskWrite = history.push(history.diskWrite, Disks.writeRate);
            history.networkRx = history.push(history.networkRx, Network.rxRate);
            history.networkTx = history.push(history.networkTx, Network.txRate);
        }
    }
}

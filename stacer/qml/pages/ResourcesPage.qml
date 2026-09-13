import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.quickcharts as Charts

Kirigami.ScrollablePage {
    id: page

    title: qsTr("Resources")

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        ChartCard {
            Layout.fillWidth: true
            Layout.preferredHeight: 260
            title: qsTr("CPU Usage")
            yMax: 100
            names: [qsTr("Usage")]

            valueSources: [
                Charts.HistoryProxySource {
                    source: Charts.ArraySource {
                        array: [Cpu.usage]
                    }
                    interval: 1000
                    maximumHistory: 60
                }
            ]
        }

        ChartCard {
            Layout.fillWidth: true
            Layout.preferredHeight: 260
            title: qsTr("CPU Load Average")
            automaticYRange: true
            yMax: Math.max(1, Cpu.load1 * 1.2)
            names: [qsTr("1 min"), qsTr("5 min"), qsTr("15 min")]

            valueSources: [
                Charts.HistoryProxySource {
                    source: Charts.ArraySource {
                        array: [Cpu.load1]
                    }
                    interval: 1000
                    maximumHistory: 60
                },
                Charts.HistoryProxySource {
                    source: Charts.ArraySource {
                        array: [Cpu.load5]
                    }
                    interval: 1000
                    maximumHistory: 60
                },
                Charts.HistoryProxySource {
                    source: Charts.ArraySource {
                        array: [Cpu.load15]
                    }
                    interval: 1000
                    maximumHistory: 60
                }
            ]
        }

        ChartCard {
            Layout.fillWidth: true
            Layout.preferredHeight: 260
            title: qsTr("Memory and Swap")
            yMax: 100
            names: [qsTr("Memory"), qsTr("Swap")]

            valueSources: [
                Charts.HistoryProxySource {
                    source: Charts.ArraySource {
                        array: [Memory.usagePercent]
                    }
                    interval: 1000
                    maximumHistory: 60
                },
                Charts.HistoryProxySource {
                    source: Charts.ArraySource {
                        array: [Memory.swapPercent]
                    }
                    interval: 1000
                    maximumHistory: 60
                }
            ]
        }

        ChartCard {
            Layout.fillWidth: true
            Layout.preferredHeight: 260
            title: qsTr("Disk Read / Write")
            automaticYRange: true
            yMax: Math.max(Disks.readRate, Disks.writeRate)
            names: [qsTr("Read"), qsTr("Write")]

            valueSources: [
                Charts.HistoryProxySource {
                    source: Charts.ArraySource {
                        array: [Disks.readRate]
                    }
                    interval: 1000
                    maximumHistory: 60
                },
                Charts.HistoryProxySource {
                    source: Charts.ArraySource {
                        array: [Disks.writeRate]
                    }
                    interval: 1000
                    maximumHistory: 60
                }
            ]
        }

        ChartCard {
            Layout.fillWidth: true
            Layout.preferredHeight: 260
            title: qsTr("Network Download / Upload")
            automaticYRange: true
            yMax: Math.max(Network.rxRate, Network.txRate)
            names: [qsTr("Download"), qsTr("Upload")]

            valueSources: [
                Charts.HistoryProxySource {
                    source: Charts.ArraySource {
                        array: [Network.rxRate]
                    }
                    interval: 1000
                    maximumHistory: 60
                },
                Charts.HistoryProxySource {
                    source: Charts.ArraySource {
                        array: [Network.txRate]
                    }
                    interval: 1000
                    maximumHistory: 60
                }
            ]
        }
    }
}

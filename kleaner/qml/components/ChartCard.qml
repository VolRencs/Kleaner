// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.quickcharts as Charts

AppCard {
    id: root

    property string title
    property string valueText
    property var seriesColors: [Design.accent]
    property alias names: nameSource.array
    property alias valueSources: lineChart.valueSources
    property bool automaticYRange: false
    property real yMax: 100
    property int historySeconds: 60

    contentItem: ColumnLayout {
        spacing: Design.space12

        RowLayout {
            Layout.fillWidth: true
            spacing: Design.space8

            Controls.Label {
                Layout.fillWidth: true
                text: root.title
                color: Design.text
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            Controls.Label {
                visible: root.valueText.length > 0
                text: root.valueText
                color: Design.accent
                font.weight: Font.DemiBold
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: Kirigami.Units.gridUnit * 7
            clip: true

            Repeater {
                model: 4

                delegate: Rectangle {
                    width: parent.width
                    height: 1
                    y: Math.round(parent.height * (index + 1) / 5)
                    color: Design.border
                    opacity: 0.7
                }
            }

            Charts.LineChart {
                id: lineChart

                anchors.fill: parent

                lineWidth: 2
                fillOpacity: 0.14

                colorSource: Charts.ArraySource {
                    array: root.seriesColors
                }
                fillColorSource: Charts.ArraySource {
                    array: root.seriesColors
                }
                nameSource: Charts.ArraySource {
                    id: nameSource
                    array: []
                }

                xRange.from: 0
                xRange.to: root.historySeconds
                xRange.automatic: false

                yRange.from: 0
                yRange.to: Math.max(1, root.yMax)
                yRange.automatic: root.automaticYRange
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Design.space16
            visible: root.names !== undefined && root.names.length > 0

            Repeater {
                model: root.names

                delegate: RowLayout {
                    spacing: 6

                    Rectangle {
                        Layout.alignment: Qt.AlignVCenter
                        Layout.preferredWidth: 8
                        Layout.preferredHeight: 8
                        radius: 4
                        color: root.seriesColors[index % Math.max(1, root.seriesColors.length)]
                    }

                    Controls.Label {
                        text: modelData
                        color: Design.textMuted
                        font.pointSize: Design.smallFontSize
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}

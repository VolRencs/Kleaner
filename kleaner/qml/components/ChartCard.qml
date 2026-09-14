// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import Kleaner

pragma ComponentBehavior: Bound

// History chart card in the spirit of the KDE System Monitor "History" page:
// translucent gradient area fills, a labelled grid, live statistics for the
// primary series and a legend with current values.
AppCard {
    id: root

    property string title
    property var series: []
    property var headerValues: []
    property real yMax: 100
    property bool automaticYRange: false
    property bool fillAreas: true
    // Opacity of the area fill drawn under every series, matching the KDE
    // System Monitor default (lineChartFillOpacity = 10%).
    property real fillOpacity: 0.10
    property real lineWidth: 1.8
    property real legendValueWidth: 80
    property int historySeconds: 60
    property int hoveredIndex: -1
    property var valueFormatter: function(value) { return Format.number(value, 2); }

    // Values used for the header and the statistics line. A dedicated set can
    // be supplied when the card shows many series, e.g. per-core CPU usage
    // with the total load in the header.
    readonly property var statsValues: root.headerValues.length > 0 ? root.headerValues : root.primaryValues
    readonly property var primarySeries: root.series.length > 0 ? root.series[0] : null
    readonly property var primaryValues: root.primarySeries !== null ? root.primarySeries.values : []
    readonly property real primaryValue: root.statsValues.length > 0 ? root.statsValues[root.statsValues.length - 1] : 0

    onSeriesChanged: canvas.requestPaint()
    onHoveredIndexChanged: canvas.requestPaint()

    function seriesValue(entry) {
        return entry.values.length > 0 ? entry.values[entry.values.length - 1] : 0;
    }

    function seriesAt(index) {
        return index >= 0 && index < root.series.length ? root.series[index] : null;
    }

    contentItem: ColumnLayout {
        spacing: Design.space8

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
                Layout.alignment: Qt.AlignVCenter
                text: root.valueFormatter(root.primaryValue)
                color: root.primarySeries !== null ? root.primarySeries.color : Design.accent
                font.weight: Font.DemiBold
            }
        }

        Controls.Label {
            Layout.fillWidth: true
            visible: root.statsValues.length > 1
            text: qsTr("min %1 · avg %2 · max %3")
                  .arg(root.valueFormatter(History.minimum(root.statsValues)))
                  .arg(root.valueFormatter(History.average(root.statsValues)))
                  .arg(root.valueFormatter(History.maximum(root.statsValues)))
            color: Design.textFaint
            font.pointSize: Design.tinyFontSize
            elide: Text.ElideRight
        }

        Canvas {
            id: canvas

            Layout.fillWidth: true
            Layout.preferredHeight: 170
            Layout.topMargin: Design.space4
            Layout.bottomMargin: Design.space4

            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
            onPaint: root.paintChart(canvas)
        }

        Flow {
            Layout.fillWidth: true
            spacing: Design.space8

            Repeater {
                // A stable model keeps the delegates (and therefore the hover
                // state) alive while the values change every second.
                model: root.series.length

                delegate: Rectangle {
                    required property int index

                    radius: Design.radiusItem
                    color: root.hoveredIndex === index ? Design.surfaceHover : "transparent"
                    implicitWidth: legendRow.implicitWidth + Design.space8
                    implicitHeight: legendRow.implicitHeight + 2 * Design.space4

                    readonly property var entry: root.seriesAt(index)

                    HoverHandler {
                        onHoveredChanged: root.hoveredIndex = hovered ? index : -1
                    }

                    RowLayout {
                        id: legendRow

                        anchors.centerIn: parent
                        spacing: Design.space8

                        Rectangle {
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: Design.radiusItem
                            Layout.preferredHeight: legendName.implicitHeight
                            color: entry !== null ? entry.color : "transparent"
                        }

                        Controls.Label {
                            id: legendName

                            Layout.alignment: Qt.AlignVCenter
                            text: entry !== null ? entry.name : ""
                            color: root.hoveredIndex === index ? Design.text : Design.textMuted
                            font.pointSize: Design.smallFontSize
                        }

                        Controls.Label {
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: root.legendValueWidth
                            horizontalAlignment: Text.AlignRight
                            text: entry !== null ? root.valueFormatter(root.seriesValue(entry)) : ""
                            color: Design.text
                            font.pointSize: Design.smallFontSize
                            font.weight: Font.DemiBold
                        }
                    }
                }
            }
        }
    }

    function paintChart(item) {
        const ctx = item.getContext("2d");
        const w = item.width;
        const h = item.height;
        if (w <= 0 || h <= 0) {
            return;
        }

        ctx.clearRect(0, 0, w, h);

        const left = 58;
        const right = 10;
        const top = 6;
        const bottom = 16;
        const plotWidth = Math.max(1, w - left - right);
        const plotHeight = Math.max(1, h - top - bottom);

        let maxValue = root.yMax;
        if (root.automaticYRange) {
            maxValue = 0;
            for (let s = 0; s < root.series.length; ++s) {
                const values = root.series[s].values;
                for (let i = 0; i < values.length; ++i) {
                    if (values[i] > maxValue) {
                        maxValue = values[i];
                    }
                }
            }
            maxValue = maxValue > 0 ? maxValue * 1.15 : 1;
        }
        if (!(maxValue > 0)) {
            maxValue = 1;
        }

        const yFor = function(value) {
            return top + plotHeight * (1 - Math.min(1, Math.max(0, value / maxValue)));
        };
        // Oldest sample on the left, newest on the right: the line grows from
        // the left and scrolls once the window is full.
        const xFor = function(index) {
            if (root.historySeconds <= 1) {
                return left + plotWidth;
            }
            return left + plotWidth * index / (root.historySeconds - 1);
        };

        ctx.font = Math.max(9, Math.round(Design.tinyFontSize * 1.35)) + "px " + Design.fontFamily;
        ctx.lineWidth = 1;

        // Horizontal grid and value labels.
        const rows = 4;
        ctx.textAlign = "right";
        ctx.textBaseline = "middle";
        for (let i = 0; i <= rows; ++i) {
            const y = top + plotHeight * i / rows;
            ctx.strokeStyle = Qt.rgba(Design.border.r, Design.border.g, Design.border.b, i === rows ? 0.9 : 0.5);
            ctx.beginPath();
            ctx.moveTo(left, y);
            ctx.lineTo(w - right, y);
            ctx.stroke();

            ctx.fillStyle = Design.textFaint;
            ctx.fillText(root.valueFormatter(maxValue * (1 - i / rows)), left - 8, y);
        }

        // Time labels.
        ctx.textBaseline = "top";
        ctx.fillStyle = Design.textFaint;
        ctx.textAlign = "left";
        ctx.fillText(qsTr("-%1s").arg(root.historySeconds), left, h - bottom + 3);
        ctx.textAlign = "center";
        ctx.fillText(qsTr("-%1s").arg(Math.round(root.historySeconds / 2)), left + plotWidth / 2, h - bottom + 3);
        ctx.textAlign = "right";
        ctx.fillText(qsTr("now"), w - right, h - bottom + 3);

        // Dimmed series are drawn first and the hovered one last, so the
        // highlighted line always stays on top (KDE System Monitor style).
        const order = [];
        for (let s = 0; s < root.series.length; ++s) {
            if (s !== root.hoveredIndex) {
                order.push(s);
            }
        }
        if (root.hoveredIndex >= 0 && root.hoveredIndex < root.series.length) {
            order.push(root.hoveredIndex);
        }

        for (let o = 0; o < order.length; ++o) {
            const s = order[o];
            const entry = root.series[s];
            const values = entry.values;
            const color = entry.color;
            if (values.length === 0) {
                continue;
            }

            const highlighted = s === root.hoveredIndex;
            const dimmed = root.hoveredIndex >= 0 && !highlighted;
            const lineColor = dimmed ? Qt.rgba(color.r, color.g, color.b, 0.25) : color;
            const strength = root.fillOpacity * (dimmed ? 0.3 : 1.0);

            if (values.length < 2) {
                ctx.beginPath();
                ctx.arc(xFor(0), yFor(values[0]), highlighted ? 3.2 : 2.4, 0, Math.PI * 2);
                ctx.fillStyle = lineColor;
                ctx.fill();
                continue;
            }

            const firstX = xFor(0);
            const lastX = xFor(values.length - 1);

            if (root.fillAreas) {
                ctx.beginPath();
                for (let i = 0; i < values.length; ++i) {
                    const x = xFor(i);
                    const y = yFor(values[i]);
                    if (i === 0) {
                        ctx.moveTo(x, y);
                    } else {
                        ctx.lineTo(x, y);
                    }
                }
                ctx.lineTo(lastX, top + plotHeight);
                ctx.lineTo(firstX, top + plotHeight);
                ctx.closePath();

                ctx.fillStyle = Qt.rgba(color.r, color.g, color.b, strength);
                ctx.fill();
            }

            ctx.beginPath();
            for (let i = 0; i < values.length; ++i) {
                const x = xFor(i);
                const y = yFor(values[i]);
                if (i === 0) {
                    ctx.moveTo(x, y);
                } else {
                    ctx.lineTo(x, y);
                }
            }
            ctx.lineJoin = "round";

            if (highlighted) {
                ctx.strokeStyle = Qt.rgba(color.r, color.g, color.b, 0.22);
                ctx.lineWidth = root.lineWidth + 3.5;
                ctx.stroke();
                ctx.strokeStyle = color;
                ctx.lineWidth = root.lineWidth + 0.6;
                ctx.stroke();
            } else {
                ctx.strokeStyle = lineColor;
                ctx.lineWidth = root.lineWidth;
                ctx.stroke();
            }
        }
    }
}

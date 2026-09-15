// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import Kleaner

Item {
    id: root

    property string title
    property real value: 0
    property string valueText: Format.percent(root.value)
    property color accent: Design.accent
    property real thickness: 10

    readonly property real clamped: Math.max(0, Math.min(100, root.value))

    implicitWidth: 160
    implicitHeight: 160

    onClampedChanged: canvas.requestPaint()
    onAccentChanged: canvas.requestPaint()

    Canvas {
        id: canvas

        // Larger than the gauge itself so the glow is not clipped by the canvas
        // bounds, but small enough to stay inside the surrounding card padding.
        anchors.centerIn: parent
        width: root.width + root.thickness * 3
        height: root.height + root.thickness * 3

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            const ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            const ringSize = Math.min(root.width, root.height);
            const lineWidth = Math.min(root.thickness, ringSize * 0.12);
            const center = width / 2;
            const radius = ringSize / 2 - lineWidth / 2 - 2;

            ctx.lineCap = "round";
            ctx.lineWidth = lineWidth;

            ctx.beginPath();
            ctx.strokeStyle = Design.track;
            ctx.arc(center, center, radius, 0, Math.PI * 2);
            ctx.stroke();

            if (root.progress > 0.2) {
                const start = -Math.PI / 2;
                const end = start + Math.PI * 2 * (root.progress / 100);

                // Cheap layered glow instead of an expensive canvas shadow.
                ctx.save();
                for (let layer = 4; layer >= 1; --layer) {
                    ctx.beginPath();
                    ctx.globalAlpha = 0.055;
                    ctx.lineWidth = lineWidth + layer * 3;
                    ctx.strokeStyle = root.accent;
                    ctx.arc(center, center, radius, start, end);
                    ctx.stroke();
                }
                ctx.restore();

                const gradient = ctx.createLinearGradient(center - radius, center - radius, center + radius, center + radius);
                gradient.addColorStop(0, root.accent);
                gradient.addColorStop(1, Qt.lighter(root.accent, 1.35));

                ctx.beginPath();
                ctx.lineWidth = lineWidth;
                ctx.strokeStyle = gradient;
                ctx.arc(center, center, radius, start, end);
                ctx.stroke();
            }
        }
    }

    property real progress: root.clamped

    Behavior on progress {
        NumberAnimation {
            duration: Design.durationGauge
            easing.type: Easing.OutCubic
        }
    }

    onProgressChanged: canvas.requestPaint()

    Column {
        anchors.centerIn: parent
        spacing: 2

        Controls.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.valueText
            color: Design.text
            font.pointSize: Design.baseFontSize * 1.7
            font.weight: Font.DemiBold
        }

        Controls.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.title
            color: Design.textMuted
            font.pointSize: Design.smallFontSize
        }
    }
}

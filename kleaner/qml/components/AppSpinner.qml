// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick

Item {
    id: root

    property color color: Design.accent
    property bool running: true
    property real thickness: 3

    implicitWidth: 24
    implicitHeight: 24

    onColorChanged: canvas.requestPaint()

    Canvas {
        id: canvas

        anchors.fill: parent
        visible: root.running

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            const ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            const size = Math.min(width, height);
            const lineWidth = root.thickness;
            const center = size / 2;
            const radius = center - lineWidth / 2 - 1;

            ctx.lineWidth = lineWidth;
            ctx.lineCap = "round";

            ctx.beginPath();
            ctx.strokeStyle = Design.track;
            ctx.arc(center, center, radius, 0, Math.PI * 2);
            ctx.stroke();

            ctx.beginPath();
            ctx.strokeStyle = root.color;
            ctx.arc(center, center, radius, -Math.PI / 2, Math.PI * 0.9);
            ctx.stroke();
        }

        RotationAnimator on rotation {
            from: 0
            to: 360
            duration: 900
            loops: Animation.Infinite
            running: root.running && root.visible
        }
    }
}

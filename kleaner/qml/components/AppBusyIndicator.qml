// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import org.kde.kirigami as Kirigami
import Kleaner

// Kirigami-native busy indicator built on the platform "process-working"
// icon, the same one the Breeze busy indicator uses. Unlike rotating an
// arbitrary glyph, it can always be stopped cleanly and never leaves an icon
// frozen at a random angle.
Item {
    id: root

    property color color: Design.accent
    property bool running: false

    implicitWidth: Design.iconMedium
    implicitHeight: Design.iconMedium

    onRunningChanged: root.syncAnimation()
    Component.onCompleted: root.syncAnimation()

    function syncAnimation() {
        if (rotationAnimator.running === root.running) {
            return;
        }
        if (root.running) {
            // Start from a common phase so multiple indicators move in sync,
            // like the platform busy indicator does.
            const now = Date.now();
            const startAngle = (now % rotationAnimator.duration) / rotationAnimator.duration * 360;
            rotationAnimator.from = startAngle;
            rotationAnimator.to = startAngle + 360;
        }
        rotationAnimator.running = root.running;
    }

    Kirigami.Icon {
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height)
        height: width
        source: "process-working-symbolic"
        fallback: "process-working"
        color: root.color
        smooth: true

        visible: root.running || opacityAnimator.running
        opacity: root.running ? 1 : 0

        Behavior on opacity {
            OpacityAnimator {
                id: opacityAnimator
                duration: Design.durationFast
                easing.type: Easing.OutCubic
            }
        }

        RotationAnimator on rotation {
            id: rotationAnimator
            from: 0
            to: 360
            // The platform uses two seconds per revolution; do not scale this
            // with the animation speed settings, a busy spinner should be calm.
            duration: 2000
            loops: Animation.Infinite
            running: false
        }
    }
}

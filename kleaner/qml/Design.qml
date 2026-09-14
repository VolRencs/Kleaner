// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

pragma Singleton

import QtQuick

import org.kde.kirigami as Kirigami

QtObject {
    id: design

    function alpha(color, a) {
        return Qt.rgba(color.r, color.g, color.b, a);
    }

    // Surfaces
    readonly property color window: "#0f1115"
    readonly property color sidebar: "#0a0c0f"
    readonly property color surface: "#161a21"
    readonly property color surfaceHover: "#1c212b"
    readonly property color surfaceActive: "#232a35"
    readonly property color elevated: "#1a1f28"

    // Lines
    readonly property color border: "#232a35"
    readonly property color borderStrong: "#323c4c"
    readonly property color track: design.alpha("#ffffff", 0.07)

    // Text
    readonly property color text: "#eef1f6"
    readonly property color textMuted: "#99a3b4"
    readonly property color textFaint: "#5f6a7c"

    // Accents
    readonly property color accent: "#3daee9"
    readonly property color accentHover: "#5cbef1"
    readonly property color accentPressed: "#2f93c9"
    readonly property color accentText: "#08131a"
    readonly property color accentSoft: design.alpha(design.accent, 0.16)

    readonly property color positive: "#4cd07d"
    readonly property color warning: "#f5b74f"
    readonly property color negative: "#ff5f6d"
    readonly property color violet: "#a78bfa"
    readonly property color cyan: "#22d3ee"
    readonly property color orange: "#fb923c"

    // Geometry
    readonly property int radiusSmall: 8
    readonly property int radius: 12
    readonly property int radiusLarge: 16

    readonly property int space4: 4
    readonly property int space8: 8
    readonly property int space12: 12
    readonly property int space16: 16
    readonly property int space20: 20

    readonly property int pagePadding: 24
    readonly property int cardPadding: 18
    readonly property int controlHeight: 36

    readonly property int sidebarWidth: 238
    readonly property int sidebarCompactWidth: 72

    // Typography
    readonly property real baseFontSize: Kirigami.Theme.defaultFont.pointSize > 0
                                             ? Kirigami.Theme.defaultFont.pointSize
                                             : 10
    readonly property real titleFontSize: design.baseFontSize * 1.6
    readonly property real subtitleFontSize: design.baseFontSize * 0.95
    readonly property real smallFontSize: design.baseFontSize * 0.85
    readonly property real tinyFontSize: design.baseFontSize * 0.75
}

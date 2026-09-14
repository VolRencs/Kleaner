// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Layouts
import Kleaner

Item {
    default property alias content: row.data

    implicitHeight: row.implicitHeight + Design.space20

    RowLayout {
        id: row

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: Design.space20
        anchors.rightMargin: Design.space20
        anchors.bottomMargin: Design.space20
        spacing: Design.space8
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    }
}

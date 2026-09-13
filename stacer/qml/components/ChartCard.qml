import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls as ChartsControls

Kirigami.AbstractCard {
    id: root

    property string title
    property bool automaticYRange: false
    property real yMax: 100
    property int historySeconds: 60
    property alias names: control.names
    property alias valueSources: control.valueSources
    property alias chartColor: control.color

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Controls.Label {
            Layout.fillWidth: true
            text: root.title
            font.bold: true
        }

        ChartsControls.LineChartControl {
            id: control

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: Kirigami.Units.gridUnit * 8

            xRange.from: 0
            xRange.to: root.historySeconds
            xRange.automatic: false

            yRange.from: 0
            yRange.to: Math.max(1, root.yMax)
            yRange.automatic: root.automaticYRange

            legend.visible: root.valueSources !== undefined && root.valueSources.length > 1

            color: Kirigami.Theme.highlightColor
        }
    }
}

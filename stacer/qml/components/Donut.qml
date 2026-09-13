import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.quickcharts as Charts

Item {
    id: root

    property string title
    property real value: 0
    property string valueText: Format.percent(root.value)
    property color chartColor: Kirigami.Theme.highlightColor
    property color trackColor: Qt.rgba(Kirigami.Theme.textColor.r,
                                       Kirigami.Theme.textColor.g,
                                       Kirigami.Theme.textColor.b,
                                       0.15)

    readonly property real clampedValue: Math.max(0, Math.min(100, root.value))

    implicitWidth: 200
    implicitHeight: 200

    Charts.PieChart {
        anchors.fill: parent

        thickness: Math.max(10, Math.min(root.width, root.height) * 0.11)
        range.from: 0
        range.to: 100
        range.automatic: false

        valueSources: Charts.ArraySource {
            array: [root.clampedValue, 100 - root.clampedValue]
        }
        colorSource: Charts.ArraySource {
            array: [root.chartColor, root.trackColor]
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 0

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: root.valueText
            font.bold: true
            font.pointSize: Math.round(Kirigami.Theme.defaultFont.pointSize * 1.35)
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: root.title
            color: Kirigami.Theme.disabledTextColor
        }
    }
}

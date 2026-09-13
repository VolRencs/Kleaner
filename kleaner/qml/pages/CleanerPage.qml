import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: page

    title: qsTr("System Cleaner")

    property bool firstScanDone: false

    onVisibleChanged: {
        if (visible && !page.firstScanDone) {
            page.firstScanDone = true;
            Cleaner.scan();
        }
    }

    Connections {
        target: Cleaner
        function onCleanFinished(ok, message, count) {
            inlineMessage.type = ok ? Kirigami.MessageType.Positive : Kirigami.MessageType.Error;
            inlineMessage.text = ok
                ? qsTr("Removed %1 items.").arg(count)
                : message;
            inlineMessage.visible = true;
            hideMessageTimer.restart();
            if (ok) {
                Cleaner.scan();
            }
        }
    }

    Timer {
        id: hideMessageTimer
        interval: 6000
        onTriggered: inlineMessage.visible = false
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
            visible: false
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Controls.Button {
                icon.name: "search"
                text: qsTr("Scan")
                enabled: !Cleaner.scanning
                onClicked: Cleaner.scan()
            }

            Controls.Button {
                icon.name: "edit-clear"
                text: qsTr("Clean (%1)").arg(Format.bytes(Cleaner.checkedSize))
                enabled: !Cleaner.scanning && Cleaner.checkedSize > 0
                onClicked: Cleaner.clean()
            }

            Controls.BusyIndicator {
                running: Cleaner.scanning
                visible: running
                implicitWidth: Kirigami.Units.gridUnit * 1.5
                implicitHeight: Kirigami.Units.gridUnit * 1.5
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.Label {
                visible: !Cleaner.scanning
                text: qsTr("Selected: %1").arg(Format.bytes(Cleaner.checkedSize))
                color: Kirigami.Theme.disabledTextColor
            }
        }

        Controls.Label {
            Layout.fillWidth: true
            visible: cleanerList.count === 0 && !Cleaner.scanning
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Press Scan to look for removable files.")
            color: Kirigami.Theme.disabledTextColor
        }

        ListView {
            id: cleanerList

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: Cleaner
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: Controls.ScrollBar {}

            delegate: Controls.ItemDelegate {
                width: cleanerList.width

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Item {
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    }

                    Controls.Button {
                        visible: model.expandable
                        display: Controls.AbstractButton.IconOnly
                        flat: true
                        icon.name: model.expanded ? "go-down" : "go-next"
                        text: model.expanded ? qsTr("Collapse") : qsTr("Expand")
                        onClicked: Cleaner.toggleExpand(index)
                    }

                    Controls.CheckBox {
                        id: entryCheckBox
                        tristate: model.isCategory
                        checkState: model.checkState
                        onClicked: Cleaner.setChecked(index, checked)
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        leftPadding: model.depth > 0 ? Kirigami.Units.largeSpacing : 0
                        text: model.title
                        font.bold: model.isCategory
                        elide: Text.ElideRight
                    }

                    Controls.Label {
                        visible: model.root
                        text: qsTr("root")
                        color: Kirigami.Theme.neutralTextColor
                    }

                    Controls.Label {
                        text: model.size > 0 ? Format.bytes(model.size) : "—"
                        color: Kirigami.Theme.disabledTextColor
                    }
                }
            }
        }
    }
}

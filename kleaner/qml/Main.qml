// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

Controls.ApplicationWindow {
    id: root

    title: "Kleaner"
    visible: true
    width: 1240
    height: 800
    minimumWidth: 780
    minimumHeight: 540
    color: Design.window

    palette.active.window: Design.window
    palette.active.windowText: Design.text
    palette.active.base: Design.surface
    palette.active.alternateBase: Design.surfaceHover
    palette.active.text: Design.text
    palette.active.button: Design.surfaceHover
    palette.active.buttonText: Design.text
    palette.active.brightText: "#ffffff"
    palette.active.highlight: Design.accent
    palette.active.highlightedText: "#08131a"
    palette.active.placeholderText: Design.textFaint
    palette.disabled.windowText: Design.textFaint
    palette.disabled.text: Design.textFaint
    palette.disabled.buttonText: Design.textFaint
    palette.disabled.highlight: Design.border
    palette.disabled.highlightedText: Design.textFaint

    // Kleaner ships a single, consistent dark appearance.
    Kirigami.Theme.inherit: false
    Kirigami.Theme.backgroundColor: Design.window
    Kirigami.Theme.alternateBackgroundColor: Design.surface
    Kirigami.Theme.textColor: Design.text
    Kirigami.Theme.disabledTextColor: Design.textFaint
    Kirigami.Theme.highlightColor: Design.accent
    Kirigami.Theme.positiveTextColor: Design.positive
    Kirigami.Theme.neutralTextColor: Design.warning
    Kirigami.Theme.negativeTextColor: Design.negative

    readonly property var pageUrls: ({
        "dashboard": "pages/DashboardPage.qml",
        "resources": "pages/ResourcesPage.qml",
        "processes": "pages/ProcessesPage.qml",
        "services": "pages/ServicesPage.qml",
        "startup": "pages/StartupAppsPage.qml",
        "cleaner": "pages/CleanerPage.qml",
        "hosts": "pages/HostsPage.qml",
        "settings": "pages/SettingsPage.qml",
        "about": "pages/AboutKleanerPage.qml"
    })

    property string currentPage: root.pageUrls[Settings.startPage] !== undefined ? Settings.startPage : "dashboard"
    property bool forceQuit: false

    readonly property bool compactSidebar: width < 1000

    function pageUrl(id) {
        const relative = root.pageUrls[id] !== undefined ? root.pageUrls[id] : root.pageUrls["dashboard"];
        return Qt.resolvedUrl(relative);
    }

    function navigate(id) {
        const target = root.pageUrls[id] !== undefined ? id : "dashboard";
        if (target === root.currentPage && stack.currentItem !== null) {
            return;
        }
        root.currentPage = target;
        stack.replace(root.pageUrl(target));
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: sidebar

            Layout.fillHeight: true
            Layout.preferredWidth: root.compactSidebar ? Design.sidebarCompactWidth : Design.sidebarWidth
            color: Design.sidebar

            Behavior on Layout.preferredWidth {
                NumberAnimation {
                    duration: 180
                    easing.type: Easing.InOutQuad
                }
            }

            Rectangle {
                anchors.right: parent.right
                width: 1
                height: parent.height
                color: Design.border
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.topMargin: Design.space20
                anchors.bottomMargin: Design.space12
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 2

                Item {
                    Layout.fillWidth: true
                    Layout.bottomMargin: Design.space20
                    implicitHeight: 32

                    RowLayout {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 6
                        anchors.rightMargin: 6
                        visible: !root.compactSidebar
                        spacing: 10

                        Image {
                            Layout.preferredWidth: 30
                            Layout.preferredHeight: 30
                            source: "qrc:/kleaner-k.svg"
                            sourceSize: Qt.size(30, 30)
                            smooth: true
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0

                            Controls.Label {
                                text: "Kleaner"
                                color: Design.text
                                font.pointSize: Design.baseFontSize * 1.15
                                font.weight: Font.Bold
                            }

                            Controls.Label {
                                text: qsTr("System Optimizer")
                                color: Design.textFaint
                                font.pointSize: Design.tinyFontSize
                            }
                        }
                    }

                    Image {
                        anchors.centerIn: parent
                        visible: root.compactSidebar
                        width: 30
                        height: 30
                        source: "qrc:/kleaner-k.svg"
                        sourceSize: Qt.size(30, 30)
                        smooth: true
                    }
                }

                Controls.Label {
                    Layout.leftMargin: 10
                    Layout.topMargin: Design.space8
                    Layout.bottomMargin: 4
                    visible: !root.compactSidebar
                    text: qsTr("MONITOR")
                    color: Design.textFaint
                    font.pointSize: Design.tinyFontSize
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.topMargin: Design.space8
                    Layout.bottomMargin: 4
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    Layout.preferredHeight: 1
                    color: Design.border
                    visible: root.compactSidebar
                }

                NavItem {
                    Layout.fillWidth: true
                    compact: root.compactSidebar
                    iconName: "go-home"
                    text: qsTr("Dashboard")
                    selected: root.currentPage === "dashboard"
                    onClicked: root.navigate("dashboard")
                }

                NavItem {
                    Layout.fillWidth: true
                    compact: root.compactSidebar
                    iconName: "utilities-system-monitor"
                    text: qsTr("Resources")
                    selected: root.currentPage === "resources"
                    onClicked: root.navigate("resources")
                }

                NavItem {
                    Layout.fillWidth: true
                    compact: root.compactSidebar
                    iconName: "view-list-details"
                    text: qsTr("Processes")
                    selected: root.currentPage === "processes"
                    onClicked: root.navigate("processes")
                }

                Controls.Label {
                    Layout.leftMargin: 10
                    Layout.topMargin: Design.space16
                    Layout.bottomMargin: 4
                    visible: !root.compactSidebar
                    text: qsTr("SYSTEM")
                    color: Design.textFaint
                    font.pointSize: Design.tinyFontSize
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.topMargin: Design.space16
                    Layout.bottomMargin: 4
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    Layout.preferredHeight: 1
                    color: Design.border
                    visible: root.compactSidebar
                }

                NavItem {
                    Layout.fillWidth: true
                    compact: root.compactSidebar
                    iconName: "preferences-system-services"
                    text: qsTr("Services")
                    selected: root.currentPage === "services"
                    onClicked: root.navigate("services")
                }

                NavItem {
                    Layout.fillWidth: true
                    compact: root.compactSidebar
                    iconName: "system-run"
                    text: qsTr("Startup Apps")
                    selected: root.currentPage === "startup"
                    onClicked: root.navigate("startup")
                }

                Controls.Label {
                    Layout.leftMargin: 10
                    Layout.topMargin: Design.space16
                    Layout.bottomMargin: 4
                    visible: !root.compactSidebar
                    text: qsTr("TOOLS")
                    color: Design.textFaint
                    font.pointSize: Design.tinyFontSize
                    font.weight: Font.DemiBold
                    font.letterSpacing: 1
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.topMargin: Design.space16
                    Layout.bottomMargin: 4
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    Layout.preferredHeight: 1
                    color: Design.border
                    visible: root.compactSidebar
                }

                NavItem {
                    Layout.fillWidth: true
                    compact: root.compactSidebar
                    iconName: "edit-clear"
                    text: qsTr("System Cleaner")
                    selected: root.currentPage === "cleaner"
                    onClicked: root.navigate("cleaner")
                }

                NavItem {
                    Layout.fillWidth: true
                    compact: root.compactSidebar
                    iconName: "network-server"
                    text: qsTr("Hosts")
                    selected: root.currentPage === "hosts"
                    onClicked: root.navigate("hosts")
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.topMargin: Design.space8
                    Layout.bottomMargin: Design.space8
                    Layout.preferredHeight: 1
                    color: Design.border
                }

                NavItem {
                    Layout.fillWidth: true
                    compact: root.compactSidebar
                    iconName: "settings-configure"
                    text: qsTr("Settings")
                    selected: root.currentPage === "settings"
                    onClicked: root.navigate("settings")
                }

                NavItem {
                    Layout.fillWidth: true
                    compact: root.compactSidebar
                    iconName: "help-about"
                    text: qsTr("About")
                    selected: root.currentPage === "about"
                    onClicked: root.navigate("about")
                }
            }
        }

        Controls.StackView {
            id: stack

            Layout.fillWidth: true
            Layout.fillHeight: true

            Component.onCompleted: stack.replace(root.pageUrl(root.currentPage))

            replaceEnter: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 180
                    easing.type: Easing.OutCubic
                }
                NumberAnimation {
                    property: "y"
                    from: 14
                    to: 0
                    duration: 220
                    easing.type: Easing.OutCubic
                }
            }

            replaceExit: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 120
                }
            }
        }
    }

    onClosing: function(close) {
        if (root.forceQuit || Settings.closeBehavior === "quit") {
            Settings.sync();
            close.accepted = true;
            Qt.quit();
            return;
        }

        if (Settings.closeBehavior === "tray" && Settings.useTray && Tray.available) {
            close.accepted = false;
            root.hide();
            return;
        }

        close.accepted = false;
        closeDialog.open();
    }

    Binding {
        target: Tray
        property: "enabled"
        value: Settings.useTray && Tray.available
    }

    Connections {
        target: Tray

        function onShowRequested() {
            root.show();
            root.raise();
            root.requestActivate();
        }

        function onQuitRequested() {
            root.forceQuit = true;
            Qt.quit();
        }
    }

    AppDialog {
        id: closeDialog

        title: qsTr("Quit Kleaner")

        onOpened: dontAskCheckBox.checked = false

        contentItem: ColumnLayout {
            spacing: Design.space12

            Controls.Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: Design.text
                text: Tray.available && Settings.useTray
                      ? qsTr("Keep Kleaner running in the system tray?")
                      : qsTr("Do you want to quit Kleaner?")
            }

            AppCheckBox {
                id: dontAskCheckBox
                Layout.fillWidth: true
                visible: Tray.available && Settings.useTray
                text: qsTr("Do not ask again")
            }
        }

        footer: AppDialogFooter {
            AppButton {
                Layout.fillWidth: true
                text: qsTr("Quit")
                onClicked: {
                    if (dontAskCheckBox.checked) {
                        Settings.closeBehavior = "quit";
                    }
                    Settings.sync();
                    root.forceQuit = true;
                    closeDialog.close();
                    Qt.quit();
                }
            }

            AppButton {
                Layout.fillWidth: true
                text: qsTr("Keep in Tray")
                highlighted: true
                visible: Tray.available && Settings.useTray
                onClicked: {
                    if (dontAskCheckBox.checked) {
                        Settings.closeBehavior = "tray";
                    }
                    closeDialog.close();
                    root.hide();
                }
            }
        }
    }
}

// SPDX-FileCopyrightText: 2026 VolRen
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import Kleaner

pragma ComponentBehavior: Bound

Kirigami.ApplicationWindow {
    id: root

    title: root.pageTitles[root.currentPage] !== undefined
           ? "Kleaner — " + root.pageTitles[root.currentPage]
           : "Kleaner"
    visible: true
    width: 1240
    height: 800
    minimumWidth: 780
    minimumHeight: 540
    color: Design.window

    // Kleaner ships a single, consistent dark appearance. The QApplication
    // palette is applied in main.cpp; the Kirigami theme is completed here so
    // stock Kirigami components (messages, dialogs, placeholders) pick up the
    // exact same colours instead of the platform colour scheme.
    Kirigami.Theme.inherit: false
    Kirigami.Theme.backgroundColor: Design.window
    Kirigami.Theme.alternateBackgroundColor: Design.surface
    Kirigami.Theme.textColor: Design.text
    Kirigami.Theme.disabledTextColor: Design.textFaint
    Kirigami.Theme.activeTextColor: Design.text
    Kirigami.Theme.activeBackgroundColor: Design.surfaceHover
    Kirigami.Theme.highlightColor: Design.accent
    Kirigami.Theme.highlightedTextColor: Design.accentText
    Kirigami.Theme.focusColor: Design.accent
    Kirigami.Theme.hoverColor: Design.surfaceHover
    Kirigami.Theme.linkColor: Design.accent
    Kirigami.Theme.linkBackgroundColor: Design.accentSoft
    Kirigami.Theme.visitedLinkColor: Design.violet
    Kirigami.Theme.visitedLinkBackgroundColor: Design.alpha(Design.violet, 0.16)
    Kirigami.Theme.positiveTextColor: Design.positive
    Kirigami.Theme.neutralTextColor: Design.warning
    Kirigami.Theme.negativeTextColor: Design.negative
    Kirigami.Theme.positiveBackgroundColor: Design.alpha(Design.positive, 0.14)
    Kirigami.Theme.neutralBackgroundColor: Design.alpha(Design.warning, 0.14)
    Kirigami.Theme.negativeBackgroundColor: Design.alpha(Design.negative, 0.14)

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

    readonly property var pageTitles: ({
        "dashboard": qsTr("Dashboard"),
        "resources": qsTr("Resources"),
        "processes": qsTr("Processes"),
        "services": qsTr("Services"),
        "startup": qsTr("Startup Apps"),
        "cleaner": qsTr("System Cleaner"),
        "hosts": qsTr("Hosts"),
        "settings": qsTr("Settings"),
        "about": qsTr("About")
    })

    // Sidebar structure: sections with their entries, as before.
    readonly property var navSections: [
        {
            title: qsTr("MONITOR"),
            items: [
                { id: "dashboard", icon: "go-home", text: qsTr("Dashboard") },
                { id: "resources", icon: "utilities-system-monitor", text: qsTr("Resources") },
                { id: "processes", icon: "view-list-details", text: qsTr("Processes") }
            ]
        },
        {
            title: qsTr("SYSTEM"),
            items: [
                { id: "services", icon: "preferences-system-services", text: qsTr("Services") },
                { id: "startup", icon: "system-run", text: qsTr("Startup Apps") }
            ]
        },
        {
            title: qsTr("TOOLS"),
            items: [
                { id: "cleaner", icon: "edit-clear", text: qsTr("System Cleaner") },
                { id: "hosts", icon: "network-server", text: qsTr("Hosts") }
            ]
        }
    ]

    property string currentPage: root.pageUrls[Settings.startPage] !== undefined ? Settings.startPage : "dashboard"
    property bool forceQuit: false

    readonly property bool compactSidebar: width < 1000

    function pageUrl(id) {
        const relative = root.pageUrls[id] !== undefined ? root.pageUrls[id] : root.pageUrls["dashboard"];
        return Qt.resolvedUrl(relative);
    }

    Kirigami.PagePool {
        id: pool
    }

    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None
    pageStack.separatorVisible: false
    pageStack.leftSidebar: sidebarDrawer

    // Push the configured start page once; binding initialPage would reset the
    // stack every time currentPage changes. Restore the saved window size at
    // the same time, as a plain binding would fight user resizing.
    Component.onCompleted: {
        root.width = Settings.windowWidth;
        root.height = Settings.windowHeight;
        pageStack.initialPage = pool.loadPage(root.pageUrl(root.currentPage));
    }

    // Permanent, non-modal sidebar that pushes the page content like the old
    // hand-made RowLayout did, but managed by PageRow's own layout.
    Kirigami.OverlayDrawer {
        id: sidebarDrawer

        edge: Qt.LeftEdge
        modal: false
        interactive: false
        position: 1
        width: root.compactSidebar ? Design.sidebarCompactWidth : Design.sidebarWidth

        Behavior on width {
            NumberAnimation {
                duration: Design.durationPage
                easing.type: Easing.InOutQuad
            }
        }

        background: Rectangle {
            color: Design.sidebar

            Rectangle {
                anchors.right: parent.right
                width: 1
                height: parent.height
                color: Design.border
            }
        }

        contentItem: Item {
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
                                Layout.fillWidth: true
                                text: "Kleaner"
                                color: Design.text
                                font.pointSize: Design.baseFontSize * 1.15
                                font.weight: Font.Bold
                                elide: Text.ElideRight
                            }

                            Controls.Label {
                                Layout.fillWidth: true
                                text: qsTr("System Optimizer")
                                color: Design.textFaint
                                font.pointSize: Design.tinyFontSize
                                elide: Text.ElideRight
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

                Repeater {
                    model: root.navSections

                    delegate: ColumnLayout {
                        id: section

                        required property var modelData
                        required property int index

                        Layout.fillWidth: true
                        spacing: 2

                        Controls.Label {
                            Layout.fillWidth: true
                            Layout.leftMargin: 10
                            Layout.topMargin: section.index === 0 ? Design.space8 : Design.space16
                            Layout.bottomMargin: 4
                            visible: !root.compactSidebar
                            text: section.modelData.title
                            color: Design.textFaint
                            font.pointSize: Design.tinyFontSize
                            font.weight: Font.DemiBold
                            font.letterSpacing: 1
                            elide: Text.ElideRight
                        }

                        AppSeparator {
                            Layout.fillWidth: true
                            Layout.topMargin: section.index === 0 ? Design.space8 : Design.space16
                            Layout.bottomMargin: 4
                            Layout.leftMargin: 10
                            Layout.rightMargin: 10
                            Layout.preferredHeight: 1
                            visible: root.compactSidebar
                        }

                        Repeater {
                            model: section.modelData.items

                            delegate: NavItem {
                                id: navItem

                                required property var modelData

                                Layout.fillWidth: true
                                compact: root.compactSidebar
                                iconName: navItem.modelData.icon
                                text: navItem.modelData.text
                                selected: navAction.checked

                                Kirigami.PagePoolAction {
                                    id: navAction

                                    pagePool: pool
                                    pageStack: root.pageStack
                                    checkable: true
                                    text: navItem.text
                                    icon.name: navItem.iconName
                                    page: root.pageUrl(navItem.modelData.id)

                                    onTriggered: root.currentPage = navItem.modelData.id
                                }

                                onClicked: navAction.trigger()
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                AppSeparator {
                    Layout.fillWidth: true
                    Layout.topMargin: Design.space8
                    Layout.bottomMargin: Design.space8
                    Layout.preferredHeight: 1
                }

                NavItem {
                    id: settingsItem

                    Layout.fillWidth: true
                    compact: root.compactSidebar
                    iconName: "settings-configure"
                    text: qsTr("Settings")
                    selected: settingsAction.checked
                    onClicked: settingsAction.trigger()

                    Kirigami.PagePoolAction {
                        id: settingsAction

                        pagePool: pool
                        pageStack: root.pageStack
                        checkable: true
                        text: settingsItem.text
                        icon.name: settingsItem.iconName
                        page: root.pageUrl("settings")

                        onTriggered: root.currentPage = "settings"
                    }
                }

                NavItem {
                    id: aboutItem

                    Layout.fillWidth: true
                    compact: root.compactSidebar
                    iconName: "help-about"
                    text: qsTr("About")
                    selected: aboutAction.checked
                    onClicked: aboutAction.trigger()

                    Kirigami.PagePoolAction {
                        id: aboutAction

                        pagePool: pool
                        pageStack: root.pageStack
                        checkable: true
                        text: aboutItem.text
                        icon.name: aboutItem.iconName
                        page: root.pageUrl("about")

                        onTriggered: root.currentPage = "about"
                    }
                }
            }
        }
    }

    onClosing: function(close) {
        if (root.visibility !== Window.Maximized && root.visibility !== Window.FullScreen) {
            Settings.windowWidth = root.width;
            Settings.windowHeight = root.height;
        }

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
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root

    title: "Stacer"
    width: 1180
    height: 780
    minimumWidth: 820
    minimumHeight: 560

    readonly property var pageUrls: ({
        "dashboard": "pages/DashboardPage.qml",
        "resources": "pages/ResourcesPage.qml",
        "processes": "pages/ProcessesPage.qml",
        "services": "pages/ServicesPage.qml",
        "startup": "pages/StartupAppsPage.qml",
        "cleaner": "pages/CleanerPage.qml",
        "hosts": "pages/HostsPage.qml",
        "settings": "pages/SettingsPage.qml",
        "about": "pages/AboutStacerPage.qml"
    })

    function pageUrl(id) {
        const relative = root.pageUrls[id] !== undefined ? root.pageUrls[id] : root.pageUrls["dashboard"];
        return Qt.resolvedUrl(relative);
    }

    property bool forceQuit: false

    Kirigami.PagePool {
        id: pagePool
    }

    globalDrawer: Kirigami.GlobalDrawer {
        modal: false
        title: "Stacer"
        titleIcon: "stacer"

        actions: [
            Kirigami.PagePoolAction {
                text: qsTr("Dashboard")
                icon.name: "go-home"
                pagePool: pagePool
                page: root.pageUrl("dashboard")
            },
            Kirigami.PagePoolAction {
                text: qsTr("Resources")
                icon.name: "utilities-system-monitor"
                pagePool: pagePool
                page: root.pageUrl("resources")
            },
            Kirigami.PagePoolAction {
                text: qsTr("Processes")
                icon.name: "view-list-details"
                pagePool: pagePool
                page: root.pageUrl("processes")
            },
            Kirigami.PagePoolAction {
                text: qsTr("Services")
                icon.name: "preferences-system-services"
                pagePool: pagePool
                page: root.pageUrl("services")
            },
            Kirigami.PagePoolAction {
                text: qsTr("Startup Apps")
                icon.name: "system-run"
                pagePool: pagePool
                page: root.pageUrl("startup")
            },
            Kirigami.PagePoolAction {
                text: qsTr("System Cleaner")
                icon.name: "edit-clear"
                pagePool: pagePool
                page: root.pageUrl("cleaner")
            },
            Kirigami.PagePoolAction {
                text: qsTr("Hosts")
                icon.name: "network-server"
                pagePool: pagePool
                page: root.pageUrl("hosts")
            },
            Kirigami.PagePoolAction {
                text: qsTr("Settings")
                icon.name: "settings-configure"
                pagePool: pagePool
                page: root.pageUrl("settings")
            },
            Kirigami.PagePoolAction {
                text: qsTr("About")
                icon.name: "help-about"
                pagePool: pagePool
                page: root.pageUrl("about")
            }
        ]
    }

    pageStack.initialPage: pagePool.loadPage(root.pageUrl(Settings.startPage))

    Component.onCompleted: {
        if (pageStack.currentItem === null) {
            console.warn("Stacer: failed to load start page:", Settings.startPage);
        }
    }

    onClosing: function(close) {
        if (root.forceQuit || Settings.closeBehavior === "quit") {
            Settings.sync();
            close.accepted = true;
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

    Controls.Dialog {
        id: closeDialog

        title: qsTr("Quit Stacer")
        modal: true
        anchors.centerIn: parent
        standardButtons: Controls.Dialog.NoButton

        contentItem: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            Controls.Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: Tray.available && Settings.useTray
                      ? qsTr("Keep Stacer running in the system tray?")
                      : qsTr("Do you want to quit Stacer?")
            }

            Controls.CheckBox {
                id: dontAskCheckBox
                text: qsTr("Do not ask again")
                visible: Tray.available && Settings.useTray
            }
        }

        footer: RowLayout {
            spacing: Kirigami.Units.smallSpacing
            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft

            Controls.Button {
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

            Controls.Button {
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

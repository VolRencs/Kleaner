import QtQuick

import org.kde.kirigami as Kirigami

Kirigami.AboutPage {
    id: page

    aboutData: {
        "displayName": "Kleaner",
        "productName": "kleaner",
        "componentName": "kleaner",
        "shortDescription": qsTr("Linux System Optimizer and Monitoring"),
        "homepage": "https://github.com/QuentiumYT/Stacer",
        "bugAddress": "https://github.com/QuentiumYT/Stacer/issues",
        "programLogo": "qrc:/kleaner-logo.svg",
        "version": Qt.application.version,
        "copyrightStatement": "© 2026 Kleaner contributors (fork of Stacer by Quentin Lienhardt)",
        "otherText": "",
        "authors": [
            {
                "name": "QuentiumYT",
                "task": qsTr("Original Stacer project"),
                "emailAddress": "",
                "webAddress": "",
                "ocsUsername": ""
            }
        ],
        "credits": [],
        "translators": [],
        "licenses": [
            {
                "name": "GPL v3",
                "text": "",
                "spdx": "GPL-3.0"
            }
        ]
    }
}

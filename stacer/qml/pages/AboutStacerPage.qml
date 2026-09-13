import QtQuick

import org.kde.kirigami as Kirigami

Kirigami.AboutPage {
    id: page

    aboutData: {
        "displayName": "Stacer",
        "productName": "stacer",
        "componentName": "stacer",
        "shortDescription": qsTr("Linux System Optimizer and Monitoring"),
        "homepage": "https://github.com/QuentiumYT/Stacer",
        "bugAddress": "https://github.com/QuentiumYT/Stacer/issues",
        "version": Qt.application.version,
        "copyrightStatement": "© 2026 Stacer contributors",
        "otherText": "",
        "authors": [
            {
                "name": "QuentiumYT",
                "task": "",
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

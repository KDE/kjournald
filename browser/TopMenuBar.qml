/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.15 as Kirigami
import kjournald 1.0

MenuBar {
    signal copyViewToClipboard()
    Kirigami.Theme.colorSet: Kirigami.Theme.Header
    Menu {
        title: i18n("File")
        MenuItem {
            text: i18n("Open System Journal")
            icon.name: "document-open"
            onTriggered: {
                SessionConfigProxy.sessionMode = SessionConfig.SYSTEM
            }
        }
        MenuItem {
            text: i18n("Open from Folder")
            icon.name: "document-open"
            onTriggered: {
                folderDialog.folder = SessionConfigProxy.localJournalPath
                folderDialog.open()
            }
        }
        MenuItem {
            text: i18n("Open from File")
            icon.name: "document-open"
            onTriggered: {
                fileDialog.folder = SessionConfigProxy.localJournalPath
                fileDialog.open()
            }
        }
// disable feature for end-users until the dialog experience is more polished
//        MenuItem {
//            text: i18n("Open Remote Journal Stream")
//            icon.name: "document-import"
//            onTriggered: {
//                remoteJournalDialog.open()
//            }
//        }

        MenuSeparator { }

        MenuItem {
            text: i18n("Close")
            icon.name: "application-exit"
            onTriggered: Qt.quit()
        }
    }
    Menu {
        title: i18n("Current Journal")
        MenuItem {
            text: "Copy Current View"
            icon.name: "edit-copy"
            onTriggered: copyViewToClipboard()
        }
    }
    Menu {
        title: i18n("View")

        Menu {
            title: i18n("Timestamp Display")

            MenuItem {
                text: i18n("Localized Realtime")
                checkable: true
                checked: SessionConfigProxy.timeDisplay === SessionConfig.LOCALTIME
                onTriggered: {
                    SessionConfigProxy.timeDisplay = SessionConfig.LOCALTIME
                }
            }
            MenuItem {
                text: i18n("UTC Realtime")
                checkable: true
                checked: SessionConfigProxy.timeDisplay === SessionConfig.UTC
                onTriggered: {
                    SessionConfigProxy.timeDisplay = SessionConfig.UTC
                }
            }
            MenuItem {
                text: i18n("Monotonic Time")
                checkable: true
                checked: SessionConfigProxy.timeDisplay === SessionConfig.MONOTONIC_TIMESTAMP
                onTriggered: {
                    SessionConfigProxy.timeDisplay = SessionConfig.MONOTONIC_TIMESTAMP
                }
            }
        }
        Menu {
            title: i18n("Colorize")

            MenuItem {
                text: i18n("Systemd Unit")
                checkable: true
                checked: SessionConfigProxy.filterCriterium === SessionConfig.SYSTEMD_UNIT
                onTriggered: {
                    SessionConfigProxy.filterCriterium = SessionConfig.SYSTEMD_UNIT
                }
            }
            MenuItem {
                text: i18n("Executable")
                checkable: true
                checked: SessionConfigProxy.filterCriterium === SessionConfig.EXECUTABLE
                onTriggered: {
                    SessionConfigProxy.filterCriterium = SessionConfig.EXECUTABLE
                }
            }
        }
    }
    Menu {
        title: i18n("Help")
        MenuItem {
            text: i18n("About")
            icon.name: "help-about"
            onTriggered: aboutDialog.open()
        }
    }
}

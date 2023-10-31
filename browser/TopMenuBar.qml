/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import kjournald

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
                folderDialog.currentFolder = SessionConfigProxy.localJournalPath
                folderDialog.open()
            }
        }
        MenuItem {
            text: i18n("Open from File")
            icon.name: "document-open"
            onTriggered: {
                fileDialog.currentFolder = SessionConfigProxy.localJournalPath
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
                contentItem: RadioButton {
                    text: i18n("Localized Realtime")
                    checkable: true
                    checked: SessionConfigProxy.timeDisplay === SessionConfig.LOCALTIME
                    onToggled: {
                        SessionConfigProxy.timeDisplay = SessionConfig.LOCALTIME
                        parent.triggered()
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: i18n("UTC Realtime")
                    checkable: true
                    checked: SessionConfigProxy.timeDisplay === SessionConfig.UTC
                    onToggled: {
                        SessionConfigProxy.timeDisplay = SessionConfig.UTC
                        parent.triggered()
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: i18n("Monotonic Time")
                    checkable: true
                    checked: SessionConfigProxy.timeDisplay === SessionConfig.MONOTONIC_TIMESTAMP
                    onToggled: {
                        SessionConfigProxy.timeDisplay = SessionConfig.MONOTONIC_TIMESTAMP
                        parent.triggered()
                    }
                }
            }
        }
        Menu {
            title: i18n("Colorize")

            MenuItem {
                contentItem: RadioButton {
                    text: i18n("Systemd Unit")
                    checkable: true
                    checked: SessionConfigProxy.filterCriterium === SessionConfig.SYSTEMD_UNIT
                    onToggled: {
                        SessionConfigProxy.filterCriterium = SessionConfig.SYSTEMD_UNIT
                        parent.triggered()
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: i18n("Executable")
                    checkable: true
                    checked: SessionConfigProxy.filterCriterium === SessionConfig.EXECUTABLE
                    onToggled: {
                        SessionConfigProxy.filterCriterium = SessionConfig.EXECUTABLE
                        parent.triggered()
                    }
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

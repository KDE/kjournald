/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.12
import QtQuick.Controls 2.12
import systemd 1.0

MenuBar {
    Menu {
        title: "File"
        MenuItem {
            text: "Open system journal"
            icon.name: "document-open"
            onTriggered: {
                g_config.sessionMode = SessionConfig.SYSTEM
            }
        }
        MenuItem {
            text: "Open from folder"
            icon.name: "document-open"
            onTriggered: {
                folderDialog.folder = g_config.localJournalPath
                folderDialog.open()
            }
        }
        MenuItem {
            text: "Open from file"
            icon.name: "document-open"
            onTriggered: {
                fileDialog.folder = g_config.localJournalPath
                fileDialog.open()
            }
        }
        MenuItem {
            text: "Open remote journal stream"
            icon.name: "document-import"
            onTriggered: {
                remoteJournalDialog.open()
            }
        }

        MenuSeparator { }

        MenuItem {
            text: "Close"
            icon.name: "application-exit"
            onTriggered: Qt.quit()
        }
    }
    Menu {
        title: "Current Journal"
        MenuItem {
            text: "Copy current view"
            icon.name: "edit-copy"
            onTriggered: copyViewToClipbaord()
        }
    }
    Menu {
        title: "View"

        Menu {
            title: "Timestamp Display"

            MenuItem {
                text: "Localized Realtime"
                checkable: true
                checked: g_config.timeDisplay === SessionConfig.LOCALTIME
                onTriggered: {
                    g_config.timeDisplay = SessionConfig.LOCALTIME
                }
            }
            MenuItem {
                text: "UTC Realtime"
                checkable: true
                checked: g_config.timeDisplay === SessionConfig.UTC
                onTriggered: {
                    g_config.timeDisplay = SessionConfig.UTC
                }
            }
            MenuItem {
                text: "Monotonic Time"
                checkable: true
                checked: g_config.timeDisplay === SessionConfig.MONOTONIC_TIMESTAMP
                onTriggered: {
                    g_config.timeDisplay = SessionConfig.MONOTONIC_TIMESTAMP
                }
            }
        }
        Menu {
            title: "Filter"

            MenuItem {
                text: "Systemd Unit"
                checkable: true
                checked: g_config.filterCriterium === SessionConfig.SYSTEMD_UNIT
                onTriggered: {
                    g_config.filterCriterium = SessionConfig.SYSTEMD_UNIT
                }
            }
            MenuItem {
                text: "Executable"
                checkable: true
                checked: g_config.filterCriterium === SessionConfig.EXECUTABLE
                onTriggered: {
                    g_config.filterCriterium = SessionConfig.EXECUTABLE
                }
            }
        }
    }
}

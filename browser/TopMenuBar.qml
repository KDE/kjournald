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
            text: i18n("Open system journal")
            icon.name: "document-open"
            onTriggered: {
                g_config.sessionMode = SessionConfig.SYSTEM
            }
        }
        MenuItem {
            text: i18n("Open from folder")
            icon.name: "document-open"
            onTriggered: {
                folderDialog.folder = g_config.localJournalPath
                folderDialog.open()
            }
        }
        MenuItem {
            text: i18n("Open from file")
            icon.name: "document-open"
            onTriggered: {
                fileDialog.folder = g_config.localJournalPath
                fileDialog.open()
            }
        }
        MenuItem {
            text: i18n("Open remote journal stream")
            icon.name: "document-import"
            onTriggered: {
                remoteJournalDialog.open()
            }
        }

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
            text: "Copy current view"
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
                checked: g_config.timeDisplay === SessionConfig.LOCALTIME
                onTriggered: {
                    g_config.timeDisplay = SessionConfig.LOCALTIME
                }
            }
            MenuItem {
                text: i18n("UTC Realtime")
                checkable: true
                checked: g_config.timeDisplay === SessionConfig.UTC
                onTriggered: {
                    g_config.timeDisplay = SessionConfig.UTC
                }
            }
            MenuItem {
                text: i18n("Monotonic Time")
                checkable: true
                checked: g_config.timeDisplay === SessionConfig.MONOTONIC_TIMESTAMP
                onTriggered: {
                    g_config.timeDisplay = SessionConfig.MONOTONIC_TIMESTAMP
                }
            }
        }
        Menu {
            title: i18n("Colorize")

            MenuItem {
                text: i18n("Systemd Unit")
                checkable: true
                checked: g_config.filterCriterium === SessionConfig.SYSTEMD_UNIT
                onTriggered: {
                    g_config.filterCriterium = SessionConfig.SYSTEMD_UNIT
                }
            }
            MenuItem {
                text: i18n("Executable")
                checkable: true
                checked: g_config.filterCriterium === SessionConfig.EXECUTABLE
                onTriggered: {
                    g_config.filterCriterium = SessionConfig.EXECUTABLE
                }
            }
        }
    }
    Menu {
        title: i18n("Help")
        MenuItem {
            text: i18n("About")
            icon.name: "help-about"
            onTriggered: {
                aboutDialog.visible = true
                aboutDialog.open()
            }
        }
    }
}

// SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
// SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>

import Qt.labs.platform 1.1 as Labs

import kjournald 1.0

Labs.MenuBar {
    Labs.Menu {
        title: i18n("File")

        Labs.MenuItem {
            text: i18n("Open system journal")
            icon.name: "document-open"
            onTriggered: {
                g_config.sessionMode = SessionConfig.SYSTEM
            }
        }
        Labs.MenuItem {
            text: i18n("Open from folder")
            icon.name: "document-open"
            onTriggered: {
                folderDialog.folder = g_config.localJournalPath
                folderDialog.open()
            }
        }
        Labs.MenuItem {
            text: i18n("Open from file")
            icon.name: "document-open"
            onTriggered: {
                fileDialog.folder = g_config.localJournalPath
                fileDialog.open()
            }
        }
        Labs.MenuItem {
            text: i18n("Open remote journal stream")
            icon.name: "document-import"
            onTriggered: {
                remoteJournalDialog.open()
            }
        }

        Labs.MenuSeparator { }

        Labs.MenuItem {
            text: i18n("Close")
            icon.name: "application-exit"
            onTriggered: Qt.quit()
        }
    }
    Labs.Menu {
        title: i18n("Current Journal")
        Labs.MenuItem {
            text: "Copy current view"
            icon.name: "edit-copy"
            onTriggered: copyViewToClipboard()
        }
    }
    Labs.Menu {
        title: i18n("View")

        Labs.Menu {
            title: i18n("Timestamp Display")

            Labs.MenuItem {
                text: i18n("Localized Realtime")
                checkable: true
                checked: g_config.timeDisplay === SessionConfig.LOCALTIME
                onTriggered: {
                    g_config.timeDisplay = SessionConfig.LOCALTIME
                }
            }
            Labs.MenuItem {
                text: i18n("UTC Realtime")
                checkable: true
                checked: g_config.timeDisplay === SessionConfig.UTC
                onTriggered: {
                    g_config.timeDisplay = SessionConfig.UTC
                }
            }
            Labs.MenuItem {
                text: i18n("Monotonic Time")
                checkable: true
                checked: g_config.timeDisplay === SessionConfig.MONOTONIC_TIMESTAMP
                onTriggered: {
                    g_config.timeDisplay = SessionConfig.MONOTONIC_TIMESTAMP
                }
            }
        }
        Labs.Menu {
            title: i18n("Colorize")

            Labs.MenuItem {
                text: i18n("Systemd Unit")
                checkable: true
                checked: g_config.filterCriterium === SessionConfig.SYSTEMD_UNIT
                onTriggered: {
                    g_config.filterCriterium = SessionConfig.SYSTEMD_UNIT
                }
            }
            Labs.MenuItem {
                text: i18n("Executable")
                checkable: true
                checked: g_config.filterCriterium === SessionConfig.EXECUTABLE
                onTriggered: {
                    g_config.filterCriterium = SessionConfig.EXECUTABLE
                }
            }
        }
    }
    Labs.Menu {
        title: i18n("Help")
        Labs.MenuItem {
            text: i18n("About")
            icon.name: "help-about"
            onTriggered: {
                aboutDialog.visible = true
                aboutDialog.open()
            }
        }
    }
}

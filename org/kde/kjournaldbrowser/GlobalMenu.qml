// SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
// SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>

import Qt.labs.platform as Labs
import QtQuick.Dialogs
import QtQml
import org.kde.kirigamiaddons.statefulapp.labs as StatefulAppLabs
import org.kde.kirigamiaddons.statefulapp as StatefulApp
import org.kde.ki18n
import org.kde.kjournaldbrowser

Labs.MenuBar {
    id: root

    property StatefulApp.AbstractKirigamiApplication application: BrowserApplication
    required property FileDialog fileDialog
    required property FolderDialog folderDialog

    signal copyViewToClipboard()

    Labs.Menu {
        title: KI18n.i18nc("@title:menu", "File")

        Labs.MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Local journal")
            icon.name: "document-open"
            onTriggered: {
                DatabaseProvider.setLocalJournal()
            }
        }
        Labs.MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Open from folder")
            icon.name: "document-open"
            onTriggered: {
                root.folderDialog.folder = DatabaseProvider.localJournalPath
                root.folderDialog.open()
            }
        }
        Labs.MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Open from file")
            icon.name: "document-open"
            onTriggered: {
                root.fileDialog.folder = DatabaseProvider.localJournalPath
                root.fileDialog.open()
            }
        }
// disable option: it is not yet end-user ready
//        Labs.MenuItem {
//            text: KI18n.i18nc("@action:inmenu", "Open remote journal stream")
//            icon.name: "document-import"
//            onTriggered: {
//                remoteJournalDialog.open()
//            }
//        }

        Labs.MenuSeparator { }

        Labs.MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Close")
            icon.name: "application-exit"
            onTriggered: Qt.quit()
        }
    }
    Labs.Menu {
        title: KI18n.i18nc("@title:menu", "Current Journal")
        Labs.MenuItem {
            text: "Copy current view"
            icon.name: "edit-copy"
            onTriggered: root.copyViewToClipboard()
        }
        Labs.Menu {
            title: KI18n.i18nc("@title:menu", "Limit Accessed Logs")
            icon.name: "view-filter"
            Labs.MenuItem {
                text: KI18n.i18nc("@item:inmenu", "All logs")
                checkable: true
                checked: BrowserApplication.logViewMode === BrowserApplication.ALL_LOGS
                onTriggered: {
                    BrowserApplication.logViewMode = BrowserApplication.ALL_LOGS
                }
            }
            Labs.MenuItem {
                text: KI18n.i18nc("@item:inmenu", "Only current user logs")
                checkable: true
                checked: BrowserApplication.logViewMode === BrowserApplication.ONLY_USER
                onTriggered: {
                    BrowserApplication.logViewMode = BrowserApplication.ONLY_USER
                }
            }
            Labs.MenuItem {
                text: KI18n.i18nc("@item:inmenu", "Only system logs")
                checkable: true
                checked: BrowserApplication.logViewMode === BrowserApplication.ONLY_SYSTEM
                onTriggered: {
                    BrowserApplication.logViewMode = BrowserApplication.ONLY_SYSTEM
                }
            }
        }
    }
    Labs.Menu {
        title: KI18n.i18nc("@title:menu", "View")

        Labs.Menu {
            title: KI18n.i18nc("@title:menu", "Timestamp Display")
            icon.name: "clock"

            Labs.MenuItem {
                text: KI18n.i18nc("@item:inmenu", "Localized Realtime")
                checkable: true
                checked: BrowserApplication.timeDisplay === BrowserApplication.LOCALTIME
                onTriggered: {
                    BrowserApplication.timeDisplay = BrowserApplication.LOCALTIME
                }
            }
            Labs.MenuItem {
                text: KI18n.i18nc("@item:inmenu", "UTC Realtime")
                checkable: true
                checked: BrowserApplication.timeDisplay === BrowserApplication.UTC
                onTriggered: {
                    BrowserApplication.timeDisplay = BrowserApplication.UTC
                }
            }
            Labs.MenuItem {
                text: KI18n.i18nc("@item:inmenu", "Monotonic Time")
                checkable: true
                checked: BrowserApplication.timeDisplay === BrowserApplication.MONOTONIC_TIMESTAMP
                onTriggered: {
                    BrowserApplication.timeDisplay = BrowserApplication.MONOTONIC_TIMESTAMP
                }
            }
        }
        Labs.Menu {
            title: KI18n.i18nc("@title:menu", "Colorize")
            icon.name: "fill-color"

            Labs.MenuItem {
                text: KI18n.i18nc("@item:inmenu", "Systemd Unit")
                checkable: true
                checked: BrowserApplication.filterCriterium === BrowserApplication.SYSTEMD_UNIT
                onTriggered: {
                    BrowserApplication.filterCriterium = BrowserApplication.SYSTEMD_UNIT
                }
            }
            Labs.MenuItem {
                text: KI18n.i18nc("@item:inmenu", "Executable")
                checkable: true
                checked: BrowserApplication.filterCriterium === BrowserApplication.EXECUTABLE
                onTriggered: {
                    BrowserApplication.filterCriterium = BrowserApplication.EXECUTABLE
                }
            }
        }
        Labs.Menu {
            title: KI18n.i18nc("@title:menu", "Grouping")
            icon.name: "view-group"

            Labs.MenuItem {
                text: KI18n.i18nc("@item:inmenu", "Group templated services")
                checkable: true
                checked: BrowserApplication.serviceGrouping === BrowserApplication.GROUP_SERVICE_TEMPLATES
                onTriggered: {
                    BrowserApplication.serviceGrouping = BrowserApplication.GROUP_SERVICE_TEMPLATES
                }
            }
            Labs.MenuItem {
                text: KI18n.i18nc("@item:inmenu", "Do not group templated service")
                checkable: true
                checked: BrowserApplication.serviceGrouping === BrowserApplication.UNGROUP_SERVICE_TEMPLATES
                onTriggered: {
                    BrowserApplication.serviceGrouping = BrowserApplication.UNGROUP_SERVICE_TEMPLATES
                }
            }
        }
    }
    Labs.Menu {
        title: KI18n.i18nc("@title:menu", "Help")

        StatefulAppLabs.NativeMenuItem {
            actionName: "open_about_page"
            application: root.application
        }

        StatefulAppLabs.NativeMenuItem {
            actionName: "open_about_kde_page"
            application: root.application
        }
    }
}

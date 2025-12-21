// SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
// SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>

import Qt.labs.platform as Labs
import org.kde.kirigamiaddons.statefulapp.labs as StatefulAppLabs
import org.kde.kirigamiaddons.statefulapp as StatefulApp
import org.kde.ki18n
import org.kde.kjournaldbrowser

Labs.MenuBar {
    id: root

    property StatefulApp.AbstractKirigamiApplication application: BrowserApplication

    Labs.Menu {
        title: KI18n.i18nc("@title:menu", "File")

        Labs.MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Open system journal")
            icon.name: "document-open"
            onTriggered: {
                DatabaseProvider.setSystemJournal()
            }
        }
        Labs.MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Open user journal")
            icon.name: "document-open"
            onTriggered: {
                DatabaseProvider.setUserJournal()
            }
        }
        Labs.MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Open from folder")
            icon.name: "document-open"
            onTriggered: {
                folderDialog.folder = DatabaseProvider.localJournalPath
                folderDialog.open()
            }
        }
        Labs.MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Open from file")
            icon.name: "document-open"
            onTriggered: {
                fileDialog.folder = DatabaseProvider.localJournalPath
                fileDialog.open()
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
            onTriggered: copyViewToClipboard()
        }
    }
    Labs.Menu {
        title: KI18n.i18nc("@title:menu", "View")

        Labs.Menu {
            title: KI18n.i18nc("@title:menu", "Timestamp Display")

            Labs.MenuItem {
                text: KI18n.i18n("@item:inmenu", "Localized Realtime")
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
            title: KI18n.i18n("@title:menu", "Colorize")

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

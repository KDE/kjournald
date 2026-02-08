// SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
// SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import org.kde.ki18n
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.statefulapp as StatefulApp
import org.kde.kjournaldbrowser

MenuBar {
    id: root

    property StatefulApp.AbstractKirigamiApplication application: BrowserApplication
    required property FileDialog fileDialog
    required property FolderDialog folderDialog

    signal copyViewToClipboard()

    Kirigami.Theme.colorSet: Kirigami.Theme.Header
    Menu {
        title: KI18n.i18nc("@title:menu", "File")
        MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Local Journal")
            icon.name: "document-open"
            onTriggered: {
                DatabaseProvider.setLocalJournal()
            }
        }
        MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Open from Folder")
            icon.name: "document-open"
            onTriggered: {
                root.folderDialog.currentFolder = DatabaseProvider.localJournalPath
                root.folderDialog.open()
            }
        }
        MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Open from File")
            icon.name: "document-open"
            onTriggered: {
                root.fileDialog.currentFolder = DatabaseProvider.localJournalPath
                root.fileDialog.open()
            }
        }
// disable feature for end-users until the dialog experience is more polished
//        MenuItem {
//            text: KI18n.i18nc("@action:inmenu": "Open Remote Journal Stream")
//            icon.name: "document-import"
//            onTriggered: {
//                remoteJournalDialog.open()
//            }
//        }

        MenuSeparator { }

        MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Close")
            icon.name: "application-exit"
            onTriggered: Qt.quit()
        }
    }
    Menu {
        title: KI18n.i18nc("@title:menu", "Current Journal")
        MenuItem {
            text: KI18n.i18nc("@action:inmenu", "Copy Current View")
            icon.name: "edit-copy"
            onTriggered: root.copyViewToClipboard()
        }
        Menu {
            title: KI18n.i18nc("@title:menu", "Limit Accessed Logs")
            icon.name: "view-filter"
            MenuItem {
                contentItem: RadioButton {
                    text: KI18n.i18nc("@item:inmenu", "All logs")
                    checkable: true
                    checked: BrowserApplication.logViewMode === BrowserApplication.ALL_LOGS
                    onToggled: {
                        BrowserApplication.logViewMode = BrowserApplication.ALL_LOGS
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: KI18n.i18nc("@item:inmenu", "Only current user logs")
                    checkable: true
                    checked: BrowserApplication.logViewMode === BrowserApplication.ONLY_USER
                    onToggled: {
                        BrowserApplication.logViewMode = BrowserApplication.ONLY_USER
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: KI18n.i18nc("@item:inmenu", "Only system logs")
                    checkable: true
                    checked: BrowserApplication.logViewMode === BrowserApplication.ONLY_SYSTEM
                    onToggled: {
                        BrowserApplication.logViewMode = BrowserApplication.ONLY_SYSTEM
                    }
                }
            }
        }
    }
    Menu {
        title: KI18n.i18nc("@title:menu", "View")

        Menu {
            title: KI18n.i18nc("@title:menu", "Timestamp Display")

            MenuItem {
                contentItem: RadioButton {
                    text: KI18n.i18nc("@item:inmenu", "Localized Realtime")
                    checkable: true
                    checked: BrowserApplication.timeDisplay === BrowserApplication.LOCALTIME
                    onToggled: {
                        BrowserApplication.timeDisplay = BrowserApplication.LOCALTIME
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: KI18n.i18nc("@item:inmenu", "UTC Realtime")
                    checkable: true
                    checked: BrowserApplication.timeDisplay === BrowserApplication.UTC
                    onToggled: {
                        BrowserApplication.timeDisplay = BrowserApplication.UTC
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: KI18n.i18nc("@item:inmenu", "Monotonic Time")
                    checkable: true
                    checked: BrowserApplication.timeDisplay === BrowserApplication.MONOTONIC_TIMESTAMP
                    onToggled: {
                        BrowserApplication.timeDisplay = BrowserApplication.MONOTONIC_TIMESTAMP
                    }
                }
            }
        }
        Menu {
            title: KI18n.i18nc("@title:menu", "Colorize")

            MenuItem {
                contentItem: RadioButton {
                    text: KI18n.i18nc("@item:inmenu", "Systemd Unit")
                    checkable: true
                    checked: BrowserApplication.filterCriterium === BrowserApplication.SYSTEMD_UNIT
                    onToggled: {
                        BrowserApplication.filterCriterium = BrowserApplication.SYSTEMD_UNIT
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: KI18n.i18nc("@item:inmenu", "Executable")
                    checkable: true
                    checked: BrowserApplication.filterCriterium === BrowserApplication.EXECUTABLE
                    onToggled: {
                        BrowserApplication.filterCriterium = BrowserApplication.EXECUTABLE
                    }
                }
            }
        }
        Menu {
            title: KI18n.i18nc("@title:menu", "Grouping")

            MenuItem {
                contentItem: RadioButton {
                    text: KI18n.i18nc("@item:inmenu", "Group templated services")
                    checkable: true
                    checked: BrowserApplication.serviceGrouping === BrowserApplication.GROUP_SERVICE_TEMPLATES
                    onToggled: {
                        BrowserApplication.serviceGrouping = BrowserApplication.GROUP_SERVICE_TEMPLATES
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: KI18n.i18nc("@item:inmenu", "Do not group templated service")
                    checkable: true
                    checked: BrowserApplication.serviceGrouping === BrowserApplication.UNGROUP_SERVICE_TEMPLATES
                    onToggled: {
                        BrowserApplication.serviceGrouping = BrowserApplication.UNGROUP_SERVICE_TEMPLATES
                    }
                }
            }
        }
    }
    Menu {
        title: KI18n.i18nc("@title:menu", "Help")

        Kirigami.Action {
            fromQAction: root.application.action('open_about_page')
        }

        Kirigami.Action {
            fromQAction: root.application.action('open_about_kde_page')
        }
    }
}

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.statefulapp as StatefulApp
import org.kde.kjournaldbrowser

MenuBar {
    id: root

    property StatefulApp.AbstractKirigamiApplication application: BrowserApplication

    signal copyViewToClipboard()
    Kirigami.Theme.colorSet: Kirigami.Theme.Header
    Menu {
        title: i18nc("@action:menu", "File")
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
                    checked: BrowserApplication.timeDisplay === BrowserApplication.LOCALTIME
                    onToggled: {
                        BrowserApplication.timeDisplay = BrowserApplication.LOCALTIME
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: i18n("UTC Realtime")
                    checkable: true
                    checked: BrowserApplication.timeDisplay === BrowserApplication.UTC
                    onToggled: {
                        BrowserApplication.timeDisplay = BrowserApplication.UTC
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: i18n("Monotonic Time")
                    checkable: true
                    checked: BrowserApplication.timeDisplay === BrowserApplication.MONOTONIC_TIMESTAMP
                    onToggled: {
                        BrowserApplication.timeDisplay = BrowserApplication.MONOTONIC_TIMESTAMP
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
                    checked: BrowserApplication.filterCriterium === BrowserApplication.SYSTEMD_UNIT
                    onToggled: {
                        BrowserApplication.filterCriterium = BrowserApplication.SYSTEMD_UNIT
                    }
                }
            }
            MenuItem {
                contentItem: RadioButton {
                    text: i18n("Executable")
                    checkable: true
                    checked: BrowserApplication.filterCriterium === BrowserApplication.EXECUTABLE
                    onToggled: {
                        BrowserApplication.filterCriterium = BrowserApplication.EXECUTABLE
                    }
                }
            }
        }
    }
    Menu {
        title: i18nc("@action:menu", "Help")

        Kirigami.Action {
            fromQAction: root.application.action('open_about_page')
        }

        Kirigami.Action {
            fromQAction: root.application.action('open_about_kde_page')
        }
    }
}

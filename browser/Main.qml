/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.0
import QtQuick.Layouts 1.15
import systemd 1.0

// only for initial testing of tree model
import QtQuick.Controls 1.4 as QQC1

ApplicationWindow {
    width: 800
    height: 640
    visible: true

    menuBar: TopMenuBar {
        onCopyViewToClipboard: logView.copyTextFromView()
    }

    header: ToolBar {
        RowLayout {
            anchors {
                fill: parent
                leftMargin: 10
            }
            // forward page key events to listview
            Keys.forwardTo: [logView]
            focus: true

            Label {
                text: "Boot:"
            }
            ComboBox {
                id: bootIdComboBox
                implicitWidth: Math.max(300, implicitContentWidth)
                model: g_bootModel
                textRole: g_config.timeDisplay
                          === SessionConfig.UTC ? "displayshort_utc" : "displayshort_localtime"
                valueRole: "bootid"
            }
            ToolSeparator {}

            Label {
                text: "Highlight:"
            }
            TextField {
                id: hightlightTextField
                text: ""
            }

            ToolSeparator {}
            ToolButton {
                text: "Browse"
                icon.name: "transform-browse"
                checkable: true
                checked: g_config.viewMode === SessionConfig.BROWSE
                onClicked: {
                    g_config.viewMode = SessionConfig.BROWSE
                }
            }
            ToolButton {
                text: "Select"
                icon.name: "edit-select-text"
                checkable: true
                checked: g_config.viewMode === SessionConfig.SELECT
                onClicked: {
                    g_config.viewMode = SessionConfig.SELECT
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    FileDialog {
        id: folderDialog
        title: "Select journal folder"
        selectFolder: true
        onAccepted: {
            g_config.localJournalPath = folderDialog.fileUrl
            g_config.sessionMode = SessionConfig.LOCALFOLDER
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select journal file"
        nameFilters: ["Journal files (*.journal)", "All files (*)"]
        onAccepted: {
            g_config.localJournalPath = fileDialog.fileUrl
            g_config.sessionMode = SessionConfig.LOCALFOLDER
        }
    }

    RemoteJournalConfigDialog {
        id: remoteJournalDialog
        onAccepted: {
            console.log("set remote journal to: " + url + ":" + port)
            g_config.remoteJournalPort = remoteJournalDialog.port
            g_config.remoteJournalUrl = remoteJournalDialog.url
            g_config.sessionMode = SessionConfig.REMOTE
        }
    }

    FlattenedFilterCriteriaProxyModel {
        id: flatFilterSelection
        sourceModel: g_filterModel
    }

    SplitView {
        anchors.fill: parent

        // forward page key events to listview
        Keys.forwardTo: [logView]
        focus: true

        ScrollView {
            height: parent.height
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            SplitView.preferredWidth: 300

            FilterCriteriaView {
                width: parent.width
            }
        }
        Rectangle {
            color: "#ffffff"
            height: parent.height
            SplitView.fillWidth: true
            LogView {
                id: logView
                anchors.fill: parent
                journalModel: g_journalModel
                displayRoleRight: g_config.filterCriterium === SessionConfig.SYSTEMD_UNIT ?
                                      JournaldViewModel.SYSTEMD_UNIT : JournaldViewModel.EXE
                snapToFollowMode: true
                textSelectionMode: g_config.viewMode === SessionConfig.SELECT
                visible: count > 0
                onTextCopied: {
                    clipboard.setText(text)
                    console.log("view content copied")
                }
            }
            Text {
                anchors.centerIn: parent
                text: "No log entries apply to current filter selection"
                visible: !(logView.count > 0
                           && (g_unitModel.selectedEntries.length > 0
                               || g_executableModel.selectedEntries.length > 0
                               || root.journalModel.kernelFilter))
            }
        }
    }

    JournaldViewModel {
        id: g_journalModel
        journalPath: g_config.sessionMode === SessionConfig.LOCALFOLDER
                     || g_config.sessionMode
                     === SessionConfig.REMOTE ? g_config.localJournalPath : undefined
        systemdUnitFilter: g_filterModel.systemdUnitFilter
        exeFilter: g_filterModel.exeFilter
        bootFilter: bootIdComboBox.currentValue
        priorityFilter: g_filterModel.priorityFilter
        kernelFilter: g_filterModel.kernelFilter
    }
    ClipboardProxy {
        id: clipboard
    }
}

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.0
import systemd 1.0

ApplicationWindow {
    width: 800
    height: 640
    visible: true

    menuBar: TopMenuBar {}

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
        nameFilters: [ "Journal files (*.journal)", "All files (*)" ]
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

    Rectangle {
        id: topMenu
        height: 42
        width: parent.width
        z: 1 // put on top of list view
        color: "#cccccc"

        // forward page key events to listview
        Keys.forwardTo: [ logView ]
        focus: true

        Row {
            spacing: 20
            anchors {
                fill: parent
                leftMargin: 5
            }
            Label {
                anchors {
                    verticalCenter: parent.verticalCenter
                }
                text: "Boot ID:"
                font.pixelSize: 16
            }
            ComboBox {
                id: bootIdComboBox
                width: 300
                model: g_bootModel
                textRole: "displayshort"
                valueRole: "bootid"
            }
            Label {
                anchors {
                    verticalCenter: parent.verticalCenter
                }
                text: "Priority:"
                font.pixelSize: 16
            }
            ComboBox {
                id: priorityComboBox
                property int priority: 6
                ListModel {
                    id: priorityModel
                    ListElement{
                        text: "emergency"
                        value: 0
                    }
                    ListElement{
                        text: "alert"
                        value: 1
                    }
                    ListElement{
                        text: "critical"
                        value: 2
                    }
                    ListElement{
                        text: "error"
                        value: 3
                    }
                    ListElement{
                        text: "warning"
                        value: 4
                    }
                    ListElement{
                        text: "notice"
                        value: 5
                    }
                    ListElement{
                        text: "info"
                        value: 6
                    }
                    ListElement{
                        text: "debug"
                        value: 7
                    }
                }

                currentIndex: 5
                model: priorityModel
                textRole: "text"
                onCurrentIndexChanged: {
                    priorityComboBox.priority = priorityModel.get(currentIndex).value
                }
            }
            Label {
                anchors {
                    verticalCenter: parent.verticalCenter
                }
                text: "Highlight:"
                font.pixelSize: 16
            }
            TextField {
                id: hightlightTextField
                text: ""
            }
        }
    }

    Row {
        anchors {
            top: topMenu.bottom
        }
        width: parent.width
        height: parent.height - topMenu.height

        // forward page key events to listview
        Keys.forwardTo: [ logView ]
        focus: true

        Column {
            id: unitColumn
            height: parent.height
            width: Math.min(parent.width * 0.3, 300)
            Rectangle {
                height: kernelFilterCheckbox.height + 1
                width: parent.width
                CheckBox {
                    id: kernelFilterCheckbox
                    text: "Kernel Log"
                    checked: g_journalModel.kernelFilter
                    onCheckedChanged: g_journalModel.kernelFilter = checked
                }
                Rectangle {
                    width: parent.width
                    height: 1
                    anchors.bottom: parent.bottom
                    color: "#cccccc"
                }
            }

            ListView {
                z: -1
                height: parent.height - checkboxControls.height - kernelFilterCheckbox.height
                width: parent.width
                model: g_config.filterCriterium === SessionConfig.SYSTEMD_UNIT ? g_unitSortProxyModel : g_executableSortProxyModel
                delegate: CheckBox {
                    checked: model.selected
                    text: model.field
                    width: unitColumn.width
                    hoverEnabled: true
                    ToolTip.delay: 1000
                    ToolTip.visible: hovered
                    ToolTip.text: model.field
                    onCheckStateChanged: model.selected = checkState
                }
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    active: ScrollBar.AlwaysOn
                }
            }
            Rectangle {
                color: "#ffffff"
                width: parent.width
                height: checkboxControls.height
                Row {
                    id: checkboxControls
                    Button {
                        id: selectNoneButton
                        text: "Check/uncheck all"
                        icon.name: "checkbox"
                        property bool selectAll: true
                        onClicked: {
                            selectAll = !selectAll
                            if (g_config.filterCriterium === SessionConfig.SYSTEMD_UNIT) {
                                g_unitModel.setAllSelectionStates(selectAll)
                            }
                            else if (g_config.filterCriterium === SessionConfig.EXECUTABLE) {
                                g_executableModel.setAllSelectionStates(selectAll)
                            }
                            else {
                                console.warn("unhandled check-all type")
                            }
                        }
                    }
                }
            }
        }
        Rectangle {
            color: "#ffffff"
            height: parent.height
            width: parent.width - unitColumn.width
            LogView {
                id: logView
                anchors.fill: parent
                journalModel: g_journalModel
                snapToFollowMode: true
                visible: count > 0
                onTextCopied: {
                    clipboard.setText(text)
                    console.log("view content copied")
                }
            }
            Text {
                anchors.centerIn: parent
                text: "No log entries apply to current filter selection"
                visible: !(logView.count > 0 && (g_unitModel.selectedEntries.length > 0 || g_executableModel.selectedEntries.length > 0 || root.journalModel.kernelFilter))
            }
        }
    }

    JournaldViewModel {
        id: g_journalModel
        journalPath: g_config.sessionMode === SessionConfig.LOCALFOLDER || g_config.sessionMode === SessionConfig.REMOTE ? g_config.localJournalPath : undefined
        systemdUnitFilter: g_config.filterCriterium === SessionConfig.SYSTEMD_UNIT ? g_unitModel.selectedEntries : []
        exeFilter: g_config.filterCriterium === SessionConfig.EXECUTABLE ? g_executableModel.selectedEntries : []
        bootFilter: bootIdComboBox.currentValue
        priorityFilter: priorityComboBox.priority
    }
    ClipboardProxy {
        id: clipboard
    }
}

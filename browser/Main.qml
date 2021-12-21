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

    Row {
        anchors.fill: parent

        // forward page key events to listview
        Keys.forwardTo: [logView]
        focus: true

        Column {
            id: unitColumn
            height: parent.height
            width: Math.min(parent.width * 0.3, 300)

            FlattenedFilterCriteriaProxyModel {
                id: flatFilterSelection
                sourceModel: g_filterModel
            }

            ScrollView {
                height: parent.height
                width: parent.width
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

                Column {
                    ButtonGroup{ id: priorityGroup }
                    Repeater {
                        id: rootElements
                        model: flatFilterSelection

                        Component {
                            id: firstLevelComponent
                            ItemDelegate {
                                height: 20 + children.height
                                property var model
                                text:  model.text
                                onClicked: model.expanded = !model.expanded
                            }
                        }
                        Component {
                            id: secondLevelCheckboxComponent
                            CheckBox {
                                property var model
                                height: 20 + children.height
                                text: model ? model.text : ""
                                leftPadding: 20
                                onCheckedChanged: model.selected = checked
                                onModelChanged: if (model) { checked = model.selected }

                                ToolTip.delay: 1000
                                ToolTip.timeout: 5000
                                ToolTip.visible: hovered
                                ToolTip.text: model !== null ? model.longtext : ""
                            }
                        }
                        Component {
                            id: secondLevelCheckboxColorCodeComponent
                            CheckBox {
                                id: control
                                property var model
                                height: 20 + children.height
                                text: model ? model.text : ""
                                leftPadding: 20
                                onCheckedChanged: model.selected = checked
                                onModelChanged: if (model) { checked = model.selected }

                                ToolTip.delay: 1000
                                ToolTip.timeout: 5000
                                ToolTip.visible: hovered
                                ToolTip.text: model !== null ? model.longtext : ""

                                contentItem: Row {
                                    leftPadding: control.indicator && !control.mirrored ? control.indicator.width + control.spacing : 0
                                    rightPadding: control.indicator && control.mirrored ? control.indicator.width + control.spacing : 0
                                    spacing: 6

                                    Rectangle {
                                        width: 24
                                        height: 24
                                        radius: 4
                                        color: model ? model.color : "#FF0000"
                                    }

                                    Text {
                                        text: control.text
                                        font: control.font
                                        color: control.palette.windowText
                                    }
                                }
                            }
                        }
                        Component {
                            id: secondLevelRadiobuttonComponent
                            RadioButton {
                                property var model
                                height: 20 + children.height
                                text: model ? model.text : ""
                                leftPadding: 20
                                onCheckedChanged: model.selected = checked
                                onModelChanged: if (model) { checked = model.selected }
                                ButtonGroup.group: priorityGroup

                                ToolTip.delay: 1000
                                ToolTip.timeout: 5000
                                ToolTip.visible: hovered
                                ToolTip.text: model !== null ? model.longtext : ""
                            }
                        }
                        Loader {
                            sourceComponent: {
                                if (model.indentation === 0) {
                                    return firstLevelComponent;
                                }
                                if (model.type === FlattenedFilterCriteriaProxyModel.CHECKBOX) {
                                    return secondLevelCheckboxComponent
                                }
                                if (model.type === FlattenedFilterCriteriaProxyModel.CHECKBOX_COLORED) {
                                    return secondLevelCheckboxColorCodeComponent
                                }
                                if (model.type === FlattenedFilterCriteriaProxyModel.RADIOBUTTON) {
                                    return secondLevelRadiobuttonComponent
                                }
                                console.warn("fallback due to unknown type: " + model.type)
                                return firstLevelComponent;
                            }
                            onLoaded: item.model = model // pass model reference to item
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

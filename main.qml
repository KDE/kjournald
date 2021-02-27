/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.0
import systemd 1.0

ApplicationWindow {
    width: 800
    height: 640
    visible: true

    menuBar: MenuBar {
        Menu {
            title: "File"
            MenuItem {
                text: "Open Folder"
                onClicked: {
                    fileDialog.folder = journalModel.journalPath
                    fileDialog.open()
                }
            }
            MenuItem {
                text: "Close"
                onClicked: Qt.quit()
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select journal folder"
        selectFolder: true
        onAccepted: {
            journalModel.journalPath = fileDialog.fileUrl
        }
    }

    Rectangle {
        id: topMenu
        height: 42
        width: parent.width
        z: 1 // put on top of list view
        color: "#cccccc"
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
                property var bootId: [ ];
                model: g_bootModel
                textRole: "displayshort"
    //            valueRole: "_BOOT_ID" // elegant solution but not doable with Qt 5.12
                onCurrentIndexChanged: {
                    bootId = [ g_bootModel.bootId(currentIndex) ]
    //                bootId = [ currentValue ]
                }
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
                property int priority: 5
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
        }
    }

    Row {
        anchors {
            top: topMenu.bottom
        }
        width: parent.width
        height: parent.height - topMenu.height

        Column {
            id: unitColumn
            height: parent.height
            width: Math.min(parent.width * 0.3, 300)
            Row {
                Button {
                    id: selectNoneButton
                    text: "Select None"
                    onClicked: {
                        unitModel.setAllSelectionStates(false)
                    }
                }
                Button {
                    id: selectAllButton
                    text: "Select All"
                    onClicked: {
                        unitModel.setAllSelectionStates(true)
                    }
                }
            }
            ListView {
                z: -1
                height: parent.height - selectNoneButton.height
                width: parent.width
                model: unitModel
                delegate: CheckBox {
                    checked: model.selected
                    text: model.field
                    onCheckStateChanged: model.selected = checkState
                }
            }
        }
        ListView {
            height: parent.height
            width: parent.width - unitColumn.width
            model: journalModel
            delegate: Rectangle
            {
                color: model.unitcolor
                width: parent.width
                height: messageText.height
                Text {
                    id: messageText
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: Qt.formatTime(model.date, "HH:mm:ss.zzz") + " " + model.message
                    color: {
                        switch(model.priority) {
                        case 0: return "#700293" // emergency (violet)
                        case 1: return "#930269" // alert
                        case 2: return "#930202" // critical
                        case 3: return "#ff0000" // error (red)
                        case 4: return "#cc9c00" // warning (orange)
                        case 5: return "#015eff" // notice (blue)
                        case 6: return "#029346" // information (green)
                        case 7: return "#000000" // debug
                        }
                    }
                    Rectangle {
                        anchors.right: parent.right
                        width: unitInfo.width + 8
                        height: unitInfo.height
                        color: "#ffffff"
                        Text {
                            id: unitInfo
                            anchors {
                                right: parent.right
                                rightMargin: 4
                            }
                            text: model.systemdunit
                        }
                    }
                }
            }
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOn
                active: ScrollBar.AlwaysOn
            }
        }
    }

    JournaldUniqueQueryModel {
        id: unitModel
        journalPath: g_path
        field: "_SYSTEMD_UNIT"
    }
    JournaldViewModel {
        id: journalModel
        journalPath: g_path
        systemdUnitFilter: unitModel.selectedEntries
        bootFilter: bootIdComboBox.bootId
        priorityFilter: priorityComboBox.priority
    }
}

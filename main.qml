/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import systemd 1.0

Window {
    width: 800
    height: 640
    visible: true

    Row {
        id: topMenu
        height: 42
        width: parent.width
        Label {
            anchors {
                verticalCenter: parent.verticalCenter
            }
            text: "Boot ID: "
            font.pixelSize: 16
        }
        ComboBox {
            id: bootIdComboBox
            width: 300
            property var bootId: [ ];
            model: g_bootModel
            textRole: "displayshort"
            valueRole: "_BOOT_ID"
            onActivated: {
                bootId = [ currentValue ]
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
            Button {
                id: selectNoneButton
                text: "Select None"
                onClicked: {
                    unitModel.setAllSelectionStates(false)
                }
            }
            ListView {
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
                color: model.index % 2 === 0 ? "#efefef" : "#ffffff"
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
                        case 0: return "#ff0000" // emergency
                        case 1: return "#cc0000" // alert
                        case 2: return "#cc5200" // critical
                        case 3: return "#00ff00" // error
                        case 4: return "#cc9c00" // warning
                        case 5: return "#015eff" // notice
                        case 6: return "#015e39" // information
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
        }
    }

    JournaldUniqueQueryModel {
        id: unitModel
        journalPath: "/opt/workspace/journald-browser/TESTDATA/journal/"
        field: "_SYSTEMD_UNIT"
    }

    JournaldUniqueQueryModel {
        id: bootModel
        journalPath: "/opt/workspace/journald-browser/TESTDATA/journal/"
        field: "_BOOT_ID"
    }

    JournaldViewModel {
        id: journalModel
        // file journal currently broken
        journalPath: "/opt/workspace/journald-browser/TESTDATA/journal/"
        systemdUnitFilter: unitModel.selectedEntries
        bootFilter: bootIdComboBox.bootId
    }
}


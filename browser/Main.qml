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

    function copyViewToClipbaord() {
        var startIndex = viewRoot.indexAt(1, viewRoot.contentY)
        var endIndex = viewRoot.indexAt(1, viewRoot.contentY + viewRoot.height);
        if (endIndex < 0) {
            endIndex = g_journalModel.rowCount()
        }
        var content = ""
        for (var i = startIndex; i < endIndex; ++i) {
            content += g_journalModel.formatTime(g_journalModel.data(g_journalModel.index(i, 0), JournaldViewModel.DATE), true) + " UTC "
                        + g_journalModel.data(g_journalModel.index(i, 0), JournaldViewModel.SYSTEMD_UNIT) + " "
                        + g_journalModel.data(g_journalModel.index(i, 0), JournaldViewModel.MESSAGE) + "\n"
        }
        clipboard.setText(content)
        console.log("view content copied")
    }

    menuBar: MenuBar {
        Menu {
            title: "File"
            MenuItem {
                text: "Copy Log Output"
                onClicked: copyViewToClipbaord()
            }
            MenuItem {
                text: "Close"
                onClicked: Qt.quit()
            }
        }
        Menu {
            title: "Journal"
            MenuItem {
                text: "Use default journal"
                onClicked: {
                    g_config.sessionMode = SessionConfig.SYSTEM
                }
            }
            MenuItem {
                text: "Open from folder"
                onClicked: {
                    fileDialog.folder = g_config.localJournalPath
                    fileDialog.open()
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select journal folder"
        selectFolder: true
        onAccepted: {
            g_config.localJournalPath = fileDialog.fileUrl
            g_config.sessionMode = SessionConfig.LOCALFOLDER
        }
    }

    Rectangle {
        id: topMenu
        height: 42
        width: parent.width
        z: 1 // put on top of list view
        color: "#cccccc"

        // forward page key events to listview
        Keys.forwardTo: [ viewRoot ]
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
                text: "Kernel Log:"
                font.pixelSize: 16
            }
            CheckBox {
                checked: g_journalModel.kernelFilter
                onCheckedChanged: g_journalModel.kernelFilter = checked
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
        Keys.forwardTo: [ viewRoot ]
        focus: true

        Column {
            id: unitColumn
            height: parent.height
            width: Math.min(parent.width * 0.3, 300)
            Row {
                Button {
                    id: selectNoneButton
                    text: "Select None"
                    onClicked: {
                        g_unitModel.setAllSelectionStates(false)
                    }
                }
                Button {
                    id: selectAllButton
                    text: "Select All"
                    onClicked: {
                        g_unitModel.setAllSelectionStates(true)
                    }
                }
            }
            ListView {
                z: -1
                height: parent.height - selectNoneButton.height
                width: parent.width
                model: g_unitModel
                delegate: CheckBox {
                    checked: model.selected
                    text: model.field
                    onCheckStateChanged: model.selected = checkState
                }
            }
        }
        ListView {
            id: viewRoot
            highlightMoveDuration: 10
            height: parent.height
            width: parent.width - unitColumn.width
            model: g_journalModel
            focus: true
            Component.onCompleted: forceActiveFocus()
            delegate: Rectangle
            {
                color: model.unitcolor
                width: viewRoot.width
                height: messageText.height
                LogLine {
                    id: messageText
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    index: model.index
                    date: model.date
                    priority: model.priority
                    message: model.message
                    highlight: hightlightTextField.text
                    modelProxy: viewRoot.model

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
            TextMetrics {
                id: messageIdMetrics
                text: "Example Log message"
            }
            Keys.onPressed: {
                var currentIndex = viewRoot.indexAt(1, viewRoot.contentY + 1) // 1 pixel right and down to really get the first item in view
                var scrollIndexSkip = Math.floor( 0.9 * viewRoot.height / 30) // hard-coded estimate of one line log height
                if (event.key === Qt.Key_PageDown) {
                    if (event.modifiers & Qt.ControlModifier) {
                        // model provides just a sliding window over the journal, fetch tail data first
                        g_journalModel.seekTail()
                        viewRoot.positionViewAtEnd()
                    }
                    else {
                        if (viewRoot.contentHeight - viewRoot.originY > viewRoot.height) {
                            if (viewRoot.contentY - viewRoot.originY + 2 * viewRoot.height >= viewRoot.contentHeight) {
                                // enforce fetching here such that it does not happen implictly during calculation the new contentY
                                g_journalModel.fetchMore(g_journalModel.index(0, 0))
                            }
                            positionViewAtIndex(currentIndex + scrollIndexSkip, ListView.Beginning)
                        } else {
                            viewRoot.contentY = viewRoot.originY
                        }
                    }
                }
                if (event.key === Qt.Key_PageUp) {
                    if (event.modifiers & Qt.ControlModifier) {
                        // model provides just a sliding window over the journal, fetch head data first
                        g_journalModel.seekHead()
                        viewRoot.positionViewAtBeginning()
                    }
                    else {
                        if (viewRoot.contentHeight - viewRoot.originY > viewRoot.height) {
                            if (viewRoot.contentY - viewRoot.originY <= 3 * viewRoot.height) {
                                // enforce fetching here such that it does not happen implictly during calculation the new contentY
                                g_journalModel.fetchMore(g_journalModel.index(0, 0))

                                // update currentIndex, because it has changed when rows added at top
                                currentIndex = viewRoot.indexAt(1, viewRoot.contentY + 1)
                            }
                            positionViewAtIndex(currentIndex - scrollIndexSkip, ListView.Beginning)
                        } else {
                            viewRoot.contentY = viewRoot.originY
                        }
                    }
                }
                if (event.key === Qt.Key_F3) {
                    var index = g_journalModel.search(hightlightTextField.text, viewRoot.currentIndex + 1)
                    if (index >= 0) {
                        viewRoot.currentIndex = index
                        positionViewAtIndex(index, ListView.Beginning)
                    }
                }
                if (event.key === Qt.Key_C && (event.modifiers & Qt.ControlModifier)) {
                    copyViewToClipbaord()
                }
            }
        }
    }

    JournaldViewModel {
        id: g_journalModel
        journalPath: g_config.sessionMode === SessionConfig.LOCALFOLDER ? g_config.localJournalPath : undefined
        systemdUnitFilter: g_unitModel.selectedEntries
        bootFilter: bootIdComboBox.bootId
        priorityFilter: priorityComboBox.priority
    }
    ClipboardProxy {
        id: clipboard
    }
}

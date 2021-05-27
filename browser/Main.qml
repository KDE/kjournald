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
                text: "Open system journal"
                icon.name: "document-open"
                onTriggered: {
                    g_config.sessionMode = SessionConfig.SYSTEM
                }
            }
            MenuItem {
                text: "Open from folder"
                icon.name: "document-open"
                onTriggered: {
                    folderDialog.folder = g_config.localJournalPath
                    folderDialog.open()
                }
            }
            MenuItem {
                text: "Open from file"
                icon.name: "document-open"
                onTriggered: {
                    fileDialog.folder = g_config.localJournalPath
                    fileDialog.open()
                }
            }
            MenuItem {
                text: "Open remote journal stream"
                icon.name: "document-import"
                onTriggered: {
                    remoteJournalDialog.open()
                }
            }

            MenuSeparator { }

            MenuItem {
                text: "Close"
                icon.name: "application-exit"
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: "Current Journal"
            MenuItem {
                text: "Copy current view"
                icon.name: "edit-copy"
                onTriggered: copyViewToClipbaord()
            }
        }
        Menu {
            title: "View"

            Menu {
                title: "Timestamp Display"

                MenuItem {
                    text: "Localized Realtime"
                    checkable: true
                    checked: g_config.timeDisplay === SessionConfig.LOCALTIME
                    onTriggered: {
                        g_config.timeDisplay = SessionConfig.LOCALTIME
                    }
                }
                MenuItem {
                    text: "UTC Realtime"
                    checkable: true
                    checked: g_config.timeDisplay === SessionConfig.UTC
                    onTriggered: {
                        g_config.timeDisplay = SessionConfig.UTC
                    }
                }
                MenuItem {
                    text: "Monotonic Time"
                    checkable: true
                    checked: g_config.timeDisplay === SessionConfig.MONOTONIC_TIMESTAMP
                    onTriggered: {
                        g_config.timeDisplay = SessionConfig.MONOTONIC_TIMESTAMP
                    }
                }
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
                model: g_unitSortProxyModel
                delegate: CheckBox {
                    checked: model.selected
                    text: model.field
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
                            g_unitModel.setAllSelectionStates(selectAll)
                        }
                    }
                }
            }
        }
        Rectangle {
            color: "#ffffff"
            height: parent.height
            width: parent.width - unitColumn.width
            ListView {
                id: viewRoot

                readonly property bool logEntriesAvailable: viewRoot.count > 0 && (g_unitModel.selectedEntries.length > 0 || g_journalModel.kernelFilter)

                visible: viewRoot.logEntriesAvailable
                highlightMoveDuration: 10
                anchors.fill: parent
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
                        monotonicTimestamp: model.monotonictimestamp
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
                    var scrollIndexSkip = Math.floor(0.9 * viewRoot.height / 30) // hard-coded estimate of one line log height
                    if (event.key === Qt.Key_PageDown) {
                        if (event.modifiers & Qt.ControlModifier) {
                            // model provides just a sliding window over the journal, fetch tail data first
                            g_journalModel.seekTail()
                            viewRoot.positionViewAtEnd()
                        }
                        else {
                            if (viewRoot.contentHeight - viewRoot.originY > viewRoot.height) {
                                if (viewRoot.contentY - viewRoot.originY + 2 * viewRoot.height >= viewRoot.contentHeight) {
                                    // enforce fetching here such that it does not happen implicitly during calculation the new contentY
                                    g_journalModel.fetchMore(g_journalModel.index(0, 0))
                                    viewRoot.forceLayout()
                                }
                                // update currentIndex, because it has changed when rows added at top
                                currentIndex = viewRoot.indexAt(1, viewRoot.contentY + 1)
                                positionViewAtIndex(Math.min(viewRoot.count, currentIndex + scrollIndexSkip), ListView.Beginning)
                            } else {
                                viewRoot.contentY = viewRoot.originY
                            }
                        }
                    }
                    if (event.key === Qt.Key_End) {
                        viewRoot.positionViewAtEnd()
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
                                    viewRoot.forceLayout()
                                }
                                // update currentIndex, because it has changed when rows added at top
                                currentIndex = viewRoot.indexAt(1, viewRoot.contentY + 1)
                                positionViewAtIndex(Math.max(0, currentIndex - scrollIndexSkip), ListView.Beginning)
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
            Text {
                anchors.centerIn: parent
                text: "No log entries apply to current filter selection"
                visible: !viewRoot.logEntriesAvailable
            }
        }
    }

    JournaldViewModel {
        id: g_journalModel
        journalPath: g_config.sessionMode === SessionConfig.LOCALFOLDER || g_config.sessionMode === SessionConfig.REMOTE ? g_config.localJournalPath : undefined
        systemdUnitFilter: g_unitModel.selectedEntries
        bootFilter: bootIdComboBox.bootId
        priorityFilter: priorityComboBox.priority
    }
    ClipboardProxy {
        id: clipboard
    }
}

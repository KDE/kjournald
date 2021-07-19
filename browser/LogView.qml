/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import systemd 1.0

ListView {
    id: root

    required property JournaldViewModel journalModel

    readonly property date currentIndexDateTime: root.journalModel.datetime(root.indexAt(1, root.contentY + root.height / 2))

    signal textCopied(string text)


    Connections {
        target: root.journalModel
        property date lastDateInFocus
        function onModelAboutToBeReset() {
            lastDateInFocus = root.currentIndexDateTime
        }
        function onModelReset() {
            root.currentIndex = root.journalModel.closestIndexForData(lastDateInFocus)
        }
    }

    highlightMoveDuration: 10
    model: root.journalModel
    focus: true
    Component.onCompleted: forceActiveFocus()
    delegate: Rectangle
    {
        color: model.unitcolor
        width: root.width
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
            modelProxy: root.model

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
        var currentIndex = root.indexAt(1, root.contentY + 1) // 1 pixel right and down to really get the first item in view
        var scrollIndexSkip = Math.floor(0.9 * root.height / 30) // hard-coded estimate of one line log height
        if (event.key === Qt.Key_PageDown) {
            if (event.modifiers & Qt.ControlModifier) {
                // model provides just a sliding window over the journal, fetch tail data first
                root.journalModel.seekTail()
                root.positionViewAtEnd()
            }
            else {
                if (root.contentHeight - root.originY > root.height) {
                    if (root.contentY - root.originY + 2 * root.height >= root.contentHeight) {
                        // enforce fetching here such that it does not happen implicitly during calculation the new contentY
                        root.journalModel.fetchMore(root.journalModel.index(0, 0))
                        root.forceLayout()
                    }
                    // update currentIndex, because it has changed when rows added at top
                    currentIndex = root.indexAt(1, root.contentY + 1)
                    positionViewAtIndex(Math.min(root.count, currentIndex + scrollIndexSkip), ListView.Beginning)
                } else {
                    root.contentY = root.originY
                }
            }
        }
        if (event.key === Qt.Key_End) {
            root.positionViewAtEnd()
        }

        if (event.key === Qt.Key_PageUp) {
            if (event.modifiers & Qt.ControlModifier) {
                // model provides just a sliding window over the journal, fetch head data first
                root.journalModel.seekHead()
                root.positionViewAtBeginning()
            }
            else {
                if (root.contentHeight - root.originY > root.height) {
                    if (root.contentY - root.originY <= 3 * root.height) {
                        // enforce fetching here such that it does not happen implictly during calculation the new contentY
                        root.journalModel.fetchMore(root.journalModel.index(0, 0))
                        root.forceLayout()
                    }
                    // update currentIndex, because it has changed when rows added at top
                    currentIndex = root.indexAt(1, root.contentY + 1)
                    positionViewAtIndex(Math.max(0, currentIndex - scrollIndexSkip), ListView.Beginning)
                } else {
                    root.contentY = root.originY
                }
            }
        }
        if (event.key === Qt.Key_F3) {
            var index = root.journalModel.search(hightlightTextField.text, root.currentIndex + 1)
            if (index >= 0) {
                root.currentIndex = index
                positionViewAtIndex(index, ListView.Beginning)
            }
        }
        if (event.key === Qt.Key_C && (event.modifiers & Qt.ControlModifier)) {
            var startIndex = root.indexAt(1, root.contentY)
            var endIndex = root.indexAt(1, root.contentY + root.height);
            if (endIndex < 0) {
                endIndex = root.journalModel.rowCount()
            }
            var content = ""
            for (var i = startIndex; i < endIndex; ++i) {
                content += root.journalModel.formatTime(root.journalModel.data(root.journalModel.index(i, 0), JournaldViewModel.DATE), true) + " UTC "
                            + root.journalModel.data(root.journalModel.index(i, 0), JournaldViewModel.SYSTEMD_UNIT) + " "
                            + root.journalModel.data(root.journalModel.index(i, 0), JournaldViewModel.MESSAGE) + "\n"
            }
            root.textCopied(content)
        }
    }
}

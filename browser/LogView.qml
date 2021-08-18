/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml 2.15
import systemd 1.0

ListView {
    id: root

    /**
     * journalModel the JournaldViewModel object that shall be visualized
     */
    required property JournaldViewModel journalModel

    /**
     * if set to true then scrolling to the end of the view enables automatic following of the journal when it increases
     * @note this requires a journald DB that supports this behavior (e.g. a local journal, which has an FD notifier)
     * @note runtime changes of this property are not supported, i.e. follow mode is only activated after the next move
     * of the content
     */
    property bool snapToFollowMode: false

    property bool __followMode: false
    onContentYChanged: {
        if (snapToFollowMode === true) {
            root.__followMode = root.atYEnd
        }
    }

    /**
     * when new log entries are added, there is a delay between the update of the model (see rowsInserted) and that the
     * visual delagates were created; this approach ensures that on any size change a check is performed if scolling
     * needs to continue to the end
     */
    onContentHeightChanged: {
        if (root.__followMode === true) {
            root.positionViewAtEnd()
        }
    }

    readonly property date currentIndexDateTime: root.journalModel.datetime(root.indexAt(1, root.contentY + root.height / 2))

    /**
     * Event is fired when log text is obtained for using in clipboard
     */
    signal textCopied(string text)

    /**
     * Copy all log entries from the visible part of the view.
     * The result of this copy operation will be provided with the @see textCopied signal
     */
    function copyTextFromView() {
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
    Component.onCompleted: {
        forceActiveFocus()
    }
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
            root.copyTextFromView()
        }
    }
}

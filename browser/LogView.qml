/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml 2.15
import kjournald 1.0

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

    property int displayRoleRight: JournaldViewModel.SYSTEMD_UNIT

    readonly property date currentIndexDateTime: root.journalModel.datetime(root.indexAt(1, root.contentY + root.height / 2))

    /**
     * @private
     * indicates internal state of log viewer if it follows the end of the log
     */
    property bool __followMode: false

    /**
     * if set to yes, then mouse interaction with view lead to text selection and not browsing
     */
    property bool textSelectionMode: false

    /**
     * Event is fired when log text is obtained for using in clipboard
     */
    signal textCopied(string text)

    /**
     * Copy all log entries from the visible part of the view.
     * The result of this copy operation will be provided with the @see textCopied signal
     */
    function copyTextFromView(from, to) {
        var startIndex = Math.min(from, to)
        var endIndex = Math.max(from, to)
        if (endIndex < 0) {
            endIndex = root.journalModel.rowCount()
        }
        var content = ""
        for (var i = startIndex; i <= endIndex; ++i) {
            // TODO print date/time information with user selected time zone
            content += root.journalModel.formatTime(root.journalModel.data(root.journalModel.index(i, 0), JournaldViewModel.DATETIME), true) + " UTC "
                        + root.journalModel.data(root.journalModel.index(i, 0), JournaldViewModel.SYSTEMD_UNIT) + " "
                        + root.journalModel.data(root.journalModel.index(i, 0), JournaldViewModel.MESSAGE) + "\n"
        }
        root.textCopied(content)
    }

    highlightMoveDuration: 10
    model: root.journalModel
    focus: true
    interactive: textSelectionMode === false

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

    Component.onCompleted: {
        forceActiveFocus()
    }
    delegate: Rectangle
    {
        color: {
            if (textSelectionHandler.selectionActive
                && model.index >= Math.min(textSelectionHandler.startIndex, textSelectionHandler.temporaryEndIndex)
                && model.index <= Math.max(textSelectionHandler.startIndex, textSelectionHandler.temporaryEndIndex)) {
                return "#cccccc"
            }
            switch (displayRoleRight) {
            case JournaldViewModel.SYSTEMD_UNIT: return model ? model.systemdunitcolor_background : "#ffffff"
            case JournaldViewModel.EXE: return model ? model.execolor_background : "#ffffff"
            }
            return "#ffffff"
        }
        width: root.width
        height: messageText.height
        LogLine {
            id: messageText
            anchors {
                left: parent.left
                right: parent.right
            }
            index: model.index
            date: model.datetime
            monotonicTimestamp: model.monotonictimestamp
            priority: model.priority
            message: model.message
            highlight: hightlightTextField.text
            modelProxy: root.model

            Rectangle {
                anchors.right: parent.right
                width: categoryInfo.width + 8
                height: categoryInfo.height
                radius: 4
                color: {
                    switch (displayRoleRight) {
                    case JournaldViewModel.SYSTEMD_UNIT: return model ? model.systemdunitcolor_foreground : "#ffffff"
                    case JournaldViewModel.EXE: return model ? model.execolor_foreground : "#ffffff"
                    }
                    return "#ffffff"
                }
                Text {
                    id: categoryInfo
                    anchors {
                        right: parent.right
                        rightMargin: 4
                    }
                    width: Math.min(implicitWidth, 0.5 * messageText.width)
                    text: {
                        switch (displayRoleRight) {
                        case JournaldViewModel.SYSTEMD_UNIT: return model.systemdunit
                        case JournaldViewModel.EXE: return model.exe
                        }
                        return ""
                    }
                    elide: Text.ElideLeft
                }
            }
        }
    }
    TextMetrics {
        id: messageIdMetrics
        text: "Example Log message"
    }

    // separate log by days
    section.property: "date"
    section.criteria: ViewSection.FullString
    section.delegate: Rectangle {
        id: sectionContainer
        width: parent.width
        height: dateSectionText.height
        Text {
            id: dateSectionText
            text: Qt.formatDate(section, "dddd, yyyy-MM-dd")
            font.pixelSize: 20
        }
    }

    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AlwaysOn
        active: ScrollBar.AlwaysOn
    }

    MouseArea {
        id: textSelectionHandler
        property bool selectionActive
        property int startIndex: 0
        property int temporaryEndIndex: 0 // current mouse position's index
        anchors.fill: parent
        enabled: textSelectionMode === true
        onPressed: {
            startIndex = root.indexAt(mouseX, mouseY + root.contentY - root.originY)
            if (startIndex === -1) { // when drag started at section header
                return
            }
            temporaryEndIndex = startIndex
            selectionActive = true
        }
        onPositionChanged: {
            var newIndex = root.indexAt(mouseX, mouseY + root.contentY - root.originY)
            if (newIndex !== -1) {
                temporaryEndIndex = newIndex
            }
        }
        onReleased: {
            if (selectionActive) {
                copyTextFromView(startIndex, temporaryEndIndex)
            }
            selectionActive = false
        }
        onCanceled: {
            selectionActive = false
        }
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
            root.copyTextFromView(root.indexAt(1, root.contentY), root.indexAt(1, root.contentY + root.height))
        }
    }
}

/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick
import QtQuick.Controls.Material
import org.kde.kjournaldbrowser

Item {
    id: root
    required property entry logEntry
    readonly property bool __isHighlighted : logEntry.matches(TextSearch.needle, TextSearch.caseSensitive)

    implicitWidth: text.implicitWidth
    implicitHeight: text.implicitHeight

    Rectangle {
        visible: root.__isHighlighted
        anchors.fill: parent
        color: Material.accent
    }
    Row {
        spacing: 4
        Text {
            readonly property string timeString: {
                switch (BrowserApplication.timeDisplay) {
                case BrowserApplication.UTC: return Formatter.formatTime(root.logEntry.date, true);
                case BrowserApplication.LOCALTIME: return Formatter.formatTime(root.logEntry.date, false);
                case BrowserApplication.MONOTONIC_TIMESTAMP: return (root.logEntry.monotonicTimestamp / 1000).toFixed(3) // display miliseconds
                }
                return ""
            }
            color: root.__isHighlighted ? Material.primaryHighlightedTextColor : Material.iconDisabledColor
            text: timeString
            HoverHandler {
                id: timeHoverHandler
                cursorShape: Qt.PointingHandCursor
            }
            ToolTip.delay: 1000
            ToolTip.timeout: 5000
            ToolTip.visible: timeHoverHandler.hovered
            ToolTip.text: "UTC " + Formatter.formatTime(root.logEntry.date, true)
        }

        Text {
            id: text
            text: root.logEntry.message
            color: {
                if (root.__isHighlighted) {
                    return Material.primaryHighlightedTextColor
                }

                switch(root.logEntry.priority) {
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
        }
    }
}

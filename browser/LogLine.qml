/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick
import QtQuick.Controls.Material
import kjournald

Item {
    id: root
    property string message
    property date date
    property var monotonicTimestamp // unsigned int 64
    property int priority
    property string highlight
    property QtObject modelProxy
    property int index
    readonly property bool __isHighlighted : highlight !== "" && message.includes(root.highlight)

    implicitWidth: text.implicitWidth
    implicitHeight: text.implicitHeight

    Rectangle {
        visible: root.__isHighlighted
        anchors.fill: parent
        color: Material.highlightedButtonColor
    }
    Row {
        spacing: 4
        Text {
            readonly property string timeString: {
                switch (SessionConfigProxy.timeDisplay) {
                case SessionConfig.UTC: return modelProxy.formatTime(root.date, true);
                case SessionConfig.LOCALTIME: return modelProxy.formatTime(root.date, false);
                case SessionConfig.MONOTONIC_TIMESTAMP: return (root.monotonicTimestamp / 1000).toFixed(3) // display miliseconds
                }
                return ""
            }
            color: root.__isHighlighted ? Material.primaryHighlightedTextColor : Material.iconDisabledColor
            text: timeString
        }

        Text {
            id: text
            text: root.message
            color: {
                if (root.__isHighlighted) {
                    return Material.primaryHighlightedTextColor
                }

                switch(root.priority) {
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

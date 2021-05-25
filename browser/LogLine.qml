/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.0

Item {
    id: root
    property string message
    property date date
    property int priority
    property string highlight
    property QtObject modelProxy
    property int index

    implicitWidth: text.implicitWidth
    implicitHeight: text.implicitHeight

    Rectangle {
        visible: highlight !== "" && message.includes(root.highlight)
        anchors.fill: parent
        color: "#ffff00"
    }
    Text {
        id: text
        anchors.fill: parent

        text: modelProxy.formatTime(root.date, g_config.displayUtcTime) + " " + root.message
//        text: root.index + " " + modelProxy.formatTime(root.date, true) + " " + root.message // alternativ output for debugging
        color: {
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

// SPDX-FileCopyrightText: 2021 Claudio Cambra <claudio.cambra@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.CheckBox {
    id: checkbox

    property color color: Kirigami.Theme.highlightColor
    property real radius: 4

    indicator: Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height * 0.8
        width: height
        x: checkbox.leftPadding
        y: parent.height / 2 - height / 2
        radius: checkbox.radius
        border.color: checkbox.color
        border.width: checkbox.visualFocus ? 2 : 1
        color: Qt.rgba(0,0,0,0)

        Rectangle {
            anchors.margins: parent.height * 0.2
            anchors.fill: parent
            radius: checkbox.radius / 3
            color: checkbox.color
            visible: checkbox.checked
        }
    }
}

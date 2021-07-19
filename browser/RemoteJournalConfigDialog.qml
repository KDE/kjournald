/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

Dialog {
    id: root

    readonly property string url : urlTextField.text
    readonly property int port : portSpinbox.value

    title: "Configure Remote Journal Server"
    standardButtons: Dialog.Ok | Dialog.Cancel
    width: 300
    height: 200

    GridLayout {
        columns: 2
        anchors.fill: parent
        anchors.margins: 10
        rowSpacing: 10
        columnSpacing: 10

        // first line
        Text {
            text: "Url:"
        }
        TextField {
            id: urlTextField
            text: "http://127.0.0.1"
        }

        // second line
        Text {
            text: "Port:"
        }
        SpinBox {
            id: portSpinbox
            value: 19531
            to: 99999
            editable: true
            textFromValue: function(value, locale) {
                return Number(value)
            }
        }
    }
}

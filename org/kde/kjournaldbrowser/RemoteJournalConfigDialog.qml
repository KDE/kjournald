// SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
// SPDX-FileCopyrightText: 2021 Andreas Cord-Landwehr <cordlandwehr@kde.org>

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.ki18n

Dialog {
    id: root

    readonly property string url : urlTextField.text
    readonly property int port : portSpinbox.value

    title: KI18n.i18nc("@title:window", "Configure Remote Journal Server")
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
        Label {
            text: KI18n.i18nc("@label", "URL:")
        }
        TextField {
            id: urlTextField
            text: "http://127.0.0.1"
        }

        // second line
        Label {
            text: KI18n.i18nc("@label", "Port:")
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

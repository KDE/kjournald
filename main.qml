/*
    SPDX-License-Identifier: LGPL-2.1-or-later OR MIT
    SPDX-FileCopyrightText: Andreas Cord-Landwehr <cordlandwehr@kde.org>
*/

import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.0
import systemd 1.0

Window {
    width: 640
    height: 480
    visible: true

    ListView {
        anchors.fill: parent
        model: journalModel
        delegate: Rectangle
        {
            color: model.index % 2 === 0 ? "#efefef" : "#ffffff"
            width: parent.width
            height: messageText.height
            Text {
                id: messageText
                anchors {
                    left: parent.left
                    right: parent.right
                }
                text: model.date.toISOString() + "/ " + model.message
                color: {
                    switch(model.priority) {
                    case 0: return "#ff0000" // emergency
                    case 1: return "#cc0000" // alert
                    case 2: return "#cc5200" // critical
                    case 3: return "#00ff00" // error
                    case 4: return "#cc9c00" // warning
                    case 5: return "#015eff" // notice
                    case 6: return "#015e39" // information
                    case 7: return "#000000" // debug
                    }
                }
                Text {
                    anchors.right: parent.right
                    text: model.systemdunit
                }
            }
        }
    }

    JournaldViewModel {
        id: journalModel
        // file journal currently broken
        journalPath: "/opt/workspace/journald-browser/TESTDATA/journal/"
    }
}


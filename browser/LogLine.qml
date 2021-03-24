import QtQuick 2.0

Item {
    id: root
    property string message
    property date date
    property int priority
    property string highlight
    property QtObject modelProxy

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

        text: modelProxy.formatTime(root.date, true) + " " + root.message
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
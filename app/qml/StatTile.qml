import QtQuick

// Small labeled value card used across the dashboard.
Rectangle {
    property string label: ""
    property string value: ""
    property color accent: "#e8e8ee"
    implicitWidth: 160
    implicitHeight: 80
    radius: 10
    color: "#17171d"
    border.color: "#26262e"
    border.width: 1

    Column {
        anchors.fill: parent
        anchors.margins: 13
        spacing: 5
        Text {
            text: label
            color: "#8a8a94"
            font.pixelSize: 11
            font.letterSpacing: 1
        }
        Text {
            text: value
            color: accent
            font.pixelSize: 25
            font.bold: true
        }
    }
}

import QtQuick

// Grid of per-cell tiles. Colour runs red (empty) to green (full) by SoC;
// a faulted cell is forced to a bright warning red.
Rectangle {
    property alias model: repeater.model
    radius: 10
    color: "#17171d"
    border.color: "#26262e"
    border.width: 1

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        Text {
            text: "CELLS"
            color: "#8a8a94"
            font.pixelSize: 11
            font.letterSpacing: 1
        }

        Grid {
            columns: 16
            rowSpacing: 4
            columnSpacing: 4
            Repeater {
                id: repeater
                delegate: Rectangle {
                    required property real soc
                    required property bool faulted
                    width: 15
                    height: 15
                    radius: 3
                    color: faulted ? "#ff3b3b" : Qt.hsva(0.33 * soc, 0.75, 0.9, 1.0)
                    border.color: faulted ? "#ff8a8a" : "transparent"
                    border.width: faulted ? 1 : 0
                }
            }
        }
    }
}

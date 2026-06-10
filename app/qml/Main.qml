import QtQuick
import QtQuick.Window

Window {
    width: 960
    height: 600
    visible: true
    title: "ev-cockpit-sim"
    color: "#101014"

    Text {
        anchors.centerIn: parent
        text: "ev-cockpit-sim: Qt6 QML toolchain OK"
        color: "#39d353"
        font.pixelSize: 28
    }
}

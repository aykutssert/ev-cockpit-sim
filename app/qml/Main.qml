import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import EvCockpit

Window {
    id: win
    width: 1160
    height: 760
    visible: true
    title: "ev-cockpit-sim"
    color: "#101014"

    property int currentTab: 0
    readonly property color warnRed: "#ff3b3b"

    component TabButton2: Rectangle {
        property string label: ""
        property int tabIndex: 0
        implicitWidth: 132
        implicitHeight: 38
        radius: 8
        color: win.currentTab === tabIndex ? "#26262e" : "transparent"
        Text {
            anchors.centerIn: parent
            text: label
            color: win.currentTab === tabIndex ? "white" : "#8a8a94"
            font.pixelSize: 14
            font.bold: win.currentTab === tabIndex
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: win.currentTab = tabIndex
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 18

        // Header: title, tabs, global fault banner.
        RowLayout {
            Layout.fillWidth: true
            spacing: 16
            Text {
                text: "EV COCKPIT"
                color: "white"
                font.pixelSize: 24
                font.bold: true
                font.letterSpacing: 3
            }
            Row {
                spacing: 6
                TabButton2 { label: "Dashboard"; tabIndex: 0 }
                TabButton2 { label: "Software"; tabIndex: 1 }
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                visible: vehicle.faultActive
                radius: 8
                color: "#2a1414"
                border.color: win.warnRed
                border.width: 1
                implicitHeight: 38
                implicitWidth: bannerText.implicitWidth + 28
                Text {
                    id: bannerText
                    anchors.centerIn: parent
                    text: "⚠  " + vehicle.faultText
                    color: win.warnRed
                    font.pixelSize: 14
                    font.bold: true
                    elide: Text.ElideRight
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: win.currentTab
            Dashboard {}
            OtaPanel {}
        }
    }
}

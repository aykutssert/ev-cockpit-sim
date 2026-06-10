import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls
import EvCockpit

Window {
    id: win
    width: 1160
    height: 720
    visible: true
    title: "ev-cockpit-sim"
    color: "#101014"

    readonly property color accentGreen: "#39d353"
    readonly property color warnRed: "#ff3b3b"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 22
        spacing: 18

        // Header with fault banner.
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "EV COCKPIT"
                color: "white"
                font.pixelSize: 24
                font.bold: true
                font.letterSpacing: 3
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
                }
            }
        }

        // Main content: gauge/drive on the left, stats/cells on the right.
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 18

            ColumnLayout {
                spacing: 14
                SocGauge {
                    Layout.alignment: Qt.AlignHCenter
                    value: vehicle.soc
                    arcColor: vehicle.faultActive ? win.warnRed : win.accentGreen
                }
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 12
                    StatTile {
                        label: "SPEED (km/h)"
                        value: Math.round(vehicle.speed).toString()
                    }
                    StatTile {
                        label: "RANGE (km)"
                        value: Math.round(vehicle.range).toString()
                        accent: win.accentGreen
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 14

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12
                    StatTile {
                        Layout.fillWidth: true
                        label: "PACK VOLTAGE (V)"
                        value: vehicle.packVoltage.toFixed(1)
                    }
                    StatTile {
                        Layout.fillWidth: true
                        label: "MAX CELL TEMP (°C)"
                        value: vehicle.maxTemp.toFixed(1)
                        accent: vehicle.maxTemp > 55 ? win.warnRed : "#e8e8ee"
                    }
                    StatTile {
                        Layout.fillWidth: true
                        label: "CURRENT (A)"
                        value: vehicle.current.toFixed(0)
                        accent: vehicle.current < 0 ? "#4db8ff" : "#e8e8ee"
                    }
                }

                BatteryGrid {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: vehicle.cells
                }
            }
        }

        // Controls.
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 84
            radius: 10
            color: "#17171d"
            border.color: "#26262e"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 20

                ColumnLayout {
                    Layout.preferredWidth: 320
                    spacing: 2
                    Text {
                        text: "THROTTLE  " + Math.round(vehicle.throttle * 100) + "%"
                        color: "#8a8a94"
                        font.pixelSize: 11
                        font.letterSpacing: 1
                    }
                    Slider {
                        id: throttle
                        Layout.fillWidth: true
                        from: 0
                        to: 1
                        value: vehicle.throttle
                        enabled: !vehicle.charging
                        onMoved: vehicle.setThrottle(value)
                    }
                }

                RowLayout {
                    spacing: 8
                    Switch {
                        id: chargeSwitch
                        text: "Charge"
                        checked: vehicle.charging
                        onToggled: vehicle.setCharging(checked)
                        contentItem: Text {
                            text: chargeSwitch.text
                            color: "#e8e8ee"
                            font.pixelSize: 14
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: chargeSwitch.indicator.width + 8
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Inject Overtemp"
                    onClicked: vehicle.injectOvertemp()
                }
                Button {
                    text: "Inject Imbalance"
                    onClicked: vehicle.injectImbalance()
                }
                Button {
                    text: "Clear Faults"
                    onClicked: vehicle.clearFaults()
                }
            }
        }
    }
}

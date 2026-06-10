import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import EvCockpit

// The driving/battery view. Reads the global `vehicle` context property.
Item {
    readonly property color accentGreen: "#39d353"
    readonly property color warnRed: "#ff3b3b"

    ColumnLayout {
        anchors.fill: parent
        spacing: 18

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 18

            ColumnLayout {
                spacing: 14
                SocGauge {
                    Layout.alignment: Qt.AlignHCenter
                    value: vehicle.soc
                    arcColor: vehicle.faultActive ? warnRed : accentGreen
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
                        accent: accentGreen
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
                        accent: vehicle.maxTemp > 55 ? warnRed : "#e8e8ee"
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

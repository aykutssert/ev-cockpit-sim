import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import EvCockpit

// Software-update view. Reads the global `ota` context property.
Item {
    readonly property color accentGreen: "#39d353"
    readonly property color warnRed: "#ff3b3b"
    readonly property color blue: "#4db8ff"
    readonly property var stepNames: ["Download", "Verify", "Write slot", "Health check"]

    ColumnLayout {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width, 720)
        spacing: 18

        // Current software card.
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 92
            radius: 10
            color: "#17171d"
            border.color: "#26262e"
            border.width: 1
            RowLayout {
                anchors.fill: parent
                anchors.margins: 18
                ColumnLayout {
                    spacing: 4
                    Text {
                        text: "VEHICLE SOFTWARE"
                        color: "#8a8a94"
                        font.pixelSize: 11
                        font.letterSpacing: 1
                    }
                    Text {
                        text: "v" + ota.activeVersion
                        color: "white"
                        font.pixelSize: 28
                        font.bold: true
                    }
                }
                Item { Layout.fillWidth: true }
                ColumnLayout {
                    spacing: 4
                    Text {
                        text: "ACTIVE SLOT"
                        color: "#8a8a94"
                        font.pixelSize: 11
                        font.letterSpacing: 1
                        Layout.alignment: Qt.AlignRight
                    }
                    Text {
                        text: ota.activeSlot
                        color: blue
                        font.pixelSize: 28
                        font.bold: true
                        Layout.alignment: Qt.AlignRight
                    }
                }
            }
        }

        // Update available / install.
        Rectangle {
            Layout.fillWidth: true
            visible: ota.updateAvailable
            implicitHeight: 70
            radius: 10
            color: "#13211a"
            border.color: accentGreen
            border.width: 1
            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                Text {
                    text: "Update available: v" + ota.availableVersion
                    color: accentGreen
                    font.pixelSize: 16
                    font.bold: true
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: "Install"
                    onClicked: ota.install()
                }
            }
        }

        // Progress + step indicator (during an update).
        Rectangle {
            Layout.fillWidth: true
            visible: ota.busy
            implicitHeight: 132
            radius: 10
            color: "#17171d"
            border.color: "#26262e"
            border.width: 1
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 14

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Repeater {
                        model: stepNames
                        delegate: RowLayout {
                            spacing: 8
                            Rectangle {
                                width: 22
                                height: 22
                                radius: 11
                                color: index < ota.stepIndex ? accentGreen
                                     : index === ota.stepIndex ? blue : "#26262e"
                                Text {
                                    anchors.centerIn: parent
                                    text: index < ota.stepIndex ? "✓" : (index + 1)
                                    color: "white"
                                    font.pixelSize: 12
                                    font.bold: true
                                }
                            }
                            Text {
                                text: modelData
                                color: index === ota.stepIndex ? "white" : "#8a8a94"
                                font.pixelSize: 13
                            }
                        }
                    }
                }

                ProgressBar {
                    Layout.fillWidth: true
                    from: 0
                    to: 1
                    // Show real progress while downloading; peg full for the
                    // short verify/write/health phases.
                    value: ota.stepIndex === 0 ? ota.progress : 1.0
                    indeterminate: ota.stepIndex > 0
                }

                Text {
                    text: ota.message
                    color: "#c8c8d0"
                    font.pixelSize: 13
                }
            }
        }

        // Terminal result banner.
        Rectangle {
            Layout.fillWidth: true
            visible: !ota.busy && !ota.updateAvailable
            implicitHeight: 70
            radius: 10
            color: ota.failed ? "#2a1414" : "#13211a"
            border.color: ota.failed ? warnRed : accentGreen
            border.width: 1
            RowLayout {
                anchors.fill: parent
                anchors.margins: 16
                Text {
                    text: ota.message
                    color: ota.failed ? warnRed : accentGreen
                    font.pixelSize: 15
                    font.bold: true
                }
                Item { Layout.fillWidth: true }
                Button {
                    visible: ota.failed
                    text: "Retry"
                    onClicked: ota.install()
                }
            }
        }

        // Fault injection.
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: faultCol.implicitHeight + 32
            radius: 10
            color: "#17171d"
            border.color: "#26262e"
            border.width: 1
            ColumnLayout {
                id: faultCol
                anchors.fill: parent
                anchors.margins: 16
                spacing: 6
                Text {
                    text: "FAULT INJECTION"
                    color: "#8a8a94"
                    font.pixelSize: 11
                    font.letterSpacing: 1
                }
                CheckBox {
                    text: "Simulate download failure"
                    onToggled: ota.setFailDownload(checked)
                    contentItem: Text {
                        text: parent.text
                        color: "#e8e8ee"
                        font.pixelSize: 13
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: parent.indicator.width + 6
                    }
                }
                CheckBox {
                    text: "Simulate checksum mismatch"
                    onToggled: ota.setFailChecksum(checked)
                    contentItem: Text {
                        text: parent.text
                        color: "#e8e8ee"
                        font.pixelSize: 13
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: parent.indicator.width + 6
                    }
                }
                CheckBox {
                    text: "Simulate health-check failure (triggers rollback)"
                    onToggled: ota.setFailHealth(checked)
                    contentItem: Text {
                        text: parent.text
                        color: "#e8e8ee"
                        font.pixelSize: 13
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: parent.indicator.width + 6
                    }
                }
            }
        }
    }
}

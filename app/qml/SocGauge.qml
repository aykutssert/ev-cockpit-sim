import QtQuick

// Circular arc gauge for a 0..1 value, with a big percentage in the middle.
Item {
    id: root
    property real value: 0
    property color arcColor: "#39d353"
    implicitWidth: 230
    implicitHeight: 230

    onValueChanged: canvas.requestPaint()
    onArcColorChanged: canvas.requestPaint()

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            var cx = width / 2;
            var cy = height / 2;
            var r = Math.min(width, height) / 2 - 16;
            var start = Math.PI * 0.75;
            var end = Math.PI * 2.25;
            var v = Math.max(0, Math.min(1, root.value));

            ctx.lineWidth = 16;
            ctx.lineCap = "round";

            ctx.strokeStyle = "#26262e";
            ctx.beginPath();
            ctx.arc(cx, cy, r, start, end);
            ctx.stroke();

            ctx.strokeStyle = root.arcColor;
            ctx.beginPath();
            ctx.arc(cx, cy, r, start, start + (end - start) * v);
            ctx.stroke();
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 2
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Math.round(root.value * 100) + "%"
            color: "white"
            font.pixelSize: 46
            font.bold: true
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "STATE OF CHARGE"
            color: "#8a8a94"
            font.pixelSize: 12
            font.letterSpacing: 2
        }
    }
}

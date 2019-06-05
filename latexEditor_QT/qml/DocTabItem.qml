import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12

ItemDelegate {
    width: parent.width
    onClicked: {
        maintoolbar.Material.elevation = 0
        stackView.push("TexEdit.qml")
        drawer.close()
    }
    MaterialButton {
        anchors.right: parent.right
        text: "\uf15a"
        font.pixelSize: Qt.application.font.pixelSize * 1.6
        onClicked: {

        }
    }
}

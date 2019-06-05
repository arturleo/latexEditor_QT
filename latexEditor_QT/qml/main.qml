import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.0

ApplicationWindow {
    id: window
    visible: true
    width: 840
    height: 580
    title: qsTr("Latex xwysiwyg editor")

    Component.onCompleted: {
        x = Screen.width / 2 - width / 2
        y = Screen.height / 2 - height / 2
    }

    header: ToolBar {
        id: maintoolbar
        Material.primary: Material.Cyan
        Material.elevation: 2
        contentHeight: toolButton.implicitHeight

        ToolButton {
            id: toolButton
            text: "\u2630"//stackView.depth > 1 ? "\u25C0" : "\u2630"
            font.pixelSize: Qt.application.font.pixelSize * 1.6
            onClicked: {
                if (stackView.depth > 1) {
                    drawer.open()
                    maintoolbar.Material.elevation = 2//TODO color, elevation, current page
//                    stackView.pop()
                } else {
                    drawer.open()
                }
            }
        }

        Label {
            text: stackView.currentItem.title
            anchors.centerIn: parent
        }
    }

    Drawer {
        id: drawer
        width: window.width * 0.3 > 300 ? 300: window.width * 0.3
        height: window.height

        ButtonGroup {
            buttons: doclist.children
        }
        ButtonGroup {
            buttons: settinglist.children
        }

        Column{
            id: homelist
            width:parent.width
            anchors.top: parent.Top
            ItemDelegate {
                text: qsTr("      Home")
                width: parent.width
                onClicked: {
                    if (stackView.depth > 1) {
                        maintoolbar.Material.elevation = 2//TODO color, elevation, current page
                        while(stackView.depth > 1)
                            stackView.pop()
                        drawer.close()
                    } else {
                        drawer.close()
                    }
                }
                MaterialButton {
                    anchors.left: parent.left
                    text: "\uf2dc"
                    font.pixelSize: Qt.application.font.pixelSize * 1.6

                }
            }
        }
        ListView{
            id: doclist
            width:parent.width
            anchors.top: homelist.bottom
            anchors.bottom: settinglist.top
            delegate: Row{
                width: parent.width
                //height: 20
                ItemDelegate {
                    text: docname
                    height:ItemDelegate.height
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
                            //remove
                        }
                    }
                }
            }
            model: ListModel {
                id: docmodel
                ListElement {
                    docname: "01"
                    docindex:1
                }
                ListElement {
                    docname: "02"
                    docindex:2
                }
            }
        }
        Column {
            id: settinglist
            width:parent.width
            anchors.bottom: parent.bottom
            ItemDelegate {
                text: qsTr("      New")
                width: parent.width
                onClicked: {
                    maintoolbar.Material.elevation = 2
                    stackView.push("TexEdit.qml")
                    docmodel.append({docname:"new tex", docindex:docmodel.count+1})
                    drawer.close()
                }
                MaterialButton {
                    anchors.left: parent.left
                    text: "\uf751"
                    font.pixelSize: Qt.application.font.pixelSize * 1.6
                }
            }
            ItemDelegate {
                text: qsTr("      Setting")
                width: parent.width
                onClicked: {
                    maintoolbar.Material.elevation = 2
                    stackView.push("TexSetting.qml")
                    drawer.close()
                }
                MaterialButton {
                    anchors.left: parent.left
                    text: "\uf8ba"
                    font.pixelSize: Qt.application.font.pixelSize * 1.6

                }
            }
        }
    }

    StackView {
        id: stackView
        initialItem: "TexHome.qml"
        anchors.fill: parent

    }
}

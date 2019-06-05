import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.0

Page {
    //width: parent.width
    //height: parent.height

    title: qsTr("Latex Editor Home")
    Rectangle {
        x: parent.x+20
        y: parent.y+20
        width: parent.width*0.66
        height: parent.height
        color: "white"
    GridView {
        id: templategrid
        x :parent.x+20
        y: parent.y
        width: parent.width
        height:parent.height
        //anchors.fill: parent
        cellWidth: 150
        cellHeight: 180
        delegate: Item {
            width: parent.cellWidth
            height: parent.cellHeight
            Column {
                //anchors.fill: parent
                //spacing: 10
                Image {
                    id: templateimg
                    width: 80
                    height: 100
                    source: imgsource+"."+type
                    anchors.horizontalCenter: parent.horizontalCenter
                    MouseArea {
                        hoverEnabled: true
                        onEntered: cursorShape = Qt.PointingHandCursor
                        anchors.fill: parent
                        onClicked: {
                            console.log(index)
                            maintoolbar.Material.elevation = 0
                            if(index!=0){
                                stackView.push("TexEdit.qml",{"defaultdoc":imgsource+".tex",
                                                   "docDisplayMode":1
//                                                   document:{
//                                                       "displaymode":1
//                                                   }
                                               });
                            }
                            else{
                                stackView.push("TexEdit.qml",{"docDisplayMode":0});
                            }
                            drawer.close()
                        }
                    }
                }
                Text {
                    text: "\n"+name
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.bold: true
                }
            }
        }
        model: ListModel {
            ListElement {
                name: "New"
                imgsource: "qrc:/template/docs-blank-googlecolors"
                type: "png"
            }

            ListElement {
                name: "Cheat sheet"
                imgsource: "qrc:/template/cheatsheet"
                type: "png"
            }

            ListElement {
                name: "Chemical equations"
                imgsource: "qrc:/template/chemicalEquations"
                type: "png"
            }

            ListElement {
                name: "Classic thesis styled CV"
                imgsource: "qrc:/template/ClassicthesisStyledCV"
                type: "png"
            }

            ListElement {
                name: "Compact Academic CV"
                imgsource: "qrc:/template/CompactAcademicCV"
                type: "png"
            }

            ListElement {
                name: "Journal Article"
                imgsource: "qrc:/template/journalArticle"
                type: "png"
            }

            ListElement {
                name: "Poems"
                imgsource: "qrc:/template/poems"
                type: "png"
            }

            ListElement {
                name: "Short Stylish Cover Letter"
                imgsource: "qrc:/template/shortStylishCoverLetter"
                type: "png"
            }

            ListElement {
                name: "Thin Formal Letter"
                imgsource: "qrc:/template/thinFormalLetter"
                type: "jpg"
            }

        }
    }
    }

    Rectangle {
        x: templategrid.x+templategrid.width+20
        y: parent.y+20
        width: parent.width-80-templategrid.width
        height: parent.height-40
        color: "white"
        ListView {
            id: historyrow
            anchors.fill: parent

            headerPositioning: ListView.OverlayHeader
            header:Label{
                anchors.horizontalCenter: parent.horizontalCenter
                text: "History"
            }

            delegate:
                Row {
                    id: row1
                    //anchors.horizontalCenter: parent.horizontalCenter
                    width: parent.width-15
                    height: 30
                    //spacing: 10
                    Text {
                        anchors.centerIn: parent
                        text: name
                    }
                    MouseArea {
                       hoverEnabled: true
                       onEntered: cursorShape = Qt.PointingHandCursor
                       anchors.fill: parent
                       onClicked: {
                           // do what you want here
                           maintoolbar.Material.elevation = 0
                           stackView.push("TexEdit.qml")
                           drawer.close()
                       }
                   }
                }

            model: ListModel {
                ListElement {
                    name: "No history, how about creating one?"
                }
            }
        }
    }
}

/*##^## Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
 ##^##*/

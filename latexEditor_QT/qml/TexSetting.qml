import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.0
//pandoc location
//pandoc location
//style?
/*setting:
    connection
        userid
        password
    server:
        mathjax
        syncronize server
        local pandoc//TODO
*/

/*
  userId
  password
  pandocUrl
    setting.setValue("serverUrl","localhost");
    setting.setValue("servelPort",3435);
    setting.setValue("mathJaxUrl","http://127.0.0.1:3434/mathjax");
    setting.setValue("session",session.toVariantMap());//reference?
    setting.setValue("defalut_mode",DISPLAY_VIEW);
*/

Page {
    //width: parent.width
    //height: parent.height

    title: qsTr("Settings")

    Item{
        id: setting
        width: parent.width*0.6>700?700:parent.width*0.6;
        x: parent.width/2 - this.width / 2;
        height:parent.height
        ListView {
            id: listing
            width: setting.width-19
            height: parent.height
            model: settingModel

            property string expandedSection: ""

            delegate: listdelegate

            section.property: "type"
            section.criteria: ViewSection.FullString
            section.delegate: sectionHeader
        }

        ListModel {
            id: settingModel
            ListElement { name: "userid"; type: "connection"; key:"" ;aVisible: false}
            ListElement { name: "password"; type: "connection"; key:"" ;aVisible: false }
            ListElement { name: "mathjax"; type: "server"; key:"" ;aVisible: false }
            ListElement { name: "sync server"; type: "server"; key:"" ;aVisible: false }
            ListElement { name: "local pandoc"; type: "server"; key:"" ;aVisible: false }
        }

        Component {
            id: sectionHeader

            ItemDelegate {
                id: sectionHeaderRect
                width: setting.width-19
                Material.accent: Material.Cyan
                Material.foreground: Material.black
                //Material.primary: Material.grey
                height: 50

                property bool isExpanded: false
                property string currentExpandedSection: ListView.view.expandedSection

                onCurrentExpandedSectionChanged: {
                    //console.log(currentExpandedSection,section);
                    if(currentExpandedSection === section)
                        isExpanded = true;
                    else
                        isExpanded = false;
                }

                onIsExpandedChanged: {
                    if(isExpanded){
                        Material.primary= Material.Cyan;
                        ListView.view.expandedSection = section;
                    }
                    else
                        Material.primary= Material.grey;
                    for(var i=0; i<settingModel.count; i++){
                        var set = settingModel.get(i);
                        if(section === set.type){
                            set.aVisible = sectionHeaderRect.isExpanded;
                            //console.log(set.name,sectionHeaderRect.isExpanded);
                        }
                    }
                }

                Text {
                    id: sectionHeaderText
                    text: section
                    anchors.centerIn: parent
                }

                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        sectionHeaderRect.isExpanded = !sectionHeaderRect.isExpanded;
                    }
                }
            }
        }

        Component {
            id: listdelegate
            Row{
                //Material.accent: Material.Cyan
                Rectangle {
                    id: menuItem
                    width: setting.width-19
                    height: 55
                    //color: "grey"//ListView.isCurrentItem ? "lightblue" : "grey"
                    visible: aVisible

                    onVisibleChanged: {
                        if(visible)
                            height = 55;
                        else
                            height = 0;
                    }

                    Behavior on height {
                        NumberAnimation { duration: 200 }
                    }

                    TextField {
                        id: textField
                        width:auto
                        Material.accent: Material.Cyan
                        anchors.centerIn: parent
                        placeholderText : name
                        visible: aVisible
                    }

                    Button {
                        id: button
                        Material.accent: Material.Cyan
                        y: textField.y-10
                        anchors.left: textField.right
                        text: qsTr("SET")
                        visible: aVisible
                    }
//                    Text {
//                        id: idtext
//                        text:
//                        color: "black"
//                        anchors.centerIn: parent
//                    }

//                    MouseArea {
//                        anchors.fill: parent
//                        onClicked: {
//                            listing.currentIndex = index;
//                        }
//                    }
                }
            }
        }
    }
}

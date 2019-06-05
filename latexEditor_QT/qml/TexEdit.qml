import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.0
import Qt.labs.platform 1.0
import QtQuick.Controls 2.5
import QtQuick.Controls.Material 2.12

import com.latexeditor 0.1
/*what we do when establish new connection?
 *  1: newpage in drawer, register new index
 *  2: session, add new index
 *  3: save document ,cursor position, docid, innerid in session
 *  4: click connect, set id in menu
 *when open page:
 *  1: get index
 *  2: get session
 *  3: if (has id): at present, close old connection, add new connection
 */
Page {
    id:editPage

    property string defaultdoc: ""
    property int docDisplayMode: 0
    //width: parent.width
    //height: parent.height

    title: qsTr("Tex Edit")

    Component.onCompleted: {
        if(defaultdoc.length>0)
            document.load(defaultdoc);
        //console.log(document.displaymode)
        document.displaymode=docDisplayMode
    }

    Shortcut {
        sequence: StandardKey.Open
        onActivated: openDialog.open()
    }
    Shortcut {
        sequence: StandardKey.SaveAs
        onActivated: saveDialog.open()
    }
    Shortcut {
        sequence: StandardKey.Quit
        onActivated: Qt.quit()
    }
    Shortcut {
        sequence: StandardKey.Copy
        onActivated: textEdit.copy()
    }
    Shortcut {
        sequence: StandardKey.Cut
        onActivated: textEdit.cut()
    }
    Shortcut {
        sequence: StandardKey.Paste
        onActivated: textEdit.paste()
    }
    Shortcut {
        sequence: StandardKey.Bold
        onActivated: document.bold = !document.bold
    }
    Shortcut {
        sequence: StandardKey.Italic
        onActivated: document.italic = !document.italic
    }
    Shortcut {
        sequence: StandardKey.Underline
        onActivated: document.underline = !document.underline
    }

    Timer {
            id: timer
    }

    function delay(delayTime, cb) {
            timer.interval = delayTime;
            timer.repeat = false;
            timer.triggered.connect(cb);
            timer.start();
        }

    header: MenuBar {
        id: mainMenu
        //Material.accent: Material.Cyan
        Material.background: Material.Cyan
        width: parent.width

        Menu {
            title: qsTr("&File")

            MenuItem {
                text: qsTr("&Open")
                onTriggered: openDialog.open()
            }
            MenuItem {
                text: qsTr("&Save As...")
                onTriggered: saveDialog.open()
            }
            MenuItem {
                text: qsTr("&Quit")
                onTriggered: Qt.quit()
            }
        }

        Menu {
            title: qsTr("&Edit")

            MenuItem {
                text: qsTr("&Copy")
                enabled: textEdit.selectedText
                onTriggered: textEdit.copy()
            }
            MenuItem {
                text: qsTr("Cu&t")
                enabled: textEdit.selectedText
                onTriggered: textEdit.cut()
            }
            MenuItem {
                text: qsTr("&Paste")
                enabled: textEdit.canPaste
                onTriggered: textEdit.paste()
            }
        }

//        Menu {
//            title: qsTr("F&ormat")

//            MenuItem {
//                text: qsTr("&Bold")
//                checkable: true
//                checked: document.bold
//                onTriggered: document.bold = !document.bold
//            }
//            MenuItem {
//                text: qsTr("&Italic")
//                checkable: true
//                checked: document.italic
//                onTriggered: document.italic = !document.italic
//            }
//            MenuItem {
//                text: qsTr("&Underline")
//                checkable: true
//                checked: document.underline
//                onTriggered: document.underline = !document.underline
//            }
//        }

        Menu {
            title: qsTr("&Sync")
            MenuItem {
                text: qsTr("&Reconnect")
                checkable: false
                checked: client.valid
                onTriggered: {
                    if(client.valid)
                    {
                        client.toDisconnect();
                        delay(500,function(){

                        });
                    }
                    client.toConnect();
                }
            }
            MenuItem {
                text: qsTr("&Convert")
                //checkable: true
                enabled: client.valid
                onTriggered: {

                }
            }
            MenuItem {
                text: qsTr("&ConnectDoc")
                checkable: false
                checked: client.connected
                enabled: client.valid
                onTriggered:{

                    docConnection.open()
                    //docConnection.visible=true
                }
            }
            MenuItem {
                text: qsTr("&Update")
                checkable: true
                enabled: client.connected
                checked: client.update
                onTriggered:{
                    client.update=!client.update
                }
            }

            MenuItem {
                text: qsTr("&Commit")
                //checkable: false
                enabled: client.connected
                onTriggered: {

                }
            }
        }
    }
    Dialog
    {
        id: docConnection
        width: 400
        height: 300
        anchors.centerIn: parent
        //visible: true;
        ListView{
            id: docConnectionList
            height:210
            width:parent.width
            anchors.centerIn: parent
            model: connectionSettingModel

            delegate: Row{
                Rectangle {
                    //anchors.centerIn: parent
                    width: docConnection.width-20
                    height: 70
                    TextField {
                        id: textFieldd
                        //width:auto
                        Material.accent: Material.Cyan
                        x: parent.x+(parent.width-width-buttond.width)/2
                        placeholderText : model.name
                        text: model.content
                        selectByMouse: true
                        //onTextChanged:
                        //visible: aVisible
                    }

                    Button {
                        id: buttond
                        Material.accent: Material.Cyan
                        y: textFieldd.y-10
                        anchors.left: textFieldd.right
                        text: qsTr("SET")
                        onClicked: {
                            var name=textFieldd.placeholderText
                            var str=textFieldd.text
                            //console.log(name, str)
                            setting.setValue(name,str);

                        }

                        //visible: aVisible
                    }
                }
            }
        }

        ListModel {
            id: connectionSettingModel
            Component.onCompleted: {
                //console.log(setting.value("userId",""));
                [
                    ["userId", setting.value("userId","").toString(), ""],
                    ["password", "", ""],
                    ["docId", setting.value("docId","").toString() , ""],
                ].forEach(function(element) {
                    connectionSettingModel.append({
                        name: element[0],
                        content: element[1],
                        key: element[2]
                    });
                });
            }
        }
        Button {
            id: buttonAll
            Material.accent: Material.Cyan
            y: docConnectionList.y+180
            x: parent.x+(parent.width-width)/2-20
            //anchors.left: textFieldd.right
            text: qsTr("CONNCET")
            onClicked: {
                //for(var child in docConnectionList.contentItem.children) {
                    //console.log(docConnectionList.contentItem.children[child].textFieldd);
                    //console.log(docConnectionList.contentItem.children[child].objectName)
//                    setting.setValue(
//                                docConnectionList.contentItem.children[child].placeholderText,
//                                docConnectionList.contentItem.children[child].text
//                                );
//                    console.log(docConnectionList.contentItem.children[child].placeholderText,
//                                docConnectionList.contentItem.children[child].text,
//                                setting.value(docConnectionList.contentItem.children[child].placeholderText).toString())
//
               // }
                //console.log(client.connected,client.valid,client.login);
                if(client.connected){
                    client.sendDisconnect();
                    delay(200,function(){

                    });
                }
                if(!client.login){
                    client.sendLogin();
                                //setting.value("userId").toString(),
                                 //setting.value("password").toString())
                    delay(200,function(){

                    });
                }
                client.sendConnect();
                docConnection.close();
//                setTimeout(function(){
//                }, 500);

            }
            //visible: aVisible
        }

    }

    FileDialog {
        id: openDialog
        fileMode: FileDialog.OpenFile
        selectedNameFilter.index: 1
        nameFilters: ["Tex files (*.tex)","Any files (*.*)"]
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        onAccepted: document.load(file)
    }

    FileDialog {
        id: saveDialog
        fileMode: FileDialog.SaveFile
        defaultSuffix: document.fileType
        nameFilters: openDialog.nameFilters
        selectedNameFilter.index: document.fileType === "txt" ? 0 : 1
        //document.fileUrl.length>0?document.fileUrl:
        //TODO qDebug(baseUrl.resolved(relativeUrl).toString());
        //QUrl relativeUrl("..");
        folder: StandardPaths.writableLocation(StandardPaths.DocumentsLocation)
        onAccepted: document.saveAs(file)
    }

    FontDialog {
        id: fontDialog
        onAccepted: {
            document.fontFamily = font.family;
            document.fontSize = font.pointSize;
        }
    }

    ColorDialog {
        id: colorDialog
        currentColor: "black"
    }

    MessageDialog {
        id: errorDialog
    }

    ToolBar {
        id: editToolBar
        //anchors.top: mainMenu.bottom
        width: parent.width
        leftPadding: 8
        Material.accent: Material.Cyan
        Material.background: "white"
        //Material.primary: Material.Grey
        Material.foreground: "black"

        Flow {
            id: toolFlow
            width: parent.width

            Row {
                id: fileRow
                MaterialButton {
                    id: openButton
                    text: "\ufdab" // mdi-folder-open-outline
                    onClicked: openDialog.open()
                }
                ToolSeparator {
                    contentItem.visible: fileRow.y === editRow.y
                }
            }

            Row {
                id: editRow
                MaterialButton {
                    id: copyButton
                    text: "\uf18f" //mdi-content-copy
                    focusPolicy: Qt.TabFocus
                    enabled: textEdit.selectedText
                    onClicked: textEdit.copy()
                }
                MaterialButton {
                    id: cutButton
                    text: "\uf190" // mdi-content-cut
                    focusPolicy: Qt.TabFocus
                    enabled: textEdit.selectedText
                    onClicked: textEdit.cut()
                }
                MaterialButton {
                    id: pasteButton
                    text: "\uf192" // mdi-content-paste
                    font.family: "fontello"
                    focusPolicy: Qt.TabFocus
                    enabled: textEdit.canPaste
                    onClicked: textEdit.paste()
                }
                ToolSeparator {
                    contentItem.visible: editRow.y === styleRow.y
                }
            }

            Row {
                id: styleRow
                MaterialButton {
                    id: boldButton
                    text: "\uf264" // mdi-format-bold
                    focusPolicy: Qt.TabFocus
                    checkable: true
                    checked: document.bold
                    onClicked: {
                        //textEdit.insert(textEdit.cursorPosition,"\\textbf{}");
                        //textEdit.cursorPosition -= 1;
                        document.bold = !document.bold
                    }
                }
                MaterialButton {
                    id: italicButton
                    text: "\uf277" // mdi-format-italic
                    focusPolicy: Qt.TabFocus
                    checkable: true
                    checked: document.italic
                    onClicked: {
                        //textEdit.insert(textEdit.cursorPosition,"\\textit{}");
                        //textEdit.cursorPosition -= 1;
                        document.italic = !document.italic
                    }
                }
                MaterialButton {
                    id: underlineButton
                    text: "\uf287" // mdi-format-underline
                    focusPolicy: Qt.TabFocus
                    checkable: true
                    checked: document.underline
                    onClicked: document.underline = !document.underline
                }
                MaterialButton {
                    id: fontFamilyButton
                    text: qsTr("\uf6d5") // mdi-format-font
                    font.bold: document.bold
                    font.italic: document.italic
                    font.underline: document.underline
                    onClicked: {
                        fontDialog.currentFont.family = document.fontFamily;
                        fontDialog.currentFont.pointSize = document.fontSize;
                        fontDialog.open();
                    }
                }
                MaterialButton {//TODO
                    id: fontBiggerButton
                    text: qsTr("\uf9f3") // mdi-format-font-size-increase
                    onClicked: {
                        fontDialog.currentFont.family = document.fontFamily;
                        fontDialog.currentFont.pointSize = document.fontSize;
                        fontDialog.open();
                    }
                }
                MaterialButton {//TODO
                    id: fontSmallerButton
                    text: qsTr("\uf9f2") // mdi-format-font-size-decrease
                    onClicked: {
                        fontDialog.currentFont.family = document.fontFamily;
                        fontDialog.currentFont.pointSize = document.fontSize;
                        fontDialog.open();
                    }
                }
                MaterialButton {
                    id: textColorButton
                    text: "\uf69d" // mdi-format-color-text
                    focusPolicy: Qt.TabFocus
                    onClicked: colorDialog.open()

                    Rectangle {
                        width: aFontMetrics.width + 3
                        height: 2
                        color: document.textColor
                        parent: textColorButton.contentItem
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.baseline: parent.baseline
                        anchors.baselineOffset: 6

                        TextMetrics {
                            id: aFontMetrics
                            font: textColorButton.font
                            text: textColorButton.text
                        }
                    }
                }
                ToolSeparator {
                    contentItem.visible: styleRow.y === alignRow.y
                }
            }

            Row {
                id: alignRow
                MaterialButton {
                    id: alignLeftButton
                    text: "\uf262" // mdi-format-align-left
                    focusPolicy: Qt.TabFocus
                    checkable: true
                    checked: document.alignment == Qt.AlignLeft
                    onClicked: document.alignment = Qt.AlignLeft
                }
                MaterialButton {
                    id: alignCenterButton
                    text: "\uf260" // mdi-format-align-center
                    focusPolicy: Qt.TabFocus
                    checkable: true
                    checked: document.alignment == Qt.AlignHCenter
                    onClicked: document.alignment = Qt.AlignHCenter
                }
                MaterialButton {
                    id: alignRightButton
                    text: "\uf263" // mdi-format-align-right
                    focusPolicy: Qt.TabFocus
                    checkable: true
                    checked: document.alignment == Qt.AlignRight
                    onClicked: document.alignment = Qt.AlignRight
                }
                MaterialButton {
                    id: alignJustifyButton
                    text: "\uf261" // mdi-format-align-justify
                    font.family: "fontello"
                    focusPolicy: Qt.TabFocus
                    checkable: true
                    checked: document.alignment == Qt.AlignJustify
                    onClicked: document.alignment = Qt.AlignJustify
                }
            }
            Row {
                id: formatRow
                ComboBox {
                    id: formatCombo
                    textRole: "key"
                    model: ListModel{
                        ListElement {
                           key: "author";
                           fontweight: Font.DemiBold
                        }
                        ListElement {
                            key: "Title";
                            fontweight: Font.Normal
                        }
                        ListElement {
                            key: "Subtitle";
                            fontweight: Font.Normal
                        }
                        ListElement {
                            key: "date";
                            fontweight: Font.Normal
                        }
                        ListElement {
                            key: "tableofcontents";
                            fontweight: Font.Normal
                        }
                    }
                    delegate: ItemDelegate {
                        font.weight: fontweight
                        text: key
                        width: formatCombo.width
                        highlighted: formatCombo.currentIndex == index
                        //enabled: modelData.enabled
                    }
                }
                ComboBox {
                    id: styleCombo
                    textRole: "key"
                    model: ListModel{
                        ListElement {
                           key: "abstract";
                           fontweight: Font.DemiBold
                        }
                        ListElement {
                           key: "part";
                           fontweight: Font.DemiBold
                        }
                        ListElement {
                            key: "chapter";
                            fontweight: Font.Normal
                        }
                        ListElement {
                            key: "section";
                            fontweight: Font.Normal
                        }
                        ListElement {
                            key: "subsection";
                            fontweight: Font.Normal
                        }
                        ListElement {
                            key: "subsubsection";
                            fontweight: Font.Normal
                        }
                        ListElement {
                            key: "paragraph";
                            fontweight: Font.Normal
                        }
                        ListElement {
                            key: "subparagraph";
                            fontweight: Font.Normal
                        }
                    }
                    delegate: ItemDelegate {
                        font.weight: fontweight
                        text: key
                        width: parent.width
                        highlighted: styleCombo.currentIndex == index
                        //enabled: modelData.enabled
                    }
                }
                ComboBox {
                    id: sizeCombo
                    textRole: "key"
                    model: ListModel{
                        ListElement {
                           key: "tiny";
                           fontweight: Font.DemiBold
                           tag:"tiny"
                        }
                        ListElement {
                           key: "script size";
                           fontweight: Font.DemiBold
                           tag:"script size"
                        }
                        ListElement {
                            key: "footnote size";
                            fontweight: Font.Normal
                            tag:"footnotesize"
                        }
                        ListElement {
                            key: "small";
                            fontweight: Font.Normal
                            tag:"small"
                        }
                        ListElement {
                            key: "normal size";
                            fontweight: Font.Normal
                            tag: "normalsize";
                        }
                        ListElement {
                            key: "large";
                            fontweight: Font.Normal
                            tag: "large";
                        }
                        ListElement {
                            key: "Large";
                            fontweight: Font.Normal
                            tag: "Large";
                        }
                        ListElement {
                            key: "LARGE";
                            fontweight: Font.Normal
                            tag: "LARGE";
                        }
                        ListElement {
                            key: "huge";
                            fontweight: Font.Normal
                            tag: "huge";
                        }
                        ListElement {
                            key: "HUGE";
                            fontweight: Font.Normal
                            tag: "HUGE";
                        }
                    }
                    delegate: ItemDelegate {
                        font.weight: fontweight
                        text: key
                        width: parent.width
                        highlighted: styleCombo.currentIndex == index
                        //enabled: modelData.enabled
                    }
                }
            }
        }
    }

    LatexHandler {
        id: document
        //property bool completed: false
        //setting: setting
        //client: client
        document: textEdit.textDocument
        cursorPosition: textEdit.cursorPosition
        selectionStart: textEdit.selectionStart
        selectionEnd: textEdit.selectionEnd
        textColor: colorDialog.color
        Component.onCompleted: {
            initHighlighter();
            init(setting, client);

            //completed=true;
        }

        onCursorPositionChanged: {
            //if(completed)
                textEdit.cursorPosition = cursorPosition
        }

        onLoaded: {
            textEdit.text = text
        }
        onError: {
            errorDialog.text = message
            errorDialog.visible = true
        }
    }

    Flickable {
        id: flickable
        flickableDirection: Flickable.VerticalFlick
        anchors.top: editToolBar.bottom
        anchors.left: parent.left
        //width: parent.width
        //height: editPage.height-editToolBar.height-mainMenu.height
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        focus:true

//        function ensureVisible(r)
//        {
//            if (contentX >= r.x)
//                contentX = r.x;
//            else if (contentX+width <= r.x+r.width)
//                contentX = r.x+r.width-width;
//            if (contentY >= r.y)
//                contentY = r.y;
//            else if (contentY + height <= r.y+r.height)
//                contentY = r.y+r.height-height;
//        }


        TextArea.flickable: TextArea {
            id: textEdit
            Material.accent: Material.Cyan
            anchors.fill: flickable
//            width: parent.width
//            height: parent.height
            textFormat: Qt.AutoText
            wrapMode: TextEdit.Wrap
            focus: true
            selectByMouse: true
            selectByKeyboard: true
            //selectByMouseChanged: true
            persistentSelection: true

            font.hintingPreference:Font.PreferFullHinting

            //onTextChanged: //document.textChanged();
            //onCursorPositionChanged: document.jump();

            Keys.onPressed: {
                //if(event.key==Qt.Key)
                if(event.text.length>0&&client.connected && client.update)
                    document.textChanged();
                //In fact, directly write in qml is better
                if (event.key == Qt.Key_BraceLeft) {
                    //console.log("BraceLeft");
                    document.bracket('{');
                    //cursorPosition -= 1;
                    event.accepted = true;
                }
                else if(event.key == Qt.Key_BracketLeft)
                {
                    //console.log("Key_BracketLeft");
                    //console.log(cursorPosition);
                    document.bracket('[');
                    //cursorPosition -= 1;
                    //console.log(cursorPosition);
                    event.accepted = true;
                }
                else if(event.key == Qt.Key_ParenLeft)
                {
                    //console.log("Key_BracketLeft");

                    document.bracket('(');
                    //cursorPosition -= 1;
                    event.accepted = true;
                }
            }
            //onCursorRectangleChanged: flickable.ensureVisible(cursorRectangle)

            MouseArea {
                id:textmousearea
                acceptedButtons: Qt.RightButton
                anchors.fill: parent
                onClicked: contextMenu.open()
            }

            onLinkActivated: Qt.openUrlExternally(link)
        }

        ScrollBar.vertical: ScrollBar {
            id:scrollBarin
        }
    }

    footer:Item{
        Material.accent: Material.Cyan
        Switch {
            id: switchsplit
            x: parent.x+parent.width-20-width
            anchors.bottom: parent.bottom
            text: qsTr("split")
            checked: (document.displaymode & 2)!=0
            enabled: (document.displaymode & 1)!=0
            onCheckedChanged: {
                document.displaymode ^= 2;
            }
        }

        Switch {
            id: switchview
            anchors.right: switchsplit.left
            anchors.bottom: parent.bottom
            text: qsTr("view/edit")
            checked: (document.displaymode & 1)!=0
            enabled: (document.displaymode & 2)==0
            onCheckedChanged: {
                document.displaymode ^= 1;
            }
        }
    }

    Menu {
        id: contextMenu
        x:textmousearea.mouseX
        y:textmousearea.mouseY+editToolBar.height

        MenuItem {
            text: qsTr("Copy")
            enabled: textEdit.selectedText
            onTriggered: textEdit.copy()
        }
        MenuItem {
            text: qsTr("Cut")
            enabled: textEdit.selectedText
            onTriggered: textEdit.cut()
        }
        MenuItem {
            text: qsTr("Paste")
            enabled: textEdit.canPaste
            onTriggered: textEdit.paste()
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Font...")
            onTriggered: fontDialog.open()
        }

        MenuItem {
            text: qsTr("Color...")
            onTriggered: colorDialog.open()
        }
    }
}

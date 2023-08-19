import QtQuick
import QtQuick.Window
import QtQuick.Controls

Window {
    id : ww
    width: 640
    height: 480
    visible: true
    property bool showText: true
    title: qsTr("Hello World")
    Text {
        id: name11
        text: qsTr("this is text")
        visible: ww.showText
    }
    Rectangle {
        id: leftop
        height: 20
        color:"red"
        width : 20
        anchors.top: name11.bottom

        Rectangle {
        id:k2
             height: 10
            color:"green"
            width : 10
            anchors.centerIn: parent
        }
    }

    Image {
        id: rightop
        anchors.top: parent.top
        anchors.right: parent.right
        source:"qrc:/appqml/res/green.png"
    }


    Rectangle {
        id: leftbottom
        height: 20
        color:"red"
        width : 20
        anchors.bottom: parent.bottom
        anchors.left: parent.left
    }


    Rectangle {
        id: rightbottom
        height: 20
        color:"red"
        width : 20
        anchors.bottom: parent.bottom
        anchors.right: parent.right
    }

    Button {
        anchors.centerIn: parent
        text: "click me"
        width:100
        x:100
        onClicked: {
            ww.showText = !ww.showText;
        }
    }
}

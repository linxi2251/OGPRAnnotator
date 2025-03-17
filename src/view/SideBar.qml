import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OGPRAnnotator
import "components/dialogs"

Rectangle {
    id: root
    color: palette.window
    width: 250
    border.color: "#cccccc"
    border.width: 1
    
    property var fileSystemModel: null
    
    // 添加信号，用于请求保存当前标注
    signal saveCurrentAnnotationsRequested()
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 5
        spacing: 5
        
        // 文件夹选择按钮
        Button {
            text: qsTr("选择文件夹")
            Layout.fillWidth: true
            onClicked: folderSelectionDialog.open()
        }
        
        // 文件夹路径显示
        Label {
            id: folderPathLabel
            text: fileSystemModel ? fileSystemModel.folderPath : ""
            elide: Text.ElideMiddle
            Layout.fillWidth: true
            font.italic: true
            visible: text !== ""
        }
        
        // 图片导航按钮
        RowLayout {
            Layout.fillWidth: true
            visible: fileSystemModel && fileSystemModel.imageFiles.length > 0
            
            Button {
                text: qsTr("上一张")
                enabled: fileSystemModel && fileSystemModel.currentImageIndex > 0
                onClicked: {
                    // 请求保存当前图片的标注
                    if (fileSystemModel.currentImageIndex >= 0) {
                        root.saveCurrentAnnotationsRequested()
                    }
                    fileSystemModel.previousImage()
                }
            }
            
            Label {
                text: fileSystemModel ? (fileSystemModel.currentImageIndex + 1) + "/" + fileSystemModel.imageFiles.length : ""
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
            
            Button {
                text: qsTr("下一张")
                enabled: fileSystemModel && fileSystemModel.currentImageIndex < fileSystemModel.imageFiles.length - 1
                onClicked: {
                    // 请求保存当前图片的标注
                    if (fileSystemModel.currentImageIndex >= 0) {
                        root.saveCurrentAnnotationsRequested()
                    }
                    fileSystemModel.nextImage()
                }
            }
        }
        
        // 图片列表
        ListView {
            id: imageListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: fileSystemModel ? fileSystemModel.imageFiles : []
            currentIndex: fileSystemModel ? fileSystemModel.currentImageIndex : -1
            
            delegate: Rectangle {
                width: imageListView.width
                height: 40
                color: ListView.isCurrentItem ? palette.base : "transparent"
                border.color: ListView.isCurrentItem ? "#808080" : "transparent"
                border.width: 1
                radius: 3
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 5
                    
                    Label {
                        text: (index + 1) + "."
                        font.bold: true
                        Layout.preferredWidth: 30
                    }
                    
                    Label {
                        text: fileSystemModel ? fileSystemModel.getImageFileName(index) : ""
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }
                }
                
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (fileSystemModel) {
                            // 请求保存当前图片的标注
                            if (fileSystemModel.currentImageIndex >= 0) {
                                root.saveCurrentAnnotationsRequested()
                            }
                            // 设置新的图片索引
                            fileSystemModel.setCurrentImageIndex(index)
                        }
                    }
                }
            }
            
            ScrollBar.vertical: ScrollBar {}
        }
    }
}

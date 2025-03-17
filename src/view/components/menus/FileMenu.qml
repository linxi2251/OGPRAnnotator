import QtQuick
import QtQuick.Controls

Menu {
  id: root
  title: qsTr("文件")
  
  signal openImageRequested()
  signal saveAnnotationRequested()
  
  MenuItem {
    text: qsTr("打开图片")
    onTriggered: root.openImageRequested()
  }
  
  MenuItem {
    text: qsTr("保存标注")
    onTriggered: root.saveAnnotationRequested()
  }
} 
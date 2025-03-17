import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
  id: root
  title: "快捷键列表"
  standardButtons: Dialog.Close
  width: 400
  height: 300
  modal: true
  anchors.centerIn: parent

  background: Rectangle {
    color: palette.base
  }
  
  ColumnLayout {
    anchors.fill: parent
    spacing: 10
    
    Label {
      text: "可用的快捷键"
      font.bold: true
      font.pixelSize: 16
      Layout.alignment: Qt.AlignHCenter
    }
    
    ScrollView {
      Layout.fillWidth: true
      Layout.fillHeight: true
      clip: true
      
      GridLayout {
        width: parent.width
        columns: 2
        columnSpacing: 20
        rowSpacing: 10
        
        Label { text: "Ctrl+Z"; font.bold: true }
        Label { text: "撤销上一步操作" }
        
        Label { text: "Ctrl+Y"; font.bold: true }
        Label { text: "重做上一步操作" }
        
        Label { text: "Delete"; font.bold: true }
        Label { text: "删除选中的标注" }
        
        Label { text: "鼠标左键"; font.bold: true }
        Label { text: "绘制新标注或选择已有标注" }
        
        Label { text: "长按鼠标左键"; font.bold: true }
        Label { text: "在选中标注上长按可以移动标注" }
        
        Label { text: "鼠标右键"; font.bold: true }
        Label { text: "在标注上点击显示上下文菜单" }
      }
    }
  }
} 

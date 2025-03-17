import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import OGPRAnnotator

// 类别图例组件
Rectangle {
  id: root
  width: 120
  color: "white"
  border.color: "black"
  border.width: 1
  radius: 5
  
  property var categoryManager: null
  
  Column {
    anchors {
      fill: parent
      margins: 5
    }
    spacing: 5
    
    Label {
      text: "类别图例"
      font.bold: true
      anchors.horizontalCenter: parent.horizontalCenter
    }
    
    Repeater {
      model: categoryManager ? categoryManager.categories : []
      delegate: Row {
        spacing: 5
        Rectangle {
          width: 15
          height: 15
          color: categoryManager.getCategoryColor(modelData)
        }
        Label {
          text: modelData
        }
      }
    }
  }
}

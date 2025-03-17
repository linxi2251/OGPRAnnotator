import QtQuick
import QtQuick.Dialogs

FileDialog {
  id: root
  title: "选择图片"
  nameFilters: ["Image files (*.jpg *.jpeg *.png *.bmp)"]
  
  property string imageSource
  
  onAccepted: {
    imageSource = selectedFile
  }
} 
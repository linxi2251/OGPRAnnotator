import QtQuick
import QtQuick.Dialogs

FileDialog {
  id: root
  title: "导入类别列表"
  nameFilters: ["JSON files (*.json)"]
  
  signal importCategories(string filePath)
  
  onAccepted: {
    const realPath = selectedFile.toString().replace("file:///", "")
    importCategories(realPath)
  }
} 
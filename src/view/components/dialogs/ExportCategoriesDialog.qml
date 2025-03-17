import QtQuick
import QtQuick.Dialogs

FileDialog {
  id: root
  title: "导出类别列表"
  fileMode: FileDialog.SaveFile
  nameFilters: ["JSON files (*.json)"]
  
  signal exportCategories(string filePath)
  
  onAccepted: {
    const realPath = selectedFile.toString().replace("file:///", "")
    exportCategories(realPath)
  }
} 
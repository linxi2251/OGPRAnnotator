import QtQuick
import QtQuick.Dialogs

FileDialog {
  id: root
  title: "保存标注"
  fileMode: FileDialog.SaveFile
  nameFilters: ["JSON files (*.json)"]
  
  signal saveAnnotations(string filePath)
  
  onAccepted: {
    const realPath = selectedFile.toString().replace("file:///", "")
    saveAnnotations(realPath)
  }
} 
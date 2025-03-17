import QtQuick
import QtQuick.Dialogs

FolderDialog {
  id: root
  title: "选择图片文件夹"
  
  property string selectedFolderPath: ""
  signal folderSelected(string folder)
  
  onAccepted: {
    selectedFolderPath = selectedFolder
    folderSelected(selectedFolder)
  }
} 
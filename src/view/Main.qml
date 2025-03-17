import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OGPRAnnotator
import "./components/menus"
import "./components/dialogs"

ApplicationWindow {
  id: mainWindow
  visible: true
  width: 1280
  height: 720
  title: qsTr("图片标注工具")

  menuBar: MenuBar {
    FileMenu {
      id: fileMenu
      onOpenImageRequested: imageFileDialog.open()
      onSaveAnnotationRequested: saveDialog.open()
    }
    
    CategoryMenu {
      id: categoryMenu
      onAddCategoryRequested: annotationArea.addCategoryDialog.open()
      onImportCategoriesRequested: importCategoriesDialog.open()
      onExportCategoriesRequested: exportCategoriesDialog.open()
    }
    
    HelpMenu {
      id: helpMenu
      onShowShortcutsRequested: shortcutsDialog.open()
    }
  }

  ImageFileDialog {
    id: imageFileDialog
    onImageSourceChanged: {
      annotationArea.imageSource = imageSource
    }
  }

  SaveDialog {
    id: saveDialog
    onSaveAnnotations: function(filePath) {
      annotationArea.saveAnnotations(filePath)
    }
  }
  
  ImportCategoriesDialog {
    id: importCategoriesDialog
    onImportCategories: function(filePath) {
      annotationArea.importCategories(filePath)
    }
  }
  
  ExportCategoriesDialog {
    id: exportCategoriesDialog
    onExportCategories: function(filePath) {
      annotationArea.exportCategories(filePath)
    }
  }

  // 快捷键对话框
  ShortcutsDialog {
    id: shortcutsDialog
  }

  // Add message dialog for save status
  MessageDialog {
    id: messageDialog
  }

  // 文件夹选择对话框
  FolderSelectionDialog {
    id: folderSelectionDialog
    currentFolder: fileSystemModel.folderPath
    onFolderSelected: function(folder) {
      fileSystemModel.openFolder(folder)
    }
  }

  // 文件系统模型实例
  FileSystemModel {
    id: fileSystemModel
    onCurrentImagePathChanged: {
      // 当图片路径改变时，加载对应的标注文件
      console.log("annotationFilePath")
      if (currentImagePath !== "") {
        const annotationFilePath = getAnnotationFilePath(currentImageIndex)
        const fileUrl = "file:///" + annotationFilePath
        annotationArea.loadAnnotations(fileUrl)
      } else {
        annotationArea.clearAnnotations()
      }
    }
  }
  
  RowLayout {
    anchors.fill: parent
    spacing: 0
    
    // 侧边栏
    SideBar {
      id: sideBar
      Layout.fillHeight: true
      Layout.preferredWidth: 250
      fileSystemModel: fileSystemModel
      
      // 连接保存标注请求信号
      onSaveCurrentAnnotationsRequested: {
        annotationArea.saveCurrentImageAnnotations()
      }
    }
    
    // 主内容区域
    ColumnLayout {
      Layout.fillWidth: true
      Layout.fillHeight: true
      spacing: 5

      ToolBar {
        Layout.fillWidth: true
        RowLayout {
          anchors.fill: parent
          ToolButton {
            text: qsTr("清除所有标注")
            onClicked: annotationArea.clearAnnotations()
          }
        }
      }
      
      // 标注区域
      AnnotationArea {
        id: annotationArea
        Layout.fillWidth: true
        Layout.fillHeight: true
        imageSource: "file:///" + fileSystemModel.currentImagePath
        
        // 保存当前图像的标注
        function saveCurrentImageAnnotations() {
          if (fileSystemModel.currentImagePath === "") {
            console.log("No image loaded, cannot save annotations");
            return false;
          }
          
          var annotationFilePath = fileSystemModel.getAnnotationFilePath(fileSystemModel.currentImageIndex);
          console.log("Saving annotations to:", annotationFilePath);
          
          // 调用 AnnotationArea 的 saveAnnotations 方法
          annotationArea.saveAnnotations(annotationFilePath);
          return true;
        }
      }

      Button {
        text: qsTr("保存当前标注")
        Layout.fillWidth: true
        visible: fileSystemModel && fileSystemModel.currentImageIndex >= 0
        onClicked: {
          annotationArea.saveCurrentImageAnnotations()
        }
      }
    }
  }
}

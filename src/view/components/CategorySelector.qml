import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OGPRAnnotator

// 类别选择器组件
Item {
  id: root
  height: 40
  
  property alias currentCategory: categoryComboBox.currentText
  property alias categoryManager: categoryManager
  
  signal categoryChanged(string category)
  
  CategoryManager {
    id: categoryManager
    onOperationCompleted: function(success, message) {
      console.log("Category operation:", success, message)
    }
  }
  
  RowLayout {
    anchors.fill: parent
    spacing: 5
    
    Label {
      text: "选择类别:"
    }
    
    ComboBox {
      id: categoryComboBox
      model: categoryManager.categories
      currentIndex: 0
      onCurrentTextChanged: {
        categoryChanged(currentText)
      }
    }
    
    Button {
      text: "添加类别"
      onClicked: {
        addCategoryDialog.open()
      }
    }
    
    Item {
      Layout.fillWidth: true
    }
  }
  
  // 添加类别对话框
  Dialog {
    id: addCategoryDialog
    title: "添加新类别"
    standardButtons: Dialog.Ok | Dialog.Cancel
    
    ColumnLayout {
      anchors.fill: parent
      
      TextField {
        id: newCategoryField
        placeholderText: "输入新类别名称"
        Layout.fillWidth: true
      }
      
      ColorDialog {
        id: categoryColorDialog
        title: "选择类别颜色"
        selectedColor: "#000000"
      }
      
      Button {
        text: "选择颜色"
        onClicked: categoryColorDialog.open()
        Layout.fillWidth: true
      }
    }
    
    onAccepted: {
      if (newCategoryField.text.trim() !== "") {
        const newCategory = newCategoryField.text.trim()
        
        // 确保颜色值是有效的颜色字符串
        let colorStr = categoryColorDialog.selectedColor.toString()
        // 如果颜色值不是以#开头的十六进制颜色，则转换它
        if (!colorStr.startsWith('#')) {
          // 提取RGB值并转换为十六进制
          const colorObj = categoryColorDialog.selectedColor
          const r = Math.floor(colorObj.r * 255).toString(16).padStart(2, '0')
          const g = Math.floor(colorObj.g * 255).toString(16).padStart(2, '0')
          const b = Math.floor(colorObj.b * 255).toString(16).padStart(2, '0')
          colorStr = `#${r}${g}${b}`
        }
        
        // 使用C++类添加类别
        categoryManager.addCategory(newCategory, colorStr)
        
        // 更新ComboBox选择最新添加的类别
        categoryComboBox.currentIndex = categoryManager.categories.length - 1
        
        console.log("添加新类别: " + newCategory + " 颜色: " + colorStr)
      }
    }
  }
  
  // 导入类别列表的函数
  function importCategories(filePath) {
    // 使用C++类导入类别
    categoryManager.importCategories(filePath)
  }
  
  // 导出类别列表的函数
  function exportCategories(filePath) {
    // 使用C++类导出类别
    categoryManager.exportCategories(filePath)
  }
}

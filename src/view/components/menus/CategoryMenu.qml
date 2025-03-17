import QtQuick
import QtQuick.Controls

Menu {
  id: root
  title: qsTr("类别")
  
  signal addCategoryRequested()
  signal importCategoriesRequested()
  signal exportCategoriesRequested()
  
  MenuItem {
    text: qsTr("添加新类别")
    onTriggered: root.addCategoryRequested()
  }
  
  MenuItem {
    text: qsTr("导入类别列表")
    onTriggered: root.importCategoriesRequested()
  }
  
  MenuItem {
    text: qsTr("导出类别列表")
    onTriggered: root.exportCategoriesRequested()
  }
} 
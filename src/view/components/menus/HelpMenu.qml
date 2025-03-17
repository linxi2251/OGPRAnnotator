import QtQuick
import QtQuick.Controls

Menu {
  id: root
  title: qsTr("帮助")
  
  signal showShortcutsRequested()
  
  MenuItem {
    text: qsTr("查看快捷键")
    onTriggered: root.showShortcutsRequested()
  }
} 
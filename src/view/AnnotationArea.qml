import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import OGPRAnnotator

// 引入自定义组件
import "./components"

Rectangle {
  id: root
  color: palette.window

  property var annotations: []
  property alias imageSource: imageViewer.imgSource
  property string currentCategory: categorySelector.currentCategory
  property int selectedAnnotationIndex: annotationCanvas.selectedAnnotationIndex

  signal saveStatus(bool status, string msg)
  
  // 添加快捷键
  Shortcut {
    sequence: "Ctrl+Z"
    onActivated: {
      const newAnnotations = historyManager.undo()
      if (newAnnotations.length > 0) {
        annotations = newAnnotations
        annotationCanvas.requestPaint()
      }
    }
  }
  
  Shortcut {
    sequence: "Ctrl+Y"
    onActivated: {
      const newAnnotations = historyManager.redo()
      if (newAnnotations.length > 0) {
        annotations = newAnnotations
        annotationCanvas.requestPaint()
      }
    }
  }
  
  Shortcut {
    sequence: "Delete"
    onActivated: {
      if (selectedAnnotationIndex >= 0 && selectedAnnotationIndex < annotations.length) {
        annotationCanvas.deleteSelectedAnnotation()
      }
    }
  }

  AnnotationManager {
    id: _annotationMgr
    onSaveCompleted: function (success, message) {
      console.log("Save completed:", success, message)
      root.saveStatus(success, message)
    }
  }

  OperationHistoryManager {
    id: historyManager
    onOperationCompleted: function(success, message) {
      console.log("History operation:", success, message)
    }
  }

  // 类别选择区域
  CategorySelector {
    id: categorySelector
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
      margins: 5
    }
    onCategoryChanged: function(category){
      annotationCanvas.currentCategory = category
    }
  }

  // 使用 ImageViewer 替代 Image
  ImageViewer {
    id: imageViewer
    anchors {
      top: categorySelector.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
      margins: 5
    }
    // 当图片变换时，更新标记
    onImageTransformChanged: {
      annotationCanvas.requestPaint()
    }
  }

  // 标注画布
  AnnotationCanvas {
    id: annotationCanvas
    anchors {
      fill: imageViewer
      // 确保不覆盖滚动条
      rightMargin: 10
      bottomMargin: 10
    }
    annotations: root.annotations
    categoryManager: categorySelector.categoryManager
    currentCategory: root.currentCategory
    // 添加 imageViewer 属性，用于坐标转换
    imageViewer: imageViewer
    
    // 初始化时更新鼠标状态
    Component.onCompleted: {
      // 确保初始状态正确
      isOverImage = imageViewer && imageViewer.imgSource !== ""
    }
    
    onAnnotationAdded: function(annotation) {
      // 记录添加操作
      historyManager.recordAdd(annotation, annotations);
    }
    
    onAnnotationModified: function(index, beforeAnnotations, afterAnnotations) {
      // 记录修改操作
      historyManager.recordModify(index, beforeAnnotations, afterAnnotations);
    }
    
    onAnnotationDeleted: function(index, annotation, annotations) {
      // 记录删除操作
      historyManager.recordDelete(index, annotation, annotations);
    }
  }
  
  // 类别图例
  CategoryLegend {
    id: categoryLegend
    anchors {
      top: categorySelector.bottom
      right: parent.right
      margins: 10
    }
    categoryManager: categorySelector.categoryManager
  }
  
  // 加载标注
  function loadAnnotations(filePath) {
    // 使用Qt的文件路径格式处理
    const localFilePath = filePath.toString().replace("file:///", "")
    
    // 使用AnnotationManager加载标注
    const annotationsData = _annotationMgr.loadAnnotations(localFilePath)
    if (annotationsData) {
      try {
        const data = JSON.parse(annotationsData)
        if (Array.isArray(data)) {
          // 确保每个标注都有对应的颜色
          for (let i = 0; i < data.length; i++) {
            const category = data[i].category
            // 如果类别不存在于当前类别列表中，添加它
            if (category && !categorySelector.categoryManager.categories.includes(category)) {
              // 生成随机颜色
              const randomColor = '#' + Math.floor(Math.random()*16777215).toString(16)
              // 使用C++类添加类别
              categorySelector.categoryManager.addCategory(category, randomColor)
            }
            
            // 确保标注使用相对坐标格式
            if (data[i].relX === undefined || data[i].relY === undefined || 
                data[i].relWidth === undefined || data[i].relHeight === undefined) {
              
              // 如果没有相对坐标，但有绝对坐标，则进行转换
              if (data[i].x !== undefined && data[i].y !== undefined && 
                  data[i].width !== undefined && data[i].height !== undefined) {
                
                console.log("转换标注 #" + i + " 从绝对坐标到相对坐标");
                
                // 使用绝对坐标创建相对坐标
                data[i].relX = data[i].x;
                data[i].relY = data[i].y;
                data[i].relWidth = data[i].width;
                data[i].relHeight = data[i].height;
                
                // 删除旧的绝对坐标
                delete data[i].x;
                delete data[i].y;
                delete data[i].width;
                delete data[i].height;
              } else {
                console.error("标注 #" + i + " 缺少有效的坐标");
              }
            }
            
            // 验证相对坐标是否有效
            if (data[i].relX === undefined || data[i].relY === undefined || 
                data[i].relWidth === undefined || data[i].relHeight === undefined ||
                isNaN(data[i].relX) || isNaN(data[i].relY) || 
                isNaN(data[i].relWidth) || isNaN(data[i].relHeight)) {
              console.error("标注 #" + i + " 有无效的相对坐标，将被跳过");
              data.splice(i, 1);
              i--;
              continue;
            }
            
            // 打印调试信息
            console.log("加载标注 #" + i + " - relX: " + data[i].relX + 
                        ", relY: " + data[i].relY + 
                        ", relWidth: " + data[i].relWidth + 
                        ", relHeight: " + data[i].relHeight);
          }
          
          annotations = data
          annotationCanvas.requestPaint()
          console.log("成功加载 " + annotations.length + " 个标注")
        } else {
          console.error("加载的JSON文件格式不正确")
          annotations = []
          annotationCanvas.requestPaint()
        }
      } catch (e) {
        console.error("解析JSON文件失败: " + e)
        annotations = []
        annotationCanvas.requestPaint()
      }
    } else {
      console.error("读取文件失败")
      annotations = []
      annotationCanvas.requestPaint()
    }
  }

  // 导入类别列表
  function importCategories(filePath) {
    categorySelector.importCategories(filePath)
    annotationCanvas.requestPaint()
  }
  
  // 导出类别列表
  function exportCategories(filePath) {
    categorySelector.exportCategories(filePath)
  }

  // 保存标注
  function saveAnnotations(filePath) {
    // 在保存前验证所有标注
    for (let i = 0; i < annotations.length; i++) {
      if (annotations[i].relX === undefined || annotations[i].relY === undefined || 
          annotations[i].relWidth === undefined || annotations[i].relHeight === undefined ||
          isNaN(annotations[i].relX) || isNaN(annotations[i].relY) || 
          isNaN(annotations[i].relWidth) || isNaN(annotations[i].relHeight)) {
        console.error("标注 #" + i + " 有无效的相对坐标，将被跳过");
        annotations.splice(i, 1);
        i--;
        continue;
      }
      
      // 打印每个标注的详细信息
      console.log("保存标注 #" + i + ":");
      console.log("  - 类别: " + annotations[i].category);
      console.log("  - relX: " + annotations[i].relX);
      console.log("  - relY: " + annotations[i].relY);
      console.log("  - relWidth: " + annotations[i].relWidth);
      console.log("  - relHeight: " + annotations[i].relHeight);
      
      // 确保标注只包含相对坐标，删除可能存在的绝对坐标
      delete annotations[i].x;
      delete annotations[i].y;
      delete annotations[i].width;
      delete annotations[i].height;
    }
    
    // 使用 AnnotationManager 保存标注
    var success = _annotationMgr.saveAnnotationToFile(filePath, annotations)
    if (success) {
      console.log("Save request sent to backend, path:", filePath)
    } else {
      console.log("Save Failed!")
    }
  }

  // 清除标注
  function clearAnnotations() {
    // 记录清除操作（如果有标注的话）
    if (annotations.length > 0) {
      historyManager.recordDelete(-1, {}, annotations);
    }
    
    annotations = []
    annotationCanvas.selectedAnnotationIndex = -1
    annotationCanvas.requestPaint()
  }
}



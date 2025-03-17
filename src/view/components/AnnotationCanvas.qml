import QtQuick
import QtQuick.Controls
import OGPRAnnotator

// 标注画布组件
Canvas {
  id: root
  
  property var annotations: []
  property bool isDrawing: false
  property point startPoint
  property point currentPoint
  property string currentCategory: ""
  property int selectedAnnotationIndex: -1
  property bool isMovingAnnotation: false
  property bool isPressHolding: false
  property bool isPointInImageArea: true
  property point lastMousePos
  property var originalAnnotation: null
  property var categoryManager: null
  // 添加 imageViewer 属性，用于坐标转换
  property var imageViewer: null
  // 添加属性，暴露鼠标是否在图片上的状态
  property bool isOverImage: annotationMouseArea ? annotationMouseArea.isOverImage : false
  
  // 信号
  signal annotationAdded(var annotation)
  signal annotationModified(int index, var beforeAnnotations, var afterAnnotations)
  signal annotationDeleted(int index, var annotation, var annotations)
  
  // 调试函数：打印标注信息
  function debugAnnotation(annotation, prefix) {
    console.log(prefix + " - relX: " + annotation.relX + 
                ", relY: " + annotation.relY + 
                ", relWidth: " + annotation.relWidth + 
                ", relHeight: " + annotation.relHeight);
  }
  
  onPaint: {
    var ctx = getContext("2d")
    ctx.clearRect(0, 0, width, height)
    ctx.lineWidth = 2

    // 绘制已保存的标注
    for (var i = 0; i < annotations.length; i++) {
      var box = annotations[i]
      const categoryColor = categoryManager.getCategoryColor(box.category)
      
      // 确保标注有有效的相对坐标
      if (box.relX === undefined || box.relY === undefined || 
          box.relWidth === undefined || box.relHeight === undefined ||
          isNaN(box.relX) || isNaN(box.relY) || 
          isNaN(box.relWidth) || isNaN(box.relHeight)) {
        console.error("Invalid annotation coordinates:", box);
        continue;
      }
      
      // 将相对坐标（百分比）转换为屏幕坐标
      var screenBox = imageViewer.relativeToScreen(box.relX, box.relY, box.relWidth, box.relHeight)
      
      // 如果是选中的标注，使用更粗的线条和特殊效果
      if (i === selectedAnnotationIndex) {
        if (isMovingAnnotation) {
          // 移动状态 - 使用虚线边框和更明显的高亮
          ctx.lineWidth = 5
          ctx.strokeStyle = "#FF5722" // 红橙色高亮
          ctx.setLineDash([5, 3]); // 设置虚线样式
          
          // 绘制半透明背景表示移动状态
          ctx.fillStyle = "rgba(255, 87, 34, 0.2)";
          ctx.fillRect(screenBox.x, screenBox.y, screenBox.width, screenBox.height);
        } else {
          // 选中状态 - 使用实线粗边框
          ctx.lineWidth = 4
          ctx.strokeStyle = "#FF9800" // 橙色高亮
          ctx.setLineDash([]); // 实线
        }
      } else {
        ctx.lineWidth = 2
        ctx.strokeStyle = categoryColor
        ctx.setLineDash([]); // 实线
      }
      
      ctx.strokeRect(screenBox.x, screenBox.y, screenBox.width, screenBox.height)
      ctx.setLineDash([]); // 重置为实线
      
      // 绘制类别标签
      ctx.fillStyle = categoryColor
      ctx.font = "12px sans-serif"
      ctx.fillText(box.category, screenBox.x, screenBox.y - 5)
    }
    
    // 恢复默认线宽
    ctx.lineWidth = 2

    // 绘制当前正在绘制的标注
    if (isDrawing) {
      ctx.strokeStyle = categoryManager.getCategoryColor(currentCategory)
      var currentWidth = currentPoint.x - startPoint.x
      var currentHeight = currentPoint.y - startPoint.y
      ctx.strokeRect(startPoint.x, startPoint.y, currentWidth, currentHeight)
    }
  }
  
  // 检查点是否在矩形内
  function isPointInRect(px, py, rx, ry, rw, rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
  }
  
  // 查找点击位置的标注
  function findAnnotationAt(x, y) {
    for (let i = annotations.length - 1; i >= 0; i--) {
      const ann = annotations[i];
      // 确保标注有有效的相对坐标
      if (ann.relX === undefined || ann.relY === undefined || 
          ann.relWidth === undefined || ann.relHeight === undefined) {
        continue;
      }
      
      // 将相对坐标（百分比）转换为屏幕坐标
      var screenBox = imageViewer.relativeToScreen(ann.relX, ann.relY, ann.relWidth, ann.relHeight);
      if (isPointInRect(x, y, screenBox.x, screenBox.y, screenBox.width, screenBox.height)) {
        return i;
      }
    }
    return -1;
  }
  
  // 删除选中标注的函数
  function deleteSelectedAnnotation() {
    if (selectedAnnotationIndex >= 0 && selectedAnnotationIndex < annotations.length) {
      // 发出删除信号
      annotationDeleted(selectedAnnotationIndex, annotations[selectedAnnotationIndex], annotations);
      
      // 从列表中删除
      annotations.splice(selectedAnnotationIndex, 1);
      
      // 重置选中索引
      selectedAnnotationIndex = -1;
      
      // 重绘画布
      requestPaint();
    }
  }
  
  // 长按定时器
  Timer {
    id: pressHoldTimer
    interval: 500 // 长按时间阈值，500毫秒
    repeat: false
    onTriggered: {
      if (selectedAnnotationIndex >= 0) {
        isPressHolding = true
        isMovingAnnotation = true
        // 保存原始标注的副本，用于撤销
        originalAnnotation = JSON.parse(JSON.stringify(annotations[selectedAnnotationIndex]))
        console.log("长按开始移动标注")
        // 显示移动提示
        moveHintText.text = "移动模式 - 拖动鼠标移动标注"
        moveHintText.color = "#FF5722"
        moveHintText.visible = true
      }
    }
  }
  
  // 移动提示隐藏定时器
  Timer {
    id: moveHintTimer
    interval: 2000 // 2秒后隐藏提示
    repeat: false
    onTriggered: {
      moveHintText.visible = false
    }
  }
  
  // 添加移动提示文本
  Text {
    id: moveHintText
    anchors {
      horizontalCenter: parent.horizontalCenter
      top: parent.top
      topMargin: 10
    }
    text: "移动模式 - 拖动鼠标移动标注"
    color: "#FF5722"
    font.pixelSize: 16
    font.bold: true
    visible: false
    z: 10 // 确保在其他元素上方显示
  }

  // 添加函数：检查点是否在图片区域内
  function _isPointInImageArea(x, y) {
    if (!imageViewer) return false;
    
    // 获取图片在屏幕上的位置和尺寸
    var imgX = -imageViewer.imageX;
    var imgY = -imageViewer.imageY;
    var imgWidth = imageViewer.imageWidth;
    var imgHeight = imageViewer.imageHeight;
    
    // 检查点是否在图片区域内
    return (x >= imgX && x <= imgX + imgWidth && 
            y >= imgY && y <= imgY + imgHeight);
  }
  
  // 添加函数：将点限制在图片区域内
  function constrainPointToImageArea(point) {
    if (!imageViewer) return point;
    
    // 获取图片在屏幕上的位置和尺寸
    var imgX = -imageViewer.imageX;
    var imgY = -imageViewer.imageY;
    var imgWidth = imageViewer.imageWidth;
    var imgHeight = imageViewer.imageHeight;
    
    // 限制点在图片区域内
    var x = Math.max(imgX, Math.min(point.x, imgX + imgWidth));
    var y = Math.max(imgY, Math.min(point.y, imgY + imgHeight));
    
    return Qt.point(x, y);
  }

  MouseArea {
    id: annotationMouseArea
    anchors.fill: parent
    hoverEnabled: true
    acceptedButtons: Qt.LeftButton | Qt.RightButton
    propagateComposedEvents: true  // 允许事件传播
    z: 2  // 确保在其他元素上方
    
    // 鼠标状态管理
    property bool isOverImage: false
    property bool isOverAnnotation: false
    property bool isOverScrollBar: false
    
    // 设置默认光标为箭头
    cursorShape: {
      // 如果在滚动条上，显示默认光标
      if (isOverScrollBar) {
        return Qt.ArrowCursor;
      }
      
      // 如果不在图片区域内，显示箭头光标
      if (!isOverImage) {
        return Qt.ArrowCursor;
      }
      
      // 如果正在移动标注，显示移动光标
      if (isMovingAnnotation) {
        return Qt.SizeAllCursor;
      }
      
      // 如果鼠标悬停在标注上，显示手型光标
      if (isOverAnnotation) {
        return Qt.PointingHandCursor;
      }
      
      // 默认在图片区域内显示十字光标（用于标注）
      return Qt.CrossCursor;
    }
    
    // 检查是否在滚动条区域
    function checkIfOverScrollBar(x, y) {
      if (!imageViewer) return false;
      
      // 获取滚动条的位置和尺寸
      var vScrollBarX = imageViewer.width - 10; // 假设垂直滚动条宽度为10
      var hScrollBarY = imageViewer.height - 10; // 假设水平滚动条高度为10
      
      // 检查是否在垂直滚动条区域
      if (x > vScrollBarX && x <= imageViewer.width) {
        return true;
      }
      
      // 检查是否在水平滚动条区域
      if (y > hScrollBarY && y <= imageViewer.height) {
        return true;
      }
      
      return false;
    }

    onPressed: function(mouse) {
      if (parent.width === 0 || parent.height === 0) return
      
      // 检查是否在滚动条区域
      isOverScrollBar = checkIfOverScrollBar(mouseX, mouseY);
      if (isOverScrollBar) {
        mouse.accepted = false;
        return;
      }
      
      // 检查点击是否在图片区域内
      if (!isOverImage) {
        console.log("点击位置在图片区域外，忽略操作");
        mouse.accepted = false;  // 允许事件传递
        return;
      }
      
      // 记录鼠标按下位置
      lastMousePos = Qt.point(mouseX, mouseY)
      
      if (mouse.button === Qt.LeftButton) {
        // 检查是否点击了已有标注
        const clickedIndex = findAnnotationAt(mouseX, mouseY);
        
        if (clickedIndex >= 0) {
          // 选中标注
          selectedAnnotationIndex = clickedIndex;
          isOverAnnotation = true;
          requestPaint();
          
          // 启动长按定时器
          pressHoldTimer.start();
        } else {
          // 开始绘制新标注
          isDrawing = true;
          startPoint = Qt.point(mouseX, mouseY);
          currentPoint = startPoint;
          
          // 取消选中
          selectedAnnotationIndex = -1;
          isOverAnnotation = false;
        }
      } else if (mouse.button === Qt.RightButton) {
        // 右键点击检查是否点击了标注
        const clickedIndex = findAnnotationAt(mouseX, mouseY);
        
        if (clickedIndex >= 0) {
          // 选中标注
          selectedAnnotationIndex = clickedIndex;
          isOverAnnotation = true;
          requestPaint();
          
          // 显示上下文菜单
          contextMenu.popup();
        }
      }
    }

    onPositionChanged: function(mouse) {
      // 检查是否在滚动条区域
      isOverScrollBar = checkIfOverScrollBar(mouseX, mouseY);
      if (isOverScrollBar) {
        mouse.accepted = false;
        return;
      }
      
      // 更新鼠标是否在图片区域内
      isOverImage = _isPointInImageArea(mouseX, mouseY);
      
      // 更新鼠标是否在标注上
      if (!isDrawing && !isMovingAnnotation) {
        isOverAnnotation = findAnnotationAt(mouseX, mouseY) >= 0;
      }
      
      if (isDrawing) {
        // 正在绘制新标注，限制点在图片区域内
        currentPoint = constrainPointToImageArea(Qt.point(mouseX, mouseY));
        requestPaint()
      } else if (isMovingAnnotation && selectedAnnotationIndex >= 0) {
        // 限制鼠标位置在图片区域内
        var constrainedPoint = constrainPointToImageArea(Qt.point(mouseX, mouseY));
        const dx = constrainedPoint.x - lastMousePos.x
        const dy = constrainedPoint.y - lastMousePos.y
        
        // 获取当前标注的屏幕坐标
        var screenBox = imageViewer.relativeToScreen(
          annotations[selectedAnnotationIndex].relX,
          annotations[selectedAnnotationIndex].relY,
          annotations[selectedAnnotationIndex].relWidth,
          annotations[selectedAnnotationIndex].relHeight
        )
        
        // 更新屏幕坐标
        screenBox.x += dx
        screenBox.y += dy
        
        // 将屏幕坐标转换回相对坐标（百分比）
        var relBox = imageViewer.screenToRelative(
          screenBox.x,
          screenBox.y,
          screenBox.width,
          screenBox.height
        )
        
        // 更新标注的相对坐标
        annotations[selectedAnnotationIndex].relX = relBox.x
        annotations[selectedAnnotationIndex].relY = relBox.y
        
        // 更新鼠标位置
        lastMousePos = constrainedPoint
        
        // 在提示文本中显示位置信息
        moveHintText.text = "移动模式 - 位置: (" + 
                          Math.round(screenBox.x) + ", " + 
                          Math.round(screenBox.y) + ")"
        
        // 重绘画布
        requestPaint()
      }
    }

    onReleased: function(mouse) {
      // 检查是否在滚动条区域
      if (isOverScrollBar) {
        mouse.accepted = false;
        return;
      }
      
      // 停止长按定时器
      pressHoldTimer.stop()
      
      if (isDrawing) {
        // 完成绘制新标注
        isDrawing = false

        // 确保点在图片区域内
        var constrainedStartPoint = constrainPointToImageArea(startPoint);
        var constrainedCurrentPoint = constrainPointToImageArea(currentPoint);

        var x = Math.min(constrainedStartPoint.x, constrainedCurrentPoint.x)
        var y = Math.min(constrainedStartPoint.y, constrainedCurrentPoint.y)
        var width = Math.abs(constrainedCurrentPoint.x - constrainedStartPoint.x)
        var height = Math.abs(constrainedCurrentPoint.y - constrainedStartPoint.y)

        // 只有当标注尺寸足够大且在图片区域内时才创建
        if (width > 5 && height > 5) {
          // 将屏幕坐标转换为相对坐标（百分比）
          var relBox = imageViewer.screenToRelative(x, y, width, height)
          
          // 确保相对坐标在 0-1 范围内
          relBox.x = Math.max(0, Math.min(1, relBox.x));
          relBox.y = Math.max(0, Math.min(1, relBox.y));
          
          // 确保标注不会超出图片边界
          if (relBox.x + relBox.width > 1) relBox.width = 1 - relBox.x;
          if (relBox.y + relBox.height > 1) relBox.height = 1 - relBox.y;
          
          // 创建新标注，使用相对坐标
          const newAnnotation = {
            "relX": relBox.x,
            "relY": relBox.y,
            "relWidth": relBox.width,
            "relHeight": relBox.height,
            "category": currentCategory
          };
          
          // 调试输出
          debugAnnotation(newAnnotation, "新建标注");
          
          // 检查标注对象的属性
          console.log("新标注对象的属性:");
          for (var prop in newAnnotation) {
            console.log("  - " + prop + ": " + newAnnotation[prop]);
          }
          
          // 发出添加信号
          annotationAdded(newAnnotation);
          
          // 添加到标注列表
          annotations.push(newAnnotation);
        }
      } else if (isMovingAnnotation && selectedAnnotationIndex >= 0) {
        // 完成移动标注
        // 创建一个临时的标注列表副本，用于记录修改前的状态
        const beforeAnnotations = JSON.parse(JSON.stringify(annotations))
        // 将修改前的标注放回去，以记录修改前的状态
        beforeAnnotations[selectedAnnotationIndex] = originalAnnotation
        
        // 调试输出
        debugAnnotation(annotations[selectedAnnotationIndex], "移动后标注");
        
        // 发出修改信号
        annotationModified(selectedAnnotationIndex, beforeAnnotations, annotations)
        
        console.log("完成移动标注")
        
        // 显示移动完成提示
        moveHintText.text = "标注已移动"
        moveHintText.color = "#4CAF50" // 绿色
        
        // 设置定时器，2秒后隐藏提示
        moveHintTimer.start()
        
        // 只重置移动状态，保持选中状态
        isMovingAnnotation = false
        isPressHolding = false
        originalAnnotation = null
      } else {
        // 其他情况下重置所有状态
        isMovingAnnotation = false
        isPressHolding = false
        originalAnnotation = null
        moveHintText.visible = false
      }
      
      // 更新鼠标是否在标注上
      isOverAnnotation = findAnnotationAt(mouseX, mouseY) >= 0;
      
      requestPaint()
    }
    
    // 当鼠标进入区域时
    onEntered: {
      isOverScrollBar = checkIfOverScrollBar(mouseX, mouseY);
      isOverImage = _isPointInImageArea(mouseX, mouseY);
      isOverAnnotation = findAnnotationAt(mouseX, mouseY) >= 0;
    }
    
    // 当鼠标离开区域时
    onExited: {
      isOverScrollBar = false;
      isOverImage = false;
      isOverAnnotation = false;
    }
    
    // 处理滚轮事件
    onWheel: function(wheel) {
      // 检查是否在滚动条区域
      if (checkIfOverScrollBar(wheel.x, wheel.y)) {
        wheel.accepted = false;
        return;
      }
      
      // 将滚轮事件传递给 ImageViewer 进行图片缩放
      if (imageViewer) {
        imageViewer.zoom(wheel.x, wheel.y, wheel.angleDelta.y > 0);
      }
      
      // 不接受事件，允许继续传播
      wheel.accepted = false;
    }
  }
  
  // 上下文菜单
  Menu {
    id: contextMenu
    
    MenuItem {
      text: "删除标注"
      onTriggered: deleteSelectedAnnotation()
    }
  }
}

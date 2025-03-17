import QtQuick
import QtQuick.Controls

Item {
    id: root
    property alias imgSource: oriImg.source
    // 添加属性用于存储图片的实际尺寸和位置信息
    property real imageScale: 1.0
    property real imageX: 0
    property real imageY: 0
    property real imageWidth: oriImg.width
    property real imageHeight: oriImg.height
    // 添加信号，当图片缩放或平移时发出
    signal imageTransformChanged()
    // 添加属性表示图片是否加载成功
    property bool imageLoaded: oriImg.status === Image.Ready

    function zoomToFit() {
        // 如果图片未加载成功，则不执行缩放
        if (oriImg.status !== Image.Ready) {
            return
        }
        
        // 计算宽高比缩放
        var widthRatio = flickable.width / oriImg.sourceSize.width
        var heightRatio = flickable.height / oriImg.sourceSize.height
        var scale = Math.min(widthRatio, heightRatio)
        if (!scale) {
            return
        }

        // 选择较小的缩放因子以保持图片完整显示
        oriImg.width = oriImg.sourceSize.width * scale
        oriImg.height = oriImg.sourceSize.height * scale

        // 将图片定位在中心
        flickable.contentX = (oriImg.width - flickable.width) / 2
        flickable.contentY = (oriImg.height - flickable.height) / 2
        
        // 更新图片变换信息
        updateImageTransform()
    }

    function zoom(x, y, zoomIn) {
        // 确保坐标相对于 flickable
        var flickableX = x;
        var flickableY = y;
        
        // 检查坐标是否在 flickable 范围内
        var isMouseInImage = flickableX >= 0 && flickableX <= flickable.width &&
                             flickableY >= 0 && flickableY <= flickable.height &&
                             flickableX + flickable.contentX >= 0 &&
                             flickableX + flickable.contentX <= oriImg.width &&
                             flickableY + flickable.contentY >= 0 &&
                             flickableY + flickable.contentY <= oriImg.height;

        var scaleFactor = 1.1
        var scale

        if (zoomIn) {
            // 放大图片
            scale = scaleFactor
        } else {
            // 缩小图片
            scale = 1 / scaleFactor
        }

        var relativeX, relativeY

        if (isMouseInImage) {
            relativeX = (flickableX + flickable.contentX) / oriImg.width
            relativeY = (flickableY + flickable.contentY) / oriImg.height
        } else {
            relativeX = 0.5
            relativeY = 0.5
        }

        oriImg.width *= scale
        oriImg.height *= scale

        flickable.contentX = relativeX * oriImg.width - (isMouseInImage ? flickableX : flickable.width / 2)
        flickable.contentY = relativeY * oriImg.height - (isMouseInImage ? flickableY : flickable.height / 2)

        if (flickable.contentX < 0) {
            flickable.contentX = 0
        } else if (flickable.contentX > oriImg.width - flickable.width) {
            flickable.contentX = Math.max(oriImg.width - flickable.width, 0)
        }

        if (flickable.contentY < 0) {
            flickable.contentY = 0
        } else if (flickable.contentY > oriImg.height - flickable.height) {
            flickable.contentY = Math.max(oriImg.height - flickable.height, 0)
        }

        if (oriImg.width < flickable.width) {
            flickable.contentX = (oriImg.width - flickable.width) / 2
        }
        if (oriImg.height < flickable.height) {
            flickable.contentY = (oriImg.height - flickable.height) / 2
        }
        
        // 更新图片变换信息
        updateImageTransform()
        
        // 在缩放后请求重绘标注
        imageTransformChanged()
    }
    
    // 更新图片变换信息
    function updateImageTransform() {
        imageScale = oriImg.width / oriImg.sourceSize.width
        imageX = flickable.contentX
        imageY = flickable.contentY
        imageWidth = oriImg.width
        imageHeight = oriImg.height
        // 发出信号通知图片变换已更改
        imageTransformChanged()
    }
    
    // 修复：将相对坐标（百分比）转换为屏幕坐标
    function relativeToScreen(relX, relY, relWidth, relHeight) {
        // 相对坐标是相对于原始图片尺寸的百分比
        // 需要将其转换为当前缩放和平移后的屏幕坐标
        
        // 计算在当前缩放下的实际像素位置
        var actualX = relX * oriImg.width
        var actualY = relY * oriImg.height
        var actualWidth = relWidth * oriImg.width
        var actualHeight = relHeight * oriImg.height
        
        // 考虑平移，计算屏幕坐标
        var screenX = actualX - flickable.contentX
        var screenY = actualY - flickable.contentY
        
        return { 
            x: screenX, 
            y: screenY, 
            width: actualWidth, 
            height: actualHeight 
        }
    }
    
    // 修复：将屏幕坐标转换为相对坐标（百分比）
    function screenToRelative(screenX, screenY, screenWidth, screenHeight) {
        // 屏幕坐标需要考虑当前的平移
        // 先转换为图片上的实际像素位置
        var actualX = screenX + flickable.contentX
        var actualY = screenY + flickable.contentY
        
        // 然后计算相对于图片尺寸的百分比
        var relX = actualX / oriImg.width
        var relY = actualY / oriImg.height
        var relWidth = screenWidth / oriImg.width
        var relHeight = screenHeight / oriImg.height
        
        return { 
            x: relX, 
            y: relY, 
            width: relWidth, 
            height: relHeight 
        }
    }

    ScrollBar {
        id: vScrollBar
        policy: flickable.contentWidth > flickable.width ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
        contentItem: Rectangle {
            implicitWidth: 6
            color: "gray"
            radius: 3
        }
        height: parent.height
        anchors.right: parent.right
        z: 10 // 增加 z 值，确保滚动条在最上层
    }
    ScrollBar {
        id: hScrollBar
        policy: flickable.contentWidth > flickable.width ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
        contentItem: Rectangle {
            implicitHeight: 6
            color: "gray"
            radius: 3
        }
        width: parent.width
        anchors.bottom: parent.bottom
        z: 10 // 增加 z 值，确保滚动条在最上层
    }
    Item {
        anchors {
            left: parent.left
            right: vScrollBar.left
            top: parent.top
            bottom: hScrollBar.top
        }

        Flickable {
            id: flickable
            anchors.fill: parent
            contentWidth: oriImg.width
            contentHeight: oriImg.height
            // interactive: true  // 启用交互，允许直接拖动图片
            clip: true
            
            // 添加监听器，当内容位置改变时更新图片变换信息
            onContentXChanged: updateImageTransform()
            onContentYChanged: updateImageTransform()

            Rectangle {
                anchors.fill: oriImg
                color: "#00FF00"
            }

            Image {
                id: oriImg
                asynchronous: true // 异步加载图片
                fillMode: Image.PreserveAspectFit
                
                // 添加图片加载状态处理
                onStatusChanged: {
                    if (status === Image.Ready) {
                        console.log("Image loaded successfully: " + source)
                        root.zoomToFit()
                    } else if (status === Image.Error) {
                        console.error("Error loading image: " + source)
                    } else if (status === Image.Loading) {
                        console.log("Loading image: " + source)
                    }
                }
                
                Component.onCompleted: {
                    console.log("Image component completed, source: " + source)
                }
                
                onSourceChanged: {
                    console.log("Image source changed to: " + source)
                }
                
                onSourceSizeChanged: {
                    if (status === Image.Ready) {
                        root.zoomToFit()
                    }
                }
                // 添加监听器，当图片尺寸改变时更新图片变换信息
                onWidthChanged: updateImageTransform()
                onHeightChanged: updateImageTransform()
            }
            
            ScrollBar.vertical: vScrollBar
            ScrollBar.horizontal: hScrollBar
        }
        
        // 滚轮事件处理
        MouseArea {
            id: wheelHandler
            anchors.fill: parent
            acceptedButtons: Qt.NoButton  // 不接受任何鼠标按钮事件
            propagateComposedEvents: true // 允许事件传播
            
            onWheel: function(wheel) {
                root.zoom(wheel.x, wheel.y, wheel.angleDelta.y > 0)
                wheel.accepted = true
            }
        }
    }
}

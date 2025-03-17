/******************************************************************************
* @File: SignalBus.h
* @Author: buf
* @Created: 2025/3/17 13:10
* @Version: 1.0
* @Description: 
******************************************************************************/

#ifndef SIGNALBUS_H
#define SIGNALBUS_H
#include <QQmlEngine>
/**
设置为单例模式
*/
class SignalBus : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    // C++ 中获取单例的静态方法
    static SignalBus *instance();

    // QML 引擎调用的创建函数
    static SignalBus *create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);

Q_SIGNALS:
    void exportBScanImage(const QString &); // 导出图片 参数: 文件路径

private:
    static SignalBus *s_instance; // 静态实例指针
    SignalBus() = default;
    SignalBus(const SignalBus &) = delete;
    SignalBus &operator=(const SignalBus &) = delete;
};
#endif //SIGNALBUS_H
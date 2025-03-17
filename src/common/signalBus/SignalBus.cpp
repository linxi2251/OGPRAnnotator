#include "SignalBus.h"

// 初始化静态成员变量
SignalBus *SignalBus::s_instance = nullptr;

// C++ 获取单例实例
SignalBus *SignalBus::instance()
{
    if (!s_instance) {
        s_instance = new SignalBus();
    }
    return s_instance;
}

// QML 引擎创建单例的回调
SignalBus *SignalBus::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)

    if (!s_instance) {
        s_instance = new SignalBus();
    }
    return s_instance;
}

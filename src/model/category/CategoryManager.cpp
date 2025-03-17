#include "categorymanager.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QDir>
#include <QStandardPaths>

CategoryManager::CategoryManager(QObject *parent)
    : QObject(parent)
{
    // 初始化默认类别，但不保存到文件
    // 文件将在设置图片文件夹路径时加载或创建
    initDefaultCategories();
}

QStringList CategoryManager::categories() const
{
    return m_categories;
}

QColor CategoryManager::getCategoryColor(const QString &category) const
{
    if (m_categoryColors.contains(category)) {
        return m_categoryColors[category];
    }
    // 如果类别不存在，返回默认颜色（黑色）
    return QColor("#000000");
}

bool CategoryManager::addCategory(const QString &category, const QColor &color)
{
    // 检查类别是否已存在
    if (m_categories.contains(category)) {
        emit operationCompleted(false, QString("类别 '%1' 已存在").arg(category));
        return false;
    }
    
    // 添加新类别
    m_categories.append(category);
    m_categoryColors[category] = color;
    
    // 保存到文件
    saveCategoriesToFile(m_categoriesFilePath);
    
    // 发送信号通知QML界面更新
    emit categoriesChanged();
    emit categoryColorsChanged();
    emit operationCompleted(true, QString("成功添加类别 '%1'").arg(category));
    
    return true;
}

bool CategoryManager::removeCategory(const QString &category)
{
    // 检查类别是否存在
    if (!m_categories.contains(category)) {
        emit operationCompleted(false, QString("类别 '%1' 不存在").arg(category));
        return false;
    }
    
    // 移除类别
    m_categories.removeOne(category);
    m_categoryColors.remove(category);
    
    // 保存到文件
    saveCategoriesToFile(m_categoriesFilePath);
    
    // 发送信号通知QML界面更新
    emit categoriesChanged();
    emit categoryColorsChanged();
    emit operationCompleted(true, QString("成功删除类别 '%1'").arg(category));
    
    return true;
}

bool CategoryManager::importCategories(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        emit operationCompleted(false, QString("文件 '%1' 不存在").arg(filePath));
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        emit operationCompleted(false, QString("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    // 解析JSON数据
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(jsonData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        emit operationCompleted(false, QString("JSON解析错误: %1").arg(parseError.errorString()));
        return false;
    }
    
    QJsonObject rootObj = document.object();
    
    // 检查JSON格式是否正确
    if (!rootObj.contains("categories") || !rootObj["categories"].isArray()) {
        emit operationCompleted(false, "JSON格式不正确，缺少categories数组");
        return false;
    }
    
    // 清除现有类别
    m_categories.clear();
    m_categoryColors.clear();
    
    // 导入新类别
    QJsonArray categoriesArray = rootObj["categories"].toArray();
    for (const QJsonValue &value : categoriesArray) {
        if (value.isString()) {
            m_categories.append(value.toString());
        }
    }
    
    // 导入颜色
    if (rootObj.contains("colors") && rootObj["colors"].isObject()) {
        QJsonObject colorsObj = rootObj["colors"].toObject();
        for (const QString &category : m_categories) {
            if (colorsObj.contains(category)) {
                QString colorStr = colorsObj[category].toString();
                m_categoryColors[category] = QColor(colorStr);
            } else {
                // 如果没有对应的颜色，生成随机颜色
                m_categoryColors[category] = QColor(
                    QRandomGenerator::global()->bounded(256),
                    QRandomGenerator::global()->bounded(256),
                    QRandomGenerator::global()->bounded(256));
            }
        }
    } else {
        // 如果没有颜色信息，为每个类别生成随机颜色
        for (const QString &category : m_categories) {
            m_categoryColors[category] = QColor(
                QRandomGenerator::global()->bounded(256),
                QRandomGenerator::global()->bounded(256),
                QRandomGenerator::global()->bounded(256));
        }
    }
    
    // 保存到默认类别文件
    saveCategoriesToFile(m_categoriesFilePath);
    
    // 发送信号通知QML界面更新
    emit categoriesChanged();
    emit categoryColorsChanged();
    emit operationCompleted(true, QString("成功导入 %1 个类别").arg(m_categories.size()));
    
    return true;
}

bool CategoryManager::exportCategories(const QString &filePath)
{
    QJsonObject rootObj;
    
    // 添加类别数组
    QJsonArray categoriesArray;
    for (const QString &category : m_categories) {
        categoriesArray.append(category);
    }
    rootObj["categories"] = categoriesArray;
    
    // 添加颜色映射
    QJsonObject colorsObj;
    for (auto it = m_categoryColors.begin(); it != m_categoryColors.end(); ++it) {
        colorsObj[it.key()] = it.value().name();
    }
    rootObj["colors"] = colorsObj;
    
    // 创建JSON文档
    QJsonDocument document(rootObj);
    
    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit operationCompleted(false, QString("无法打开文件: %1").arg(file.errorString()));
        return false;
    }
    
    file.write(document.toJson(QJsonDocument::Indented));
    file.close();
    
    emit operationCompleted(true, QString("成功导出 %1 个类别到 %2").arg(m_categories.size()).arg(filePath));
    return true;
}

QVariantMap CategoryManager::getCategoryColors() const
{
    QVariantMap result;
    for (auto it = m_categoryColors.begin(); it != m_categoryColors.end(); ++it) {
        result[it.key()] = it.value().name();
    }
    return result;
}

QString CategoryManager::categoriesFilePath() const
{
    return m_categoriesFilePath;
}

void CategoryManager::setCategoriesFilePath(const QString &filePath)
{
    if (m_categoriesFilePath != filePath) {
        m_categoriesFilePath = filePath;
        emit categoriesFilePathChanged();
        
        // 尝试从新路径加载类别
        if (!loadCategoriesFromFile(m_categoriesFilePath)) {
            // 如果加载失败，保存当前类别到文件
            saveCategoriesToFile(m_categoriesFilePath);
        }
    }
}

bool CategoryManager::loadCategoriesFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << "类别文件不存在:" << filePath;
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开类别文件:" << file.errorString();
        return false;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    // 解析JSON数据
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(jsonData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误:" << parseError.errorString();
        return false;
    }
    
    QJsonObject rootObj = document.object();
    
    // 检查JSON格式是否正确
    if (!rootObj.contains("categories") || !rootObj["categories"].isArray()) {
        qDebug() << "JSON格式不正确，缺少categories数组";
        return false;
    }
    
    // 清除现有类别
    m_categories.clear();
    m_categoryColors.clear();
    
    // 导入新类别
    QJsonArray categoriesArray = rootObj["categories"].toArray();
    for (const QJsonValue &value : categoriesArray) {
        if (value.isString()) {
            m_categories.append(value.toString());
        }
    }
    
    // 导入颜色
    if (rootObj.contains("colors") && rootObj["colors"].isObject()) {
        QJsonObject colorsObj = rootObj["colors"].toObject();
        for (const QString &category : m_categories) {
            if (colorsObj.contains(category)) {
                QString colorStr = colorsObj[category].toString();
                m_categoryColors[category] = QColor(colorStr);
            } else {
                // 如果没有对应的颜色，生成随机颜色
                m_categoryColors[category] = QColor(
                    QRandomGenerator::global()->bounded(256),
                    QRandomGenerator::global()->bounded(256),
                    QRandomGenerator::global()->bounded(256));
            }
        }
    } else {
        // 如果没有颜色信息，为每个类别生成随机颜色
        for (const QString &category : m_categories) {
            m_categoryColors[category] = QColor(
                QRandomGenerator::global()->bounded(256),
                QRandomGenerator::global()->bounded(256),
                QRandomGenerator::global()->bounded(256));
        }
    }
    
    // 发送信号通知QML界面更新
    emit categoriesChanged();
    emit categoryColorsChanged();
    
    return true;
}

bool CategoryManager::saveCategoriesToFile(const QString &filePath)
{
    QJsonObject rootObj;
    
    // 添加类别数组
    QJsonArray categoriesArray;
    for (const QString &category : m_categories) {
        categoriesArray.append(category);
    }
    rootObj["categories"] = categoriesArray;
    
    // 添加颜色映射
    QJsonObject colorsObj;
    for (auto it = m_categoryColors.begin(); it != m_categoryColors.end(); ++it) {
        colorsObj[it.key()] = it.value().name();
    }
    rootObj["colors"] = colorsObj;
    
    // 创建JSON文档
    QJsonDocument document(rootObj);
    
    // 写入文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "无法打开文件保存类别:" << file.errorString();
        return false;
    }
    
    file.write(document.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

void CategoryManager::initDefaultCategories()
{
    // 添加默认类别
    m_categories.clear();
    m_categoryColors.clear();
    
    m_categories << "人" << "车" << "动物" << "其他";
    
    // 设置默认颜色
    m_categoryColors["人"] = QColor("#ff0000");      // 红色
    m_categoryColors["车"] = QColor("#00ff00");      // 绿色
    m_categoryColors["动物"] = QColor("#0000ff");    // 蓝色
    m_categoryColors["其他"] = QColor("#ff00ff");    // 紫色
}
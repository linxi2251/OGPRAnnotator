#ifndef CATEGORYMANAGER_H
#define CATEGORYMANAGER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QColor>
#include <QMap>
#include <QStringList>
#include <QVariant>
#include <qqmlintegration.h>

class CategoryManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QStringList categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(QString categoriesFilePath READ categoriesFilePath WRITE setCategoriesFilePath NOTIFY categoriesFilePathChanged)

public:
    explicit CategoryManager(QObject *parent = nullptr);

    // 获取类别列表
    QStringList categories() const;

    // 获取类别颜色
    Q_INVOKABLE QColor getCategoryColor(const QString &category) const;

    // 添加新类别
    Q_INVOKABLE bool addCategory(const QString &category, const QColor &color);

    // 删除类别
    Q_INVOKABLE bool removeCategory(const QString &category);

    // 导入类别列表
    Q_INVOKABLE bool importCategories(const QString &filePath);

    // 导出类别列表
    Q_INVOKABLE bool exportCategories(const QString &filePath);

    // 获取所有类别和颜色的映射
    Q_INVOKABLE QVariantMap getCategoryColors() const;

    // 获取类别文件路径
    QString categoriesFilePath() const;
    
    // 设置类别文件路径
    void setCategoriesFilePath(const QString &filePath);

signals:
    void categoriesChanged();
    void categoryColorsChanged();
    void operationCompleted(bool success, const QString &message);
    void categoriesFilePathChanged();

private:
    QStringList m_categories;
    QMap<QString, QColor> m_categoryColors;
    QString m_categoriesFilePath;

    // 初始化默认类别
    void initDefaultCategories();
    
    // 从文件加载类别
    bool loadCategoriesFromFile(const QString &filePath);
    
    // 保存类别到文件
    bool saveCategoriesToFile(const QString &filePath);
};

#endif // CATEGORYMANAGER_H
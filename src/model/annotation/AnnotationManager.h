#ifndef ANNOTATIONMANAGER_H
#define ANNOTATIONMANAGER_H

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QUrl>
#include <QVariantList>
#include <qqmlintegration.h>
#include <QVariant>
#include <QVariantMap>
#include <QString>

class AnnotationManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit AnnotationManager(QObject *parent = nullptr);

    Q_INVOKABLE QString loadAnnotations(const QString &filePath);

    // 添加新方法，用于保存标注到指定的文件路径
    Q_INVOKABLE bool saveAnnotationToFile(const QString &filePath, const QVariantList &annotations);

signals:
    void saveCompleted(bool success, const QString &message);
    void loadCompleted(bool success, const QString &message);
};

#endif // ANNOTATIONMANAGER_H

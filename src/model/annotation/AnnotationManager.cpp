#include "annotationmanager.h"
#include <cmath>
#include <QDebug>

AnnotationManager::AnnotationManager(QObject *parent)
    : QObject(parent)
{}

bool AnnotationManager::saveAnnotationToFile(const QString &filePath, const QVariantList &annotations)
{
    qDebug() << "Saving annotations to file:" << filePath;
    qDebug() << "Number of annotations:" << annotations.size();
    qDebug() << "Saving annotations to specific file:" << filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file for writing:" << filePath;
        emit saveCompleted(false, "Could not open file for writing");
        return false;
    }

    QJsonArray jsonArray;
    for (const QVariant &annotation : annotations) {
        QVariantMap annotationMap = annotation.toMap();

        // 打印每个标注的键和值
        qDebug() << "Annotation keys:";
        for (auto it = annotationMap.begin(); it != annotationMap.end(); ++it) {
            qDebug() << "  " << it.key() << ":" << it.value().toString();
        }

        // 检查是否有相对坐标
        if (annotationMap.contains("relX") && annotationMap.contains("relY")
            && annotationMap.contains("relWidth") && annotationMap.contains("relHeight")) {
            // 检查相对坐标是否为 NaN
            double relX = annotationMap["relX"].toDouble();
            double relY = annotationMap["relY"].toDouble();
            double relWidth = annotationMap["relWidth"].toDouble();
            double relHeight = annotationMap["relHeight"].toDouble();

            if (std::isnan(relX) || std::isnan(relY) || std::isnan(relWidth)
                || std::isnan(relHeight)) {
                qWarning() << "Skipping annotation with NaN relative coordinates";
                continue;
            }

            qDebug() << "Saving annotation with relative coordinates:";
            qDebug() << "  relX:" << relX;
            qDebug() << "  relY:" << relY;
            qDebug() << "  relWidth:" << relWidth;
            qDebug() << "  relHeight:" << relHeight;
        }
        // 兼容旧格式，检查是否有绝对坐标
        else if (
            annotationMap.contains("x") && annotationMap.contains("y")
            && annotationMap.contains("width") && annotationMap.contains("height")) {
            qDebug() << "Saving annotation with absolute coordinates (old format)";
        } else {
            qWarning() << "Annotation missing required coordinates, skipping";
            continue;
        }

        QJsonObject jsonObject = QJsonObject::fromVariantMap(annotationMap);
        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDoc(jsonArray);
    QByteArray jsonData = jsonDoc.toJson();

    if (file.write(jsonData) == -1) {
        qWarning() << "Failed to write to file:" << filePath;
        emit saveCompleted(false, "Failed to write to file");
        return false;
    }

    file.close();
    qDebug() << "Successfully saved annotations to file:" << filePath;
    emit saveCompleted(true, "Successfully saved annotations");
    return true;
}

QString AnnotationManager::loadAnnotations(const QString &filePath)
{
    qDebug() << "Loading annotations from:" << filePath;

    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << "Annotation file does not exist:" << filePath;
        return "";
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file for reading:" << file.errorString();
        return "";
    }

    QByteArray jsonData = file.readAll();
    file.close();

    // 验证JSON格式
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "Failed to parse JSON:" << parseError.errorString();
        return "";
    }

    // 检查并转换旧格式的标注数据为新的相对坐标格式
    QJsonArray annotations = document.array();
    bool needsConversion = false;

    for (int i = 0; i < annotations.size(); ++i) {
        QJsonObject annotation = annotations[i].toObject();
        if (annotation.contains("x") && annotation.contains("y") && annotation.contains("width")
            && annotation.contains("height") && !annotation.contains("relX")) {
            needsConversion = true;
            break;
        }
    }

    if (needsConversion) {
        qDebug() << "Converting old format annotations to relative coordinates format";
        QJsonArray newAnnotations;

        for (int i = 0; i < annotations.size(); ++i) {
            QJsonObject annotation = annotations[i].toObject();
            if (annotation.contains("x") && annotation.contains("y") && annotation.contains("width")
                && annotation.contains("height")) {
                // 这里我们假设图片尺寸为1.0
                // x 1.0（百分比），实际使用时会根据图片实际尺寸进行缩放
                QJsonObject newAnnotation;
                newAnnotation["relX"] = annotation["x"].toDouble();
                newAnnotation["relY"] = annotation["y"].toDouble();
                newAnnotation["relWidth"] = annotation["width"].toDouble();
                newAnnotation["relHeight"] = annotation["height"].toDouble();

                if (annotation.contains("category")) {
                    newAnnotation["category"] = annotation["category"].toString();
                } else {
                    newAnnotation["category"] = "未分类";
                }

                newAnnotations.append(newAnnotation);
            }
        }

        QJsonDocument newDocument(newAnnotations);
        return QString(newDocument.toJson());
    }

    qDebug() << "Successfully loaded annotations from" << filePath;
    return QString(jsonData);
}
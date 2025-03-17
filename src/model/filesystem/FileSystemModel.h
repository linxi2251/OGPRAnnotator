#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QObject>
#include <QDir>
#include <QFileSystemModel>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include <qqmlintegration.h>

class FileSystemModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QStringList imageFiles READ imageFiles NOTIFY imageFilesChanged)
    Q_PROPERTY(QString currentImagePath READ currentImagePath WRITE setCurrentImagePath NOTIFY currentImagePathChanged)
    Q_PROPERTY(int currentImageIndex READ currentImageIndex WRITE setCurrentImageIndex NOTIFY currentImageIndexChanged)
    Q_PROPERTY(QString folderPath READ folderPath NOTIFY folderPathChanged)

public:
    explicit FileSystemModel(QObject *parent = nullptr);

    QStringList imageFiles() const;
    QString currentImagePath() const;
    int currentImageIndex() const;
    QString folderPath() const;

    void setCurrentImagePath(const QString &path);
    Q_INVOKABLE void setCurrentImageIndex(int index);

    Q_INVOKABLE bool openFolder(const QUrl &folderUrl);
    Q_INVOKABLE bool nextImage();
    Q_INVOKABLE bool previousImage();
    Q_INVOKABLE QString getImageFileName(int index) const;
    Q_INVOKABLE QString getAnnotationFilePath(int index) const;

signals:
    void imageFilesChanged();
    void currentImagePathChanged();
    void currentImageIndexChanged();
    void folderPathChanged();

private:
    QStringList m_imageFiles;
    QString m_currentImagePath;
    int m_currentImageIndex;
    QString m_folderPath;
    QStringList m_supportedFormats;

    void updateImageFiles();
    QString generateAnnotationFilePath(const QString &imagePath) const;
};

#endif // FILESYSTEMMODEL_H
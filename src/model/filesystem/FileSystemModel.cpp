#include "filesystemmodel.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>

FileSystemModel::FileSystemModel(QObject *parent)
    : QObject(parent)
    , m_currentImageIndex(-1)
{
    // 设置支持的图片格式
    m_supportedFormats << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp";
}

QStringList FileSystemModel::imageFiles() const
{
    return m_imageFiles;
}

QString FileSystemModel::currentImagePath() const
{
    return m_currentImagePath;
}

int FileSystemModel::currentImageIndex() const
{
    return m_currentImageIndex;
}

QString FileSystemModel::folderPath() const
{
    return m_folderPath;
}

void FileSystemModel::setCurrentImagePath(const QString &path)
{
    if (m_currentImagePath != path) {
        m_currentImagePath = path;
        
        // 更新当前图片索引
        m_currentImageIndex = m_imageFiles.indexOf(path);
        
        emit currentImagePathChanged();
        emit currentImageIndexChanged();
    }
}

void FileSystemModel::setCurrentImageIndex(int index)
{
    if (index >= 0 && index < m_imageFiles.size() && m_currentImageIndex != index) {
        m_currentImageIndex = index;
        m_currentImagePath = m_imageFiles.at(index);
        
        emit currentImagePathChanged();
        emit currentImageIndexChanged();
    }
}

bool FileSystemModel::openFolder(const QUrl &folderUrl)
{
    QString folderPath = folderUrl.toLocalFile();
    QDir dir(folderPath);
    
    if (!dir.exists()) {
        qWarning() << "Folder does not exist:" << folderPath;
        return false;
    }
    
    m_folderPath = folderPath;
    updateImageFiles();
    
    // 如果有图片，选择第一张
    if (!m_imageFiles.isEmpty()) {
        setCurrentImageIndex(0);
    } else {
        m_currentImageIndex = -1;
        m_currentImagePath = "";
        emit currentImagePathChanged();
        emit currentImageIndexChanged();
    }
    qDebug() << "Opening folder:" << folderPath;
    emit folderPathChanged();
    return true;
}

bool FileSystemModel::nextImage()
{
    if (m_imageFiles.isEmpty() || m_currentImageIndex >= m_imageFiles.size() - 1) {
        return false;
    }
    
    setCurrentImageIndex(m_currentImageIndex + 1);
    return true;
}

bool FileSystemModel::previousImage()
{
    if (m_imageFiles.isEmpty() || m_currentImageIndex <= 0) {
        return false;
    }
    
    setCurrentImageIndex(m_currentImageIndex - 1);
    return true;
}

QString FileSystemModel::getImageFileName(int index) const
{
    if (index >= 0 && index < m_imageFiles.size()) {
        QFileInfo fileInfo(m_imageFiles.at(index));
        return fileInfo.fileName();
    }
    return QString();
}

void FileSystemModel::updateImageFiles()
{
    m_imageFiles.clear();
    
    if (m_folderPath.isEmpty()) {
        emit imageFilesChanged();
        return;
    }
    
    QDir dir(m_folderPath);
    QStringList files = dir.entryList(m_supportedFormats, QDir::Files, QDir::Name);
    
    for (const QString &file : files) {
        m_imageFiles.append(dir.absoluteFilePath(file));
    }
    
    emit imageFilesChanged();
}

QString FileSystemModel::generateAnnotationFilePath(const QString &imagePath) const
{
    QFileInfo fileInfo(imagePath);
    return fileInfo.absolutePath() + "/" + fileInfo.completeBaseName() + ".json";
}

QString FileSystemModel::getAnnotationFilePath(int index) const
{
    if (index >= 0 && index < m_imageFiles.size()) {
        return generateAnnotationFilePath(m_imageFiles.at(index));
    }
    return QString();
}

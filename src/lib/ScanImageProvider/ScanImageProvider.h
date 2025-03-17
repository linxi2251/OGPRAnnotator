//
// Created by buf on 2025/3/4.
//

#ifndef SCANIMAGEPROVIDER_H
#define SCANIMAGEPROVIDER_H

#include "RadarProcessor.h"
#include <Eigen/Core>
#include <opencv2/core/eigen.hpp>
#include <opencv2/opencv.hpp>
#include <QImage>
#include <QQuickImageProvider>

class ScanImageProvider : public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit ScanImageProvider(QObject *parent = nullptr);
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    void setScan(const RadarProcessor &processorBscan, int width, int height);

    QImage image() const;
    cv::Mat cvMat() const;
signals:
    void scanUpdated();
private:
    void processScanMacro(const QString &rawStr);
    RadarProcessor m_processorScan;
    QString m_macroStr;
    int m_width = 512;
    int m_height = 512;
    QImage m_image;
    cv::Mat m_cvMat;
};

#endif //SCANIMAGEPROVIDER_H

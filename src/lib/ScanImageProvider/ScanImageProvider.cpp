//
// Created by buf on 2025/3/4.
//

#include "ScanImageProvider.h"

inline Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> adjustContrast(
    const Eigen::MatrixXf &scan, const double contrastValue)
{

    if (contrastValue <= 0 || contrastValue >= 1) {
        qDebug() << "contrastValue should be in the range of (0, 1)";
        return scan;
    }

    const double maxValue = scan.maxCoeff();
    const double minValue = scan.minCoeff();
    const double adjusted_max = maxValue * (1 - contrastValue);
    const double adjusted_min = minValue * (1 - contrastValue);

    return scan.array()
        .max(adjusted_min)  // 将小于 adjusted_min 的值替换为 adjusted_min
        .min(adjusted_max); // 将大于 adjusted_max 的值替换为 adjusted_max
}

ScanImageProvider::ScanImageProvider(QObject *parent)
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_processorScan()
{}

void ScanImageProvider::processScanMacro(const QString &macroStr)
{
    m_macroStr = macroStr;
    m_processorScan.resetOriginalScan();
    if (m_macroStr.isEmpty()) {
        return;
    }
    QStringList funcs = m_macroStr.split("/");
    for (int i = 0; i < funcs.length() - 1; i++) {
        auto tmp = funcs[i].split("_");
        auto funcName = tmp[0];
        auto funcParams = tmp[1].split(",");
        // qDebug() << funcName << " " << funcParams;

        if (funcName == "DW") {
            m_processorScan.dewow();
        } else if (funcName == "STS") {
            m_processorScan.startTimeShifter(-29);
        } else if (funcName == "EG") {
            if (funcParams.length() != 2) {
                qDebug() << "exponentialGain params error";
            } else {
                const auto exponent = funcParams[0].toDouble();
                const auto exponentScale = funcParams[1].toDouble();
                m_processorScan.exponentialGain(exponentScale, exponent, 0, 483);
            }
        } else if (funcName == "BR") {
            if (funcParams.length() != 1) {
                qDebug() << "removeDynamicWindowBackground params error";
            } else {
                const auto dw = funcParams[0].toInt();
                m_processorScan.removeDynamicWindowBackground(dw, 0, 512);
            }
        } else if (funcName == "BF") {
            if (funcParams.length() != 2) {
                qDebug() << "bandpassFilter params error";
            } else {
                const auto highCut = funcParams[0].toDouble();
                const auto lowCut = funcParams[1].toDouble();
                m_processorScan.bandpassFilter(lowCut, highCut, 1500);
            }
        } else if (funcName == "ABR") {
            if (funcParams.length() != 1) {
                qDebug() << "adaptiveBackgroundRemoval params error";
            } else {
                const auto q = funcParams[0].toInt();
                m_processorScan.adaptiveBackgroundRemoval(q);
            }
        }
        else {
            // Default case
        }
    }

    if (m_processorScan.scanType() == RadarProcessor::ScanType::BScan) {
        qDebug() << "BScan";
    } else if (m_processorScan.scanType() == RadarProcessor::ScanType::CScan) {
        qDebug() << "CScan";
    } else if (m_processorScan.scanType() == RadarProcessor::ScanType::TScan) {
        qDebug() << "TScan";
    }
}

QImage ScanImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(requestedSize);

    if (m_processorScan.scan().cols() == 0 || m_processorScan.scan().rows() == 0) {
        qDebug() << "scan is empty";
        return QImage();
    }

    if (size) {
        *size = QSize(m_width, m_height);
    }
    const auto splitedId = id.split("#");
    const auto contrast = splitedId[0].toDouble();
    // qDebug() << "id:" << id;
    // qDebug() << "splitedId:" << splitedId;
    // qDebug() << "contrast:" << contrast;

    if (const auto macroStr = splitedId[1]; m_macroStr != macroStr) {
        processScanMacro(macroStr);
    }

    cv::Mat cvMat;
    const auto finalScan = adjustContrast(m_processorScan.scan(), contrast);
    cv::eigen2cv(finalScan, cvMat);
    cv::normalize(cvMat, cvMat, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    const QImage image(cvMat.data, cvMat.cols, cvMat.rows, cvMat.step, QImage::Format_Grayscale8);
    m_image = image.scaled(m_width, m_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_cvMat = cvMat;
    return m_image;
}

void ScanImageProvider::setScan(
    const RadarProcessor &processorBscan, const int width, const int height)
{
    m_processorScan = processorBscan;
    m_width = width;
    m_height = height;
    m_macroStr = "";
    emit scanUpdated();
}

QImage ScanImageProvider::image() const
{
    if (m_image.isNull()) {
        return {};
    }
    return m_image;
}

cv::Mat ScanImageProvider::cvMat() const {
    return m_cvMat;
}

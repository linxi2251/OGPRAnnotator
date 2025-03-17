#ifndef RADARPROCESSOR_H
#define RADARPROCESSOR_H
#include <unsupported/Eigen/FFT>

class RadarProcessor {
public:
    enum class ScanType {
        BScan,
        CScan,
        TScan
    };
    RadarProcessor() = default;
    // 拷贝构造函数
    RadarProcessor(const Eigen::MatrixXf &scan, ScanType scanType);

    // 移动构造函数
    RadarProcessor(Eigen::MatrixXf &&scan, ScanType scanType);

    // 析构函数
    ~RadarProcessor();

    RadarProcessor &resetOriginalScan();

    const Eigen::MatrixXf &scan() const;

    const ScanType &scanType() const;

    void setScan(const Eigen::MatrixXf &scan);

    // Dewow 算法：去除雷达数据中的低频噪声
    RadarProcessor &dewow();

    // Start Time Shifter 算法：调整雷达数据的起始时间
    RadarProcessor &startTimeShifter(int shift);

    // Sliding Window Background Removal 算法：使用滑动窗口动态调整背景去除
    RadarProcessor & removeDynamicWindowBackground(int dw, int s, int e);

    RadarProcessor &removeBackground(int windowSize);

    RadarProcessor & adaptiveBackgroundRemoval(int q);

    // Time-domain Bandpass Filter 算法：时域带通滤波，去除高频和低频噪声
    RadarProcessor & bandpassFilter(double lowCut, double highCut, double samplingRate);

    // STC Smoothed Gain 算法：STC 平滑增益，补偿雷达信号随深度衰减的问题
    // RadarProcessor & stcSmoothedGain(int windowSize, double smoothingSize);

    RadarProcessor & exponentialGain(double scale, double exponent, double startTimes=0, double lastTimes=512);

    RadarProcessor &standardizeMatrixGlobal();

    RadarProcessor & standardizeMatrixByRow();

private:
    Eigen::MatrixXf m_scan;
    Eigen::MatrixXf m_originalScan;
    ScanType m_scanType;

    void standardizeData();
};

#endif // RADARPROCESSOR_H

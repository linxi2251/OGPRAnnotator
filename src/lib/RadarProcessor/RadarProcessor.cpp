#include "RadarProcessor.h"
#include <Eigen/Dense>
#include <unsupported/Eigen/FFT>
#include <qdebug.h>
#include <cmath>
#include <iostream>
using Eigen::MatrixXf;
// 构造函数

RadarProcessor::RadarProcessor(const Eigen::MatrixXf &scan, const ScanType scanType)
    : m_scan(scan)
    , m_originalScan(scan)
    , m_scanType(scanType)
{
    // qDebug() << m_scan.maxCoeff() <<", "<< m_scan.minCoeff();
    // standardizeData(); // 将数据缩放到-20, 20
    // qDebug() << m_scan.maxCoeff() <<", "<< m_scan.minCoeff();
}

RadarProcessor::RadarProcessor(Eigen::MatrixXf &&scan, const ScanType scanType)
    : m_scan(std::move(scan))
    , m_scanType(scanType)
{
    // standardizeMatrixGlobal();
    // standardizeMatrixByRow();
    m_originalScan = m_scan; // 复制一份原始数据

}

// 析构函数
RadarProcessor::~RadarProcessor()
{
    // 清理操作（如果需要）
    qDebug() << "RadarProcessor::~RadarProcessor";
}

RadarProcessor &RadarProcessor::resetOriginalScan()
{
    m_scan = m_originalScan;
    return *this;
}

void RadarProcessor::setScan(const Eigen::MatrixXf &scan)
{
    m_scan = scan;
}

// 标准化处理函数
void RadarProcessor::standardizeData()
{
    const double maxVal = m_scan.maxCoeff();
    const double minVal = m_scan.minCoeff();
    m_scan = 2 * (m_scan.array() - minVal) / (maxVal - minVal) - 1;
}

const Eigen::MatrixXf &RadarProcessor::scan() const
{
    return m_scan;
}

const RadarProcessor::ScanType &RadarProcessor::scanType() const
{
    return m_scanType;
}

// Dewow 算法：去除雷达数据中的低频噪声
RadarProcessor &RadarProcessor::dewow()
{
    // 计算每列的平均值
    Eigen::VectorXf colMeans = m_scan.colwise().mean();
    // 减去每列的平均值
    m_scan.rowwise() -= colMeans.transpose();
    return *this;
}

// Start Time Shifter 算法：调整雷达数据的起始时间
RadarProcessor &RadarProcessor::startTimeShifter(const int shift)
{
    if (shift > 0) {
        // 向下移动
        m_scan.bottomRows(m_scan.rows() - shift) = m_scan.topRows(m_scan.rows() - shift).eval();
        m_scan.topRows(shift).setZero(); // 填充零
    } else if (shift < 0) {
        // 向上移动
        m_scan.topRows(m_scan.rows() + shift) = m_scan.bottomRows(m_scan.rows() + shift).eval();
        m_scan.bottomRows(-shift).setZero(); // 填充零
    }
    return *this;
}

RadarProcessor &RadarProcessor::adaptiveBackgroundRemoval(const int q)
{
    const int N = m_scan.cols(); // B-SCAN 数据的列数（A-SCAN 的道数）
    const int M = m_scan.rows(); // B-SCAN 数据的行数（每道 A-SCAN 的采样点数）
    const int W = N / q;         // 滑动窗口的大小
    std::cout << "N = " << N << ", M = " << M << std::endl;
    Eigen::MatrixXf B_prime = m_scan; // 复制原始数据

#pragma omp parallel for num_threads(16)
    for (int i = 0; i < N; ++i) {
        int windowStart, windowSize;

        // 确定滑动窗口的起始位置和大小
        if (i < N - W + 1) {
            // 正常情况：窗口大小为 W
            windowStart = i;
            windowSize = W;
        } else {
            // 边缘情况：窗口大小为 N - i
            windowStart = i - (N - W);
            windowSize = N - i;
        }

        // 提取滑动窗口
        Eigen::MatrixXf window = m_scan.block(0, windowStart, M, windowSize);

        // 计算窗口内每行的均值
        const Eigen::VectorXf mean = window.rowwise().mean();

        // 减去均值，去除背景杂波
        B_prime.col(i) -= mean;
    }

    m_scan = B_prime; // 更新处理后的数据
    return *this;     // 返回当前对象的引用，支持链式调用
}

RadarProcessor &RadarProcessor::removeDynamicWindowBackground(int dw, int s, int e)
{
    const int nr = m_scan.rows(); // 行数
    const int nc = m_scan.cols(); // 列数
    // std::cout << "nr: " << nr << ", nc: " << nc << ", dw: " << dw << ", s: " << s << ", e: " << e
    //           << std::endl;
    // 异常处理
    if (s < 0 || s >= nr || e <= 0 || e > nr) {
        s = 0;
        e = nr;
    }
    if (dw <= 0 || dw >= nc / 4) {
        dw = nc / 4;
    }

    // 动态窗口大小 N，确保为奇数
    const int N = (dw % 2 != 0) ? dw : dw + 1;

    // 复制数据
    Eigen::MatrixXf rewgb = m_scan;

// 动态窗口去背景（并行化列循环）
#pragma omp parallel for num_threads(16)
    for (int i = 0; i < nc; ++i) {
        if (i >= (N + 1) / 2 - 1 && i < nc - (N - 1) / 2) {
            // 正常窗口处理
            const int start = i - (N - 1) / 2;
            const int end = i + (N - 1) / 2 + 1; // Eigen 的 block 是左闭右开区间
            rewgb.block(s, i, e - s, 1)
                = m_scan.block(s, i, e - s, 1)
                  - m_scan.block(s, start, e - s, end - start).rowwise().mean();
        } else if (i < (N + 1) / 2 - 1) {
            // 左侧边缘处理
            rewgb.block(s, i, e - s, 1)
                = m_scan.block(s, i, e - s, 1)
                  - m_scan.block(s, 0, e - s, (N + 1) / 2 - 1).rowwise().mean();
        } else {
            // 右侧边缘处理
            rewgb.block(s, i, e - s, 1)
                = m_scan.block(s, i, e - s, 1)
                  - m_scan.block(s, nc - (N - 1) / 2, e - s, (N - 1) / 2).rowwise().mean();
        }
    }

    // 更新矩阵
    m_scan = rewgb;

    return *this;
}

#include <omp.h> // 引入 OpenMP 头文件
// 带通滤波
RadarProcessor &RadarProcessor::bandpassFilter(double lowCut, double highCut, double samplingRate)
{
    int nr = m_scan.rows(); // 行数
    int nc = m_scan.cols(); // 列数
    // MHz -> Hz
    // lowCut *= 1e6;
    // highCut *= 1e6;
    // samplingRate *= 1e6;
    // 计算频率分辨率
    double df = samplingRate / nr;
    // 创建 FFT 对象
    Eigen::FFT<float> fft;

    // 使用 OpenMP 并行化列处理
#pragma omp parallel for firstprivate(fft)
    for (int i = 0; i < nc; ++i) {
        // 获取当前列的数据
        Eigen::VectorXf col = m_scan.col(i);

        // 对列数据进行傅里叶变换
        Eigen::VectorXcf freq_spectrum = fft.fwd(col);
        for (int j = 0; j < nr; ++j) {
            double freq = j * df; // 当前频率

            // 如果频率不在 [lowCut, highCut] 范围内，则将频率分量置零
            if (freq < lowCut || freq > highCut) {
                freq_spectrum(j) = 0.0f;
            }
        }

        // 对滤波后的频域数据进行逆傅里叶变换，转换回时域
        Eigen::VectorXf filtered_col = fft.inv(freq_spectrum).real();

        // 将滤波后的数据存回矩阵
        m_scan.col(i) = filtered_col;
    }

    return *this;
}

// 移除背景噪声（使用滑动窗口）
RadarProcessor &RadarProcessor::removeBackground(int windowSize)
{
    int numTraces = m_scan.cols();  // 轨迹的数量
    int numSamples = m_scan.rows(); // 每个轨迹的采样点数

    // 确保窗口大小是奇数
    if (windowSize % 2 == 0) {
        windowSize += 1; // 如果不是奇数，调整为奇数
        qDebug() << "Adjusted windowSize to " << windowSize << " (must be odd).";
    }

    int halfWindow = windowSize / 2; // 窗口的一半

    // 创建一个临时矩阵来存储处理后的数据
    Eigen::MatrixXf result = m_scan;

// 遍历每个时间点
#pragma omp parallel for
    for (int t = 0; t < numSamples; ++t) {
        // 确定窗口的起始和结束位置
        int start = std::max(0, t - halfWindow);
        int end = std::min(numSamples - 1, t + halfWindow);

        // 遍历每个轨迹
        for (int i = 0; i < numTraces; ++i) {
            // 计算当前轨迹在窗口内的平均值
            float sum = 0.0f;
            int count = 0;
            for (int w = start; w <= end; ++w) {
                sum += m_scan(w, i);
                count++;
            }
            float average = sum / count;

            // 将平均值从当前时间点的轨迹中减去
            result(t, i) = m_scan(t, i) - average;
        }
    }

    m_scan = result; // 更新原始数据
    return *this;
}

RadarProcessor &RadarProcessor::exponentialGain(
    const double scale, const double exponent, double startTimes, double lastTimes)
{
    const int nr = m_scan.rows();
    // 计算 t_0 和 t_end
    const double t_0 = std::pow(startTimes, 1.0 / exponent);
    const double t_end = std::pow(lastTimes, 1.0 / exponent);

    // 创建时间向量 t
    Eigen::VectorXf t = Eigen::VectorXf::LinSpaced(nr, t_0, t_end);
    Eigen::VectorXf d = scale * t.array().pow(exponent).matrix();

    // // 将增益向量转为矩阵， 以便进行矩阵乘法
    // Eigen::MatrixXf D = d.replicate(1, m_scan.cols());

    // 应用增益到数据矩阵
    m_scan.array().colwise() *= d.array();

    return *this;
}

// 矩阵整体标准化函数
RadarProcessor &RadarProcessor::standardizeMatrixGlobal()
{
    // 获取矩阵的行数和列数
    int rows = m_scan.rows();
    int cols = m_scan.cols();

    // 计算整个矩阵的均值和标准差
    double mean = m_scan.mean();                                                       // 均值
    double stddev = std::sqrt((m_scan.array() - mean).square().sum() / (rows * cols)); // 标准差

    // 如果标准差为 0，说明所有元素相同，标准化后全部设为 0
    if (stddev == 0) {
        m_scan.setZero();
    } else {
        // 标准化：减去均值，除以标准差
        m_scan = (m_scan.array() - mean) / stddev;
    }
    return *this;
}

// 行标准化函数：对矩阵的每一行进行标准化
RadarProcessor &RadarProcessor::standardizeMatrixByRow()
{
    // 获取矩阵的行数和列数
    int rows = m_scan.rows();
    int cols = m_scan.cols();

    // 对每一行进行标准化
    for (int i = 0; i < rows; ++i) {
        // 计算当前行的均值和标准差
        double mean = m_scan.row(i).mean();
        double stddev = std::sqrt((m_scan.row(i).array() - mean).square().sum() / cols);

        // 如果标准差为 0，说明该行所有值相同，标准化后全部设为 0
        if (stddev == 0) {
            m_scan.row(i).setZero();
        } else {
            // 标准化：减去均值，除以标准差
            m_scan.row(i) = (m_scan.row(i).array() - mean) / stddev;
        }
    }
    return *this;
}

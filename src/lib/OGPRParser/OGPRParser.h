#ifndef OGPRParser_H
#define OGPRParser_H

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <unsupported/Eigen/CXX11/Tensor> // 引入 Eigen::Tensor
#include <Eigen/Dense> // 包含 Eigen::Matrix

struct RadarInfo {
    float samplingStep_m;
    float samplingTime_ns;
    float propagationVelocity_mPerSec;
    int fequency_MHz;
    QString polarization;

    RadarInfo() = default;

    explicit RadarInfo(QJsonObject &obj) {
        samplingStep_m = obj["samplingStep_m"].toDouble();
        samplingTime_ns = obj["samplingTime_ns"].toDouble();
        propagationVelocity_mPerSec = obj["propagationVelocity_mPerSec"].toDouble();
        fequency_MHz = obj["fequency_MHz"].toInt();
        polarization = obj["polarization"].toString();
    }

    static RadarInfo fromQJsonObject(QJsonObject &obj) {
        return RadarInfo(obj);
    }

    // 重载 = 运算符
    RadarInfo &operator=(const RadarInfo &other) {
        samplingStep_m = other.samplingStep_m;
        samplingTime_ns = other.samplingTime_ns;
        propagationVelocity_mPerSec = other.propagationVelocity_mPerSec;
        fequency_MHz = other.fequency_MHz;
        polarization = other.polarization;
        return *this;
    }
    // 重载 << 运算符
    friend QDebug operator<<(QDebug dbg, const RadarInfo &info) {
        dbg.nospace() << "RadarInfo("
                      << "samplingStep_m: " << info.samplingStep_m << ", "
                      << "samplingTime_ns: " << info.samplingTime_ns << ", "
                      << "propagationVelocity_mPerSec: " << info.propagationVelocity_mPerSec << ", "
                      << "fequency_MHz: " << info.fequency_MHz << ", "
                      << "polarization: " << info.polarization.toStdString()
                      << ")";
        return dbg.space();
    }
    // 重载 << 标准库运算符
    friend std::ostream &operator<<(std::ostream &os, const RadarInfo &info) {
        os << "RadarInfo("
           << "samplingStep_m: " << info.samplingStep_m << ", "
           << "samplingTime_ns: " << info.samplingTime_ns << ", "
           << "propagationVelocity_mPerSec: " << info.propagationVelocity_mPerSec << ", "
           << "fequency_MHz: " << info.fequency_MHz << ", "
           << "polarization: " << info.polarization.toStdString()
           << ")";
        return os;
    }
};

// 雷达数据块
struct RadarVolume {
    QString name;
    Eigen::Tensor<float, 3> data; // 使用 Eigen::Tensor 存储雷达数据
    QJsonObject metadata;
    RadarInfo radarInfo;
};

// 地理定位数据块
struct SampleGeolocations {
    QString name;
    QJsonObject srs; // 空间参考系统
    QVector<QPair<double, double> > latLonCoordinates;  // 用于存储经纬度信息
};

// JSON 头文件
struct OpenGPRHeader {
    int majorVersion;
    int minorVersion;
    int samplesCount;
    int channelsCount;
    int slicesCount;
    QJsonObject metadata;
    RadarVolume radarVolume;
    SampleGeolocations sampleGeolocations;

    int maxChannels() const {
        return channelsCount;
    }

    // 单位为 m
    int maxPositionM() const {
        return slicesCount * radarVolume.radarInfo.samplingStep_m;
    }
    // 单位为 cm
    int maxDepth() const {
        // qDebug() << "samplesCount: " << samplesCount;
        // qDebug() << "radarVolume.radarInfo.samplingTime_ns: " << radarVolume.radarInfo.samplingTime_ns;
        // qDebug() << "radarVolume.radarInfo.propagationVelocity_mPerSec: " << radarVolume.radarInfo.propagationVelocity_mPerSec;
        return samplesCount * radarVolume.radarInfo.samplingTime_ns * radarVolume.radarInfo.propagationVelocity_mPerSec * 1e-9 * 100 / 2;
    }
};

// 整个 .ogpr 文件
struct OpenGPRFile {
    QString magicNumber;
    QString md5;
    OpenGPRHeader header;
};

class OGPRParser {
public:
    // 构造函数
    OGPRParser();

    // 析构函数
    ~OGPRParser();

    // 解析 .ogpr 文件
    bool parseOGPRFile(const QString &filePath);

    // 获取 BScan 切片（通道方向）
    Eigen::MatrixXf getBScan(int channelIndex);

    // 获取 CScan 切片（深度方向）
    Eigen::MatrixXf getCScan(int depthIndex) const;

    // 获取 TScan 切片（行进方向）
    Eigen::MatrixXf getTScan( int sliceIndex) const;

    // 获取雷达数据块
    const RadarVolume &getRadarVolume() const;

    // 获取道头信息
    const OpenGPRHeader &getHeader() const;

    // 获取地理定位数据块
    const SampleGeolocations &getSampleGeolocations() const;

    // 获取雷达数据块的形状
    std::vector<Eigen::DenseIndex> getRadarVolumeShape(int volumeIndex) const;

private:
    // 解析雷达数据块
    bool parseRadarVolume(const QByteArray &data, const QJsonObject &blockObj);

    // 解析地理定位数据块
    bool parseSampleGeolocations(const QByteArray &data, const QJsonObject &blockObj);

    // 存储解析后的 .ogpr 文件数据
    OpenGPRFile m_ogprFile;
};

#endif // OGPRParser_H

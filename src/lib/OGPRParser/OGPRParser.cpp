#include "OGPRParser.h"
#include <QJsonArray>
#include <iostream>
// 构造函数
OGPRParser::OGPRParser()
    : m_ogprFile({})
{}
OGPRParser::~OGPRParser()
{
    qDebug() << "~OGPRParser";
}

// 解析 .ogpr 文件
bool OGPRParser::parseOGPRFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << filePath;
        return false;
    }

    // Step 1: Read Preamble
    const QByteArray preamble = file.read(47); // Fixed size preamble
    if (preamble.size() < 47) {
        qWarning() << "Invalid file format: Preamble too short";
        return false;
    }

    // Extract Magic Number
    m_ogprFile.magicNumber = preamble.left(5);
    if (m_ogprFile.magicNumber != "ogpr\n") {
        qWarning() << "Invalid file format: Magic number mismatch";
        return false;
    }

    // Extract MD5 and JSON Header Size
    m_ogprFile.md5 = preamble.mid(5, 32); // a fixed size text line (32 bytes + LF)
    const QByteArray jsonHeaderSizeStr = preamble.mid(38, 8); // a fixed size text line (8 bytes + LF)
    bool ok;
    const int jsonHeaderSize = jsonHeaderSizeStr.toInt(&ok);
    if (!ok || jsonHeaderSize <= 0) {
        qWarning() << "Invalid JSON header size";
        return false;
    }

    // Step 2: Read JSON Header
    const QByteArray jsonHeader = file.read(jsonHeaderSize);
    if (jsonHeader.size() < jsonHeaderSize) {
        qWarning() << "Invalid JSON header: Data too short";
        return false;
    }

    // Parse JSON Header
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonHeader);
    if (jsonDoc.isNull()) {
        qWarning() << "Failed to parse JSON header";
        return false;
    }

    QJsonObject jsonObj = jsonDoc.object();
    // Extract version information
    QJsonObject versionObj = jsonObj["version"].toObject();
    m_ogprFile.header.majorVersion = versionObj["major"].toInt();
    m_ogprFile.header.minorVersion = versionObj["minor"].toInt();

    // Extract main descriptor
    QJsonObject mainDescriptor = jsonObj["mainDescriptor"].toObject();
    m_ogprFile.header.samplesCount = mainDescriptor["samplesCount"].toInt();
    m_ogprFile.header.channelsCount = mainDescriptor["channelsCount"].toInt();
    m_ogprFile.header.slicesCount = mainDescriptor["slicesCount"].toInt();
    m_ogprFile.header.metadata = mainDescriptor["metadata"].toObject();

    // Step 3: Read Data Blocks
    QJsonArray dataBlockDescriptors = jsonObj["dataBlockDescriptors"].toArray();
    for (const QJsonValue &blockValue : dataBlockDescriptors) {
        QJsonObject blockObj = blockValue.toObject();
        QString type = blockObj["type"].toString();
        const auto byteSize = blockObj["byteSize"].toInt();
        const auto byteOffset = blockObj["byteOffset"].toInt();

        // Read binary data block
        file.seek(byteOffset);
        QByteArray dataBlock = file.read(byteSize);
        if (dataBlock.size() < byteSize) {
            qWarning() << "Failed to read data block:" << blockObj["name"].toString();
            continue;
        }

        // Process binary data block based on type
        if (type == "Radar Volume") {
            if (const auto ret = parseRadarVolume(dataBlock, blockObj); !ret) {
                qWarning() << "Failed to parse Radar Volume";
                return false;
            }
        } else if (type == "Sample Geolocations") {
            if (const auto ret = parseSampleGeolocations(dataBlock, blockObj); !ret) {
                qWarning() << "Failed to parse Sample Geolocations";
                return false;
            }
        }
    }

    // Step 4: Read Epilogue
    const QByteArray epilogue = file.read(33); // Fixed size epilogue
    if (epilogue.size() < 33) {
        qWarning() << "Invalid file format: Epilogue too short";
        return false;
    }

    // Verify MD5 checksum
    if (const QByteArray epilogueMd5 = epilogue.mid(1, 32); epilogueMd5 != m_ogprFile.md5) {
        qWarning() << "MD5 checksum mismatch";
        return false;
    }
    return true;
}

// 获取 BScan 切片（通道方向）
Eigen::MatrixXf OGPRParser::getBScan(const int channelIndex)
{
    // 获取切片后的二维张量
    Eigen::Tensor<float, 2> tensorSlice = m_ogprFile.header.radarVolume.data.chip(channelIndex, 1);

    // 直接使用 Eigen::Map 映射数据
    return Eigen::Map<Eigen::MatrixXf>(
        tensorSlice.data(),
        tensorSlice.dimension(0), // 行数
        tensorSlice.dimension(1)  // 列数
    );
}

// 获取 CScan 切片（深度方向）
Eigen::MatrixXf OGPRParser::getCScan(const int depthIndex) const
{
    // 获取沿第 2 维度切片的 Eigen::Tensor
    Eigen::Tensor<float, 2> tensorSlice = m_ogprFile.header.radarVolume.data.chip(depthIndex, 0);

    // 使用 Eigen::Map 将数据映射到 Eigen::Matrix
    return Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>>(
        tensorSlice.data(),
        tensorSlice.dimension(0), // 行数
        tensorSlice.dimension(1)  // 列数
    );
}

// 获取 TScan 切片（行进方向）
Eigen::MatrixXf OGPRParser::getTScan(const int sliceIndex) const
{
    if (sliceIndex < 0) {
        throw std::out_of_range("Invalid volume index");
    }

    // 获取沿第 0 维度切片的 Eigen::Tensor
    Eigen::Tensor<float, 2> tensorSlice = m_ogprFile.header.radarVolume.data.chip(sliceIndex, 2);

    // 使用 Eigen::Map 将数据映射到 Eigen::Matrix
    return Eigen::Map<Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>>(
        tensorSlice.data(),
        tensorSlice.dimension(0), // 行数
        tensorSlice.dimension(1)  // 列数
    );
}

const RadarVolume &OGPRParser::getRadarVolume() const
{
    return m_ogprFile.header.radarVolume;
}

const OpenGPRHeader &OGPRParser::getHeader() const
{
    return m_ogprFile.header;
}

const SampleGeolocations &OGPRParser::getSampleGeolocations() const
{
    return m_ogprFile.header.sampleGeolocations;
}

// 获取雷达数据块的形状
std::vector<Eigen::DenseIndex> OGPRParser::getRadarVolumeShape(const int volumeIndex) const
{
    const auto &data = m_ogprFile.header.radarVolume.data;
    return {data.dimension(0), data.dimension(1), data.dimension(2)};
}

Eigen::Tensor<int16_t, 3> transposeRadarVolume(const Eigen::Tensor<int16_t, 3> &tensor)
{
    // 定义新的维度顺序
    Eigen::array<int, 3> shuffle({2, 1, 0}); // 将维度顺序从 (0, 1, 2) 变为 (2, 1, 0)

    // 使用 shuffle 方法进行转置
    Eigen::Tensor<int16_t, 3> transposedTensor = tensor.shuffle(shuffle);

    return transposedTensor;
}

/**
*
1. First are stored all Samples of the first slice. Then follow all Samples of
the second Slice and so on. The first Slice should be the one that the
GPR system acquired first (if this statement makes sense for the GPR
system that has acquired the Radar Data Volume).

2. For each Slice, the Samples of the first Sweep are stored first. Then
follow the Samples of the second Sweep and so on. For an observer
pushing the GPR system and looking at it along the direction of
motion, Sweeps should be ordered from left to right. The first Sweep
should be the leftmost one (if this statement makes sense for the GPR
system that has acquired the Radar Data Volume).

3. For each Sweep, Samples are stored from the shallowest to the
deepest.
 */
// 将数字值转换为电压值，支持微调参数
inline float digital_to_voltage_calibrated(int digital_value,
                                     int D_min_meas = -32768,
                                     int D_max_meas = 32767,
                                     float delta_a = 0.0,
                                     float delta_b = 0.0) {
    // 理论比例因子
    float a = (D_max_meas - D_min_meas) / 40.0;
    // 理论截距
    float b = D_min_meas + 20.0 * a;
    // 加入微调参数后的比例因子和截距
    float a_adj = a + delta_a;
    float b_adj = b + delta_b;
    // 根据转换公式计算电压
    float voltage = (digital_value - b_adj) / a_adj;
    return voltage;
}
// 优化版
// 解析雷达数据块
bool OGPRParser::parseRadarVolume(const QByteArray &data, const QJsonObject &blockObj)
{
    m_ogprFile.header.radarVolume.name = blockObj["name"].toString();
    m_ogprFile.header.radarVolume.metadata = blockObj["metadata"].toObject();
    QJsonObject radar = blockObj["radar"].toObject();
    if (radar.isEmpty()) {
        qWarning() << "Radar object is empty";
        return false;
    }
    m_ogprFile.header.radarVolume.radarInfo = RadarInfo(radar);
    // 使用 Eigen::TensorMap 直接映射内存
    const auto rawData = reinterpret_cast<const int16_t *>(data.constData());
    // 这里一定要注意，Eigen::TensorMap 存储是按照列优先的，所以这里的维度顺序是 (samplesCount, channelsCount, slicesCount)
    const Eigen::TensorMap<Eigen::Tensor<const int16_t, 3>> tensorMap(
        rawData,
        m_ogprFile.header.samplesCount,
        m_ogprFile.header.channelsCount,
        m_ogprFile.header.slicesCount);
    // 转换为电压值：两个步骤合并
    m_ogprFile.header.radarVolume.data = std::move(
        tensorMap.unaryExpr([](int16_t val) {
            return digital_to_voltage_calibrated(val);
        }));

    // qDebug() << "Tensor size: " << tensorMap.size();
    // qDebug() << "Tensor memory size: " << sizeof(tensorMap);
    // 直接将 tensorMap 赋值给 radarVolume.data
    // m_ogprFile.header.radarVolume.data = std::move(
    //     tensorMap.cast<float>()); // 使用 std::move 避免复制
    // 输出最大最小值
    std::cout << "Max value: " << m_ogprFile.header.radarVolume.data.maximum();
    std::cout << "Min value: " << m_ogprFile.header.radarVolume.data.minimum();

    return true;
}

// // 未优化版
// RadarVolume OGPRParser::parseRadarVolume(const QByteArray &data, const QJsonObject &blockObj) const {
//     RadarVolume radarVolume;
//     radarVolume.name = blockObj["name"].toString();
//     radarVolume.metadata = blockObj["metadata"].toObject();
//
//     // 使用 Eigen::Tensor 存储雷达数据
//     radarVolume.data = Eigen::Tensor<int16_t, 3>(ogprFile.header.slicesCount, ogprFile.header.channelsCount,
//                                                  ogprFile.header.samplesCount);
//
//     const auto rawData = reinterpret_cast<const int16_t *>(data.constData());
//     int index = 0;
//
//     for (int slice = 0; slice < ogprFile.header.slicesCount; ++slice) {
//         for (int channel = 0; channel < ogprFile.header.channelsCount; ++channel) {
//             for (int sample = 0; sample < ogprFile.header.samplesCount; ++sample) {
//                 radarVolume.data(slice, channel, sample) = rawData[index++];
//             }
//         }
//     }
//
//     return radarVolume;
// }

// 解析地理定位数据块
bool OGPRParser::parseSampleGeolocations(const QByteArray &data, const QJsonObject &blockObj)
{
    m_ogprFile.header.sampleGeolocations.name = blockObj["name"].toString();
    m_ogprFile.header.sampleGeolocations.srs = blockObj["srs"].toObject();

    // 获取切片数量
    const int slicesCount = m_ogprFile.header.slicesCount;
    // 获取通道数量
    const int channelsCount = m_ogprFile.header.channelsCount;

    // 每个坐标块由4个双精度浮点数组成（x, y, depth, elevation）
    constexpr int coordsPerBlock = 4;
    // 每个扫描块包含2个坐标块
    constexpr int blocksPerSweep = 2;
    // 每个切片块包含一个64位整数（切片标识）和多个扫描块
    constexpr int sliceIdSize = sizeof(int64_t);

    // 计算每个切片块的大小
    const int sliceBlockSize = sliceIdSize
                               + channelsCount * blocksPerSweep * coordsPerBlock * sizeof(double);

    // 计算总的地理定位数据大小
    // 检查数据块大小是否匹配
    if (const int totalGeolocationsSize = slicesCount * sliceBlockSize;
        data.size() < totalGeolocationsSize) {
        qWarning() << "Invalid geolocations data block size";
        return false;
    }

    // 使用 Eigen::Tensor 存储地理定位数据
    // 地理定位数据的形状为 (slicesCount, channelsCount, 2, 4)
    // 其中 2 表示每个扫描块的两个坐标块，4 表示每个坐标块的四个坐标值
    // geolocations.geolocations = Eigen::Tensor<double, 4>(slicesCount, channelsCount, blocksPerSweep, coordsPerBlock);

    // 解析地理定位数据
    const auto rawData = reinterpret_cast<const double *>(data.constData());
    int index = 0;

    for (int slice = 0; slice < slicesCount; ++slice) {
        // 跳过切片标识（64位整数）
        index += sliceIdSize / sizeof(double);

        for (int channel = 0; channel < channelsCount; ++channel) {
            // 解析第一个坐标块（最小深度的坐标）
            double x = rawData[index++];         // 经度
            double y = rawData[index++];         // 纬度
            double depth = rawData[index++];     // 深度, 不使用但是需要解析，因为要index++
            double elevation = rawData[index++]; // 标高 同上

            // 只存储经纬度信息
            m_ogprFile.header.sampleGeolocations.latLonCoordinates.append(qMakePair(x, y));

            // 跳过第二个坐标块（最大深度的坐标）
            index += coordsPerBlock;
        }
    }

    return true;
}

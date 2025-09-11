// ===============================================
// common/protocol.cpp
// 协议打包实现
// ===============================================

#include "protocol.h"
#include <QDateTime>
#include <QBuffer>
#include <QImage>
#include <QtEndian>
#include <QVariant>

// 字节序转换函数
uint32_t ProtocolPackager::htonl(uint32_t hostlong) {
    return qToBigEndian(hostlong);
}

uint16_t ProtocolPackager::htons(uint16_t hostshort) {
    return qToBigEndian(hostshort);
}

uint32_t ProtocolPackager::ntohl(uint32_t netlong) {
    return qFromBigEndian(netlong);
}

uint16_t ProtocolPackager::ntohs(uint16_t netshort) {
    return qFromBigEndian(netshort);
}

QByteArray ProtocolPackager::packVideoFrame(uint32_t roomId,
                                          const QByteArray& frameData,
                                          uint64_t timestamp,
                                          int width,
                                          int height,
                                          const std::string& format,
                                          int fps) {
    QJsonObject jsonObj;
    jsonObj["roomId"] = static_cast<int>(roomId);
    jsonObj["ts"] = static_cast<qint64>(timestamp > 0 ? timestamp : QDateTime::currentMSecsSinceEpoch());
    jsonObj["width"] = width;
    jsonObj["height"] = height;
    jsonObj["format"] = QString::fromStdString(format);
    jsonObj["frameSize"] = static_cast<int>(frameData.size());
    jsonObj["fps"] = fps;
    jsonObj["type"] = "video";

    return packMessage(MsgType::VIDEO_FRAME, jsonObj, frameData);
}

QByteArray ProtocolPackager::packAudioFrame(uint32_t roomId,
                                          const QByteArray& audioData,
                                          uint64_t timestamp,
                                          const AudioFormatInfo& format) {
    QJsonObject jsonObj;
    jsonObj["roomId"] = static_cast<int>(roomId);
    jsonObj["ts"] = static_cast<qint64>(timestamp > 0 ? timestamp : QDateTime::currentMSecsSinceEpoch());
    jsonObj["sampleRate"] = format.sampleRate;
    jsonObj["channelCount"] = format.channelCount;
    jsonObj["sampleSize"] = format.sampleSize;
    jsonObj["codec"] = format.codec;
    jsonObj["sampleType"] = static_cast<int>(format.sampleType);
    jsonObj["frameSize"] = static_cast<int>(audioData.size());
    jsonObj["type"] = "audio";

    return packMessage(MsgType::AUDIO_FRAME, jsonObj, audioData);
}


QByteArray ProtocolPackager::packMessage(MsgType type,
                                       const QJsonObject& jsonData,
                                       const QByteArray& binaryData) {
    // 将JSON对象转换为字节数组
    QJsonDocument doc(jsonData);
    QByteArray jsonBytes = doc.toJson(QJsonDocument::Compact);

    // 计算总长度: type(2) + jsonSize(4) + jsonBytes + binaryData
    uint32_t totalLength = 2 + 4 + jsonBytes.size() + binaryData.size();

    // 创建数据包
    QByteArray packet;
    packet.reserve(4 + totalLength); // 预留空间: length(4) + totalLength

    // 添加length字段（大端序）
    uint32_t netLength = htonl(totalLength);
    packet.append(reinterpret_cast<const char*>(&netLength), 4);

    // 添加type字段（大端序）
    uint16_t netType = htons(static_cast<uint16_t>(type));
    packet.append(reinterpret_cast<const char*>(&netType), 2);

    // 添加jsonSize字段（大端序）
    uint32_t netJsonSize = htonl(static_cast<uint32_t>(jsonBytes.size()));
    packet.append(reinterpret_cast<const char*>(&netJsonSize), 4);

    // 添加JSON数据
    packet.append(jsonBytes);

    // 添加二进制数据（如果有）
    if (!binaryData.isEmpty()) {
        packet.append(binaryData);
    }

    return packet;
}

bool ProtocolPackager::unpackMessage(const QByteArray& data,
                                   MsgType& type,
                                   QJsonObject& jsonData,
                                   QByteArray& binaryData) {
    if (data.size() < 10) { // 最小长度: length(4) + type(2) + jsonSize(4) = 10
        return false;
    }

    const char* ptr = data.constData();

    // 读取并验证length
    uint32_t netLength;
    memcpy(&netLength, ptr, 4);
    uint32_t length = ntohl(netLength);

    if (data.size() < static_cast<int>(length + 4)) {
        return false;
    }

    // 读取type
    uint16_t netType;
    memcpy(&netType, ptr + 4, 2);
    type = static_cast<MsgType>(ntohs(netType));

    // 读取jsonSize
    uint32_t netJsonSize;
    memcpy(&netJsonSize, ptr + 6, 4);
    uint32_t jsonSize = ntohl(netJsonSize);

    // 读取JSON数据
    if (jsonSize > 0) {
        QByteArray jsonBytes(ptr + 10, jsonSize);
        QJsonDocument doc = QJsonDocument::fromJson(jsonBytes);
        if (doc.isObject()) {
            jsonData = doc.object();
        } else {
            return false;
        }
    }

    // 读取二进制数据
    uint32_t binaryStart = 10 + jsonSize;
    uint32_t binarySize = length - (2 + 4 + jsonSize);
    if (binarySize > 0) {
        binaryData = QByteArray(ptr + binaryStart, binarySize);
    }

    return true;
}
QByteArray ProtocolPackager::packControlCommand(MsgType commandType, uint32_t roomId, const QJsonObject& extraData) {
    QJsonObject jsonObj;
    jsonObj["roomId"] = static_cast<int>(roomId);
    jsonObj["ts"] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
    jsonObj["command"] = QString::number(static_cast<uint16_t>(commandType));

    // 添加额外数据
    if (!extraData.isEmpty()) {
        jsonObj["extra"] = extraData;
    }

    return packMessage(commandType, jsonObj);
}

bool ProtocolPackager::parseScreenFrameInfo(const QJsonObject& jsonData,
                                          uint32_t& roomId,
                                          uint64_t& timestamp,
                                          int& width,
                                          int& height,
                                          std::string& format,
                                          int& frameSize,
                                          int& fps,
                                          ScreenCaptureMode& mode,
                                          QRect& area) {
    try {
        if (jsonData.contains("roomId") && jsonData["roomId"].isDouble()) {
            roomId = static_cast<uint32_t>(jsonData["roomId"].toInt());
        } else {
            return false;
        }

        if (jsonData.contains("ts") && jsonData["ts"].isDouble()) {
            timestamp = static_cast<uint64_t>(jsonData["ts"].toDouble());
        } else {
            return false;
        }

        if (jsonData.contains("width") && jsonData["width"].isDouble()) {
            width = jsonData["width"].toInt();
        } else {
            width = 0;
        }

        if (jsonData.contains("height") && jsonData["height"].isDouble()) {
            height = jsonData["height"].toInt();
        } else {
            height = 0;
        }

        if (jsonData.contains("format") && jsonData["format"].isString()) {
            format = jsonData["format"].toString().toStdString();
        } else {
            format = "unknown";
        }

        if (jsonData.contains("frameSize") && jsonData["frameSize"].isDouble()) {
            frameSize = jsonData["frameSize"].toInt();
        } else {
            frameSize = 0;
        }

        if (jsonData.contains("fps") && jsonData["fps"].isDouble()) {
            fps = jsonData["fps"].toInt();
        } else {
            fps = 0;
        }

        if (jsonData.contains("mode") && jsonData["mode"].isDouble()) {
            mode = static_cast<ScreenCaptureMode>(jsonData["mode"].toInt());
        } else {
            mode = ScreenCaptureMode::FULL_SCREEN;
        }

        // 解析区域信息
        if (jsonData.contains("area") && jsonData["area"].isObject()) {
            QJsonObject areaObj = jsonData["area"].toObject();
            int x = areaObj["x"].toInt();
            int y = areaObj["y"].toInt();
            int w = areaObj["width"].toInt();
            int h = areaObj["height"].toInt();
            area = QRect(x, y, w, h);
        } else {
            area = QRect();
        }

        return true;
    } catch (const std::exception& e) {
        qWarning() << "parseScreenFrameInfo exception:" << e.what();
        return false;
    } catch (...) {
        qWarning() << "parseScreenFrameInfo unknown exception";
        return false;
    }
}

QByteArray ProtocolPackager::packScreenFrame(uint32_t roomId,
                                           const QByteArray& frameData,
                                           uint64_t timestamp,
                                           int width,
                                           int height,
                                           const std::string& format,
                                           int fps,
                                           ScreenCaptureMode mode,
                                           const QRect& area) {
    QJsonObject jsonObj;
    jsonObj["roomId"] = static_cast<int>(roomId);
    jsonObj["ts"] = static_cast<qint64>(timestamp > 0 ? timestamp : QDateTime::currentMSecsSinceEpoch());
    jsonObj["width"] = width;
    jsonObj["height"] = height;
    jsonObj["format"] = QString::fromStdString(format);
    jsonObj["frameSize"] = static_cast<int>(frameData.size());
    jsonObj["fps"] = fps;
    jsonObj["type"] = "screen";
    jsonObj["mode"] = static_cast<int>(mode);

    // 添加区域信息
    if (mode == ScreenCaptureMode::SELECTED_AREA && !area.isNull()) {
        QJsonObject areaObj;
        areaObj["x"] = area.x();
        areaObj["y"] = area.y();
        areaObj["width"] = area.width();
        areaObj["height"] = area.height();
        jsonObj["area"] = areaObj;
    }

    return packMessage(MsgType::SCREEN_FRAME, jsonObj, frameData);
}

bool ProtocolPackager::parseVideoFrameInfo(const QJsonObject& jsonData,
                                         uint32_t& roomId,
                                         uint64_t& timestamp,
                                         int& width,
                                         int& height,
                                         std::string& format,
                                         int& frameSize,
                                         int& fps) {
    try {
        if (jsonData.contains("roomId") && jsonData["roomId"].isDouble()) {
            roomId = static_cast<uint32_t>(jsonData["roomId"].toInt());
        } else {
            return false;
        }

        if (jsonData.contains("ts") && jsonData["ts"].isDouble()) {
            timestamp = static_cast<uint64_t>(jsonData["ts"].toDouble());
        } else {
            return false;
        }

        if (jsonData.contains("width") && jsonData["width"].isDouble()) {
            width = jsonData["width"].toInt();
        } else {
            width = 0;
        }

        if (jsonData.contains("height") && jsonData["height"].isDouble()) {
            height = jsonData["height"].toInt();
        } else {
            height = 0;
        }

        if (jsonData.contains("format") && jsonData["format"].isString()) {
            format = jsonData["format"].toString().toStdString();
        } else {
            format = "unknown";
        }

        if (jsonData.contains("frameSize") && jsonData["frameSize"].isDouble()) {
            frameSize = jsonData["frameSize"].toInt();
        } else {
            frameSize = 0;
        }

        if (jsonData.contains("fps") && jsonData["fps"].isDouble()) {
            fps = jsonData["fps"].toInt();
        } else {
            fps = 0;
        }

        return true;
    } catch (const std::exception& e) {
        qWarning() << "parseVideoFrameInfo exception:" << e.what();
        return false;
    } catch (...) {
        qWarning() << "parseVideoFrameInfo unknown exception";
        return false;
    }
}
bool ProtocolPackager::parseAudioFrameInfo(const QJsonObject& jsonData,
                                         uint32_t& roomId,
                                         uint64_t& timestamp,
                                         AudioFormatInfo& format,
                                         int& frameSize) {
    try {
        if (jsonData.contains("roomId") && jsonData["roomId"].isDouble()) {
            roomId = static_cast<uint32_t>(jsonData["roomId"].toInt());
        } else {
            return false;
        }

        if (jsonData.contains("ts") && jsonData["ts"].isDouble()) {
            timestamp = static_cast<uint64_t>(jsonData["ts"].toDouble());
        } else {
            return false;
        }

        if (jsonData.contains("sampleRate") && jsonData["sampleRate"].isDouble()) {
            format.sampleRate = jsonData["sampleRate"].toInt();
        } else {
            format.sampleRate = 44100;
        }

        if (jsonData.contains("channelCount") && jsonData["channelCount"].isDouble()) {
            format.channelCount = jsonData["channelCount"].toInt();
        } else {
            format.channelCount = 2;
        }

        if (jsonData.contains("sampleSize") && jsonData["sampleSize"].isDouble()) {
            format.sampleSize = jsonData["sampleSize"].toInt();
        } else {
            format.sampleSize = 16;
        }

        if (jsonData.contains("codec") && jsonData["codec"].isString()) {
            format.codec = jsonData["codec"].toString();
        } else {
            format.codec = "audio/pcm";
        }

        if (jsonData.contains("sampleType") && jsonData["sampleType"].isDouble()) {
            format.sampleType = static_cast<QAudioFormat::SampleType>(jsonData["sampleType"].toInt());
        } else {
            format.sampleType = QAudioFormat::SignedInt;
        }

        if (jsonData.contains("frameSize") && jsonData["frameSize"].isDouble()) {
            frameSize = jsonData["frameSize"].toInt();
        } else {
            frameSize = 0;
        }

        return true;
    } catch (const std::exception& e) {
        qWarning() << "parseAudioFrameInfo exception:" << e.what();
        return false;
    } catch (...) {
        qWarning() << "parseAudioFrameInfo unknown exception";
        return false;
    }
}
QByteArray ProtocolPackager::packTextMessage(uint32_t roomId,
                                           uint32_t userId,
                                           const QString& userName,
                                           const QString& content,
                                           TextMessageType type,
                                           uint64_t timestamp) {
    QJsonObject jsonObj;
    jsonObj["roomId"] = static_cast<int>(roomId);
    jsonObj["userId"] = static_cast<int>(userId);
    jsonObj["userName"] = userName;
    jsonObj["content"] = content;
    jsonObj["type"] = static_cast<int>(type);
    jsonObj["ts"] = static_cast<qint64>(timestamp > 0 ? timestamp : QDateTime::currentMSecsSinceEpoch());

    return packMessage(MsgType::TEXT_MESSAGE, jsonObj);
}
QByteArray ProtocolPackager::packSystemMessage(uint32_t roomId,
                                             const QString& content,
                                             uint64_t timestamp) {
    QJsonObject jsonObj;
    jsonObj["roomId"] = static_cast<int>(roomId);
    jsonObj["content"] = content;
    jsonObj["type"] = static_cast<int>(TextMessageType::SYSTEM);
    jsonObj["ts"] = static_cast<qint64>(timestamp > 0 ? timestamp : QDateTime::currentMSecsSinceEpoch());

    return packMessage(MsgType::SYSTEM_MESSAGE, jsonObj);
}

bool ProtocolPackager::parseTextMessage(const QJsonObject& jsonData,
                                      TextMessage& message) {
    try {
        if (jsonData.contains("roomId") && jsonData["roomId"].isDouble()) {
            message.roomId = static_cast<uint32_t>(jsonData["roomId"].toInt());
        } else {
            return false;
        }

        if (jsonData.contains("userId") && jsonData["userId"].isDouble()) {
            message.userId = static_cast<uint32_t>(jsonData["userId"].toInt());
        } else {
            message.userId = 0;
        }

        if (jsonData.contains("userName") && jsonData["userName"].isString()) {
            message.userName = jsonData["userName"].toString();
        } else {
            message.userName = "未知用户";
        }

        if (jsonData.contains("content") && jsonData["content"].isString()) {
            message.content = jsonData["content"].toString();
        } else {
            return false;
        }

        if (jsonData.contains("type") && jsonData["type"].isDouble()) {
            message.type = static_cast<TextMessageType>(jsonData["type"].toInt());
        } else {
            message.type = TextMessageType::NORMAL;
        }

        if (jsonData.contains("ts") && jsonData["ts"].isDouble()) {
            qint64 ts = static_cast<qint64>(jsonData["ts"].toDouble());
            message.timestamp = QDateTime::fromMSecsSinceEpoch(ts);
        } else {
            message.timestamp = QDateTime::currentDateTime();
        }

        return true;
    } catch (const std::exception& e) {
        qWarning() << "parseTextMessage exception:" << e.what();
        return false;
    } catch (...) {
        qWarning() << "parseTextMessage unknown exception";
        return false;
    }
}

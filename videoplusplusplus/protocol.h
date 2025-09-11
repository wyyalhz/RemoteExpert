// ===============================================
// common/protocol.h
// 添加文字消息支持
// ===============================================

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QVariant>
#include <QDebug>
#include <QAudioFormat>
#include <QRect>
#include <QDateTime>

// 消息类型枚举
enum class MsgType : uint16_t {
    VIDEO_FRAME = 1,    // 视频帧
    AUDIO_FRAME = 2,    // 音频帧
    CONTROL_CMD = 3,    // 控制命令
    HEARTBEAT   = 4,    // 心跳包
    REQUEST_AV  = 5,    // 请求音视频流
    STOP_AV     = 6,    // 停止音视频流
    SCREEN_FRAME = 7,   // 屏幕帧
    REQUEST_SCREEN = 8, // 请求屏幕共享
    STOP_SCREEN = 9,    // 停止屏幕共享
    TEXT_MESSAGE = 10,  // 文字消息
    SYSTEM_MESSAGE = 11,// 系统消息
    RECORD_START = 12,  // 开始录制
    RECORD_STOP = 13,   // 停止录制
    RECORD_DATA = 14    // 录制数据
};

// 文字消息类型
enum class TextMessageType {
    NORMAL = 0,         // 普通消息
    SYSTEM = 1,         // 系统消息
    NOTICE = 2          // 通知消息
};

// 屏幕捕获模式
enum class ScreenCaptureMode {
    FULL_SCREEN = 0,    // 全屏捕获
    ACTIVE_WINDOW = 1,  // 活动窗口
    SELECTED_AREA = 2   // 选择区域
};

// 音频格式信息结构
struct AudioFormatInfo {
    int sampleRate;
    int channelCount;
    int sampleSize;
    QString codec;
    QAudioFormat::SampleType sampleType;

    AudioFormatInfo() : sampleRate(44100), channelCount(2), sampleSize(16),
                       codec("audio/pcm"), sampleType(QAudioFormat::SignedInt) {}
};

// 屏幕捕获配置
struct ScreenCaptureConfig {
    ScreenCaptureMode mode;
    QRect captureArea;
    int fps;
    bool includeCursor;

    ScreenCaptureConfig() : mode(ScreenCaptureMode::FULL_SCREEN), fps(10), includeCursor(true) {}
};

// 文字消息结构
struct TextMessage {
    uint32_t roomId;
    uint32_t userId;
    QString userName;
    QString content;
    QDateTime timestamp;
    TextMessageType type;

    TextMessage() : roomId(0), userId(0), type(TextMessageType::NORMAL) {}
};

// 协议打包类
class ProtocolPackager {
public:
    // 打包视频帧消息
    static QByteArray packVideoFrame(uint32_t roomId,
                                   const QByteArray& frameData,
                                   uint64_t timestamp = 0,
                                   int width = 0,
                                   int height = 0,
                                   const std::string& format = "jpeg",
                                   int fps = 0);

    // 打包屏幕帧消息
    static QByteArray packScreenFrame(uint32_t roomId,
                                    const QByteArray& frameData,
                                    uint64_t timestamp = 0,
                                    int width = 0,
                                    int height = 0,
                                    const std::string& format = "jpeg",
                                    int fps = 0,
                                    ScreenCaptureMode mode = ScreenCaptureMode::FULL_SCREEN,
                                    const QRect& area = QRect());

    // 打包音频帧消息
    static QByteArray packAudioFrame(uint32_t roomId,
                                   const QByteArray& audioData,
                                   uint64_t timestamp = 0,
                                   const AudioFormatInfo& format = AudioFormatInfo());

    // 打包文字消息
    static QByteArray packTextMessage(uint32_t roomId,
                                    uint32_t userId,
                                    const QString& userName,
                                    const QString& content,
                                    TextMessageType type = TextMessageType::NORMAL,
                                    uint64_t timestamp = 0);

    // 打包系统消息
    static QByteArray packSystemMessage(uint32_t roomId,
                                      const QString& content,
                                      uint64_t timestamp = 0);

    // 打包控制命令
    static QByteArray packControlCommand(MsgType commandType, uint32_t roomId, const QJsonObject& extraData = QJsonObject());

    // 打包通用消息
    static QByteArray packMessage(MsgType type,
                                const QJsonObject& jsonData,
                                const QByteArray& binaryData = QByteArray());

    // 拆包消息
    static bool unpackMessage(const QByteArray& data,
                            MsgType& type,
                            QJsonObject& jsonData,
                            QByteArray& binaryData);

    // 解析视频帧信息
    static bool parseVideoFrameInfo(const QJsonObject& jsonData,
                                  uint32_t& roomId,
                                  uint64_t& timestamp,
                                  int& width,
                                  int& height,
                                  std::string& format,
                                  int& frameSize,
                                  int& fps);

    // 解析屏幕帧信息
    static bool parseScreenFrameInfo(const QJsonObject& jsonData,
                                   uint32_t& roomId,
                                   uint64_t& timestamp,
                                   int& width,
                                   int& height,
                                   std::string& format,
                                   int& frameSize,
                                   int& fps,
                                   ScreenCaptureMode& mode,
                                   QRect& area);

    // 解析音频帧信息
    static bool parseAudioFrameInfo(const QJsonObject& jsonData,
                                  uint32_t& roomId,
                                  uint64_t& timestamp,
                                  AudioFormatInfo& format,
                                  int& frameSize);

    // 解析文字消息
    static bool parseTextMessage(const QJsonObject& jsonData,
                               TextMessage& message);

private:
    // 字节序转换（大端序）
    static uint32_t htonl(uint32_t hostlong);
    static uint16_t htons(uint16_t hostshort);
    static uint32_t ntohl(uint32_t netlong);
    static uint16_t ntohs(uint16_t netshort);
};

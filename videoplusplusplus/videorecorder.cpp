// ===============================================
// recorder/video_recorder.cpp
// 视频录制器实现
// ===============================================

#include "videorecorder.h"
#include <QDir>
#include <QJsonDocument>
#include <QDebug>
#include <QTimer>
#include <QBuffer>
#include <QJsonArray>

VideoRecorder::VideoRecorder(QObject* parent)
    : QObject(parent)
    , m_recordFile(nullptr)
    , m_isRecording(false)
    , m_videoFrameCount(0)
    , m_audioFrameCount(0)
    , m_screenFrameCount(0)
    , m_textMessageCount(0)
    , m_totalDataSize(0)
{
    m_statsTimer = new QTimer(this);
    m_statsTimer->setInterval(1000); // 1秒更新一次统计
    connect(m_statsTimer, &QTimer::timeout, this, &VideoRecorder::updateStats);
}

VideoRecorder::~VideoRecorder() {
    stopRecording();
}

bool VideoRecorder::startRecording(const QString& basePath) {
    if (m_isRecording) {
        stopRecording();
    }

    QString filePath = generateFileName();
    if (!basePath.isEmpty()) {
        QDir dir(basePath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        filePath = dir.filePath(QFileInfo(filePath).fileName());
    }

    m_recordFile = new QFile(filePath);
    if (!m_recordFile->open(QIODevice::WriteOnly)) {
        emit recordingError("无法创建录制文件: " + filePath);
        delete m_recordFile;
        m_recordFile = nullptr;
        return false;
    }

    m_isRecording = true;
    m_currentFilePath = filePath;
    m_startTime = QDateTime::currentDateTime();
    m_videoFrameCount = 0;
    m_audioFrameCount = 0;
    m_screenFrameCount = 0;
    m_textMessageCount = 0;
    m_totalDataSize = 0;

    if (!writeHeader()) {
        stopRecording();
        return false;
    }

    m_statsTimer->start();
    emit recordingStarted(filePath);

    qDebug() << "Recording started:" << filePath;
    return true;
}

void VideoRecorder::stopRecording() {
    if (!m_isRecording) return;

    m_statsTimer->stop();

    if (m_recordFile) {
        writeTrailer();
        m_recordFile->close();

        qint64 duration = recordingDuration();
        qint64 size = m_recordFile->size();

        delete m_recordFile;
        m_recordFile = nullptr;

        emit recordingStopped(m_currentFilePath, duration, size);

        qDebug() << "Recording stopped. Duration:" << duration << "ms, Size:" << size << "bytes";
    }

    m_isRecording = false;
}

void VideoRecorder::recordVideoFrame(const QImage& frame, uint32_t roomId, uint64_t timestamp) {
    if (!m_isRecording) return;

    QByteArray frameData;
    QBuffer buffer(&frameData);
    buffer.open(QIODevice::WriteOnly);
    frame.save(&buffer, "JPEG", 80);

    QJsonObject metadata = createFrameMetadata(roomId, timestamp, frame.size());
    metadata["type"] = "video";
    metadata["fps"] = 0; // 将在后期处理时计算

    if (writeFrame(MsgType::MSG_VIDEO_FRAME, frameData, metadata)) {
        m_videoFrameCount++;
        m_totalDataSize += frameData.size();
    }
}

void VideoRecorder::recordAudioFrame(const QByteArray& audioData, const AudioFormatInfo& format, uint32_t roomId, uint64_t timestamp) {
    if (!m_isRecording) return;

    QJsonObject metadata = createFrameMetadata(roomId, timestamp);
    metadata["type"] = "audio";
    metadata["sampleRate"] = format.sampleRate;
    metadata["channelCount"] = format.channelCount;
    metadata["sampleSize"] = format.sampleSize;

    if (writeFrame(MsgType::MSG_AUDIO_FRAME, audioData, metadata)) {
        m_audioFrameCount++;
        m_totalDataSize += audioData.size();
    }
}

void VideoRecorder::recordScreenFrame(const QImage& frame, uint32_t roomId, uint64_t timestamp, ScreenCaptureMode mode) {
    if (!m_isRecording) return;

    QByteArray frameData;
    QBuffer buffer(&frameData);
    buffer.open(QIODevice::WriteOnly);
    frame.save(&buffer, "JPEG", 80);

    QJsonObject metadata = createFrameMetadata(roomId, timestamp, frame.size());
    metadata["type"] = "screen";
    metadata["captureMode"] = static_cast<int>(mode);
    metadata["fps"] = 0;

    if (writeFrame(MsgType::MSG_SCREEN_FRAME, frameData, metadata)) {
        m_screenFrameCount++;
        m_totalDataSize += frameData.size();
    }
}

void VideoRecorder::recordTextMessage(const TextMessage& message) {
    if (!m_isRecording) return;

    QJsonObject metadata;
    metadata["roomId"] = static_cast<int>(message.roomId);
    metadata["userId"] = static_cast<int>(message.userId);
    metadata["userName"] = message.userName;
    metadata["content"] = message.content;
    metadata["timestamp"] = static_cast<qint64>(message.timestamp.toMSecsSinceEpoch());
    metadata["messageType"] = static_cast<int>(message.type);

    QJsonDocument doc(metadata);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    if (writeFrame(MsgType::MSG_TEXT, jsonData)) {
        m_textMessageCount++;
        m_totalDataSize += jsonData.size();
    }
}

QString VideoRecorder::generateFileName() const {
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    return QString("meeting_recording_%1.mtr").arg(timestamp);
}

bool VideoRecorder::writeHeader() {
    QJsonObject header;
        header["version"] = "2.0";
        header["format"] = "Multi-Stream Meeting Recording";
        header["startTime"] = static_cast<qint64>(m_startTime.toMSecsSinceEpoch());
        header["creator"] = "AV Chat System";
        header["streamTypes"] = QJsonArray::fromStringList({"local_video", "local_screen", "remote_video", "remote_screen"});

    QJsonDocument doc(header);
    QByteArray headerData = doc.toJson(QJsonDocument::Compact);

    // 写入头部长度和头部数据
    quint32 headerLength = static_cast<quint32>(headerData.size());
    QByteArray lengthData(reinterpret_cast<const char*>(&headerLength), sizeof(headerLength));

    QMutexLocker locker(&m_fileMutex);
    if (m_recordFile->write(lengthData) != sizeof(headerLength) ||
        m_recordFile->write(headerData) != headerData.size()) {
        return false;
    }

    return true;
}

bool VideoRecorder::writeFrame(MsgType type, const QByteArray& data, const QJsonObject& metadata) {
    QMutexLocker locker(&m_fileMutex);

    // 写入帧类型
    quint16 frameType = static_cast<quint16>(type);
    if (m_recordFile->write(reinterpret_cast<const char*>(&frameType), sizeof(frameType)) != sizeof(frameType)) {
        return false;
    }

    // 写入时间戳
    quint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    if (m_recordFile->write(reinterpret_cast<const char*>(&timestamp), sizeof(timestamp)) != sizeof(timestamp)) {
        return false;
    }

    // 写入元数据（如果有）
    QByteArray metaData;
    if (!metadata.isEmpty()) {
        QJsonDocument doc(metadata);
        metaData = doc.toJson(QJsonDocument::Compact);
    }

    quint32 metaSize = static_cast<quint32>(metaData.size());
    if (m_recordFile->write(reinterpret_cast<const char*>(&metaSize), sizeof(metaSize)) != sizeof(metaSize)) {
        return false;
    }

    if (metaSize > 0 && m_recordFile->write(metaData) != metaData.size()) {
        return false;
    }

    // 写入数据长度和数据
    quint32 dataSize = static_cast<quint32>(data.size());
    if (m_recordFile->write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize)) != sizeof(dataSize)) {
        return false;
    }

    if (m_recordFile->write(data) != data.size()) {
        return false;
    }

    return true;
}

bool VideoRecorder::writeTrailer() {
    QJsonObject trailer;
    trailer["endTime"] = static_cast<qint64>(QDateTime::currentDateTime().toMSecsSinceEpoch());
    trailer["totalSize"] = static_cast<qint64>(m_totalDataSize);

    // 添加流统计信息
    QJsonObject streamStats;
    QMutexLocker locker(&m_statsMutex);
    for (auto it = m_streamStats.constBegin(); it != m_streamStats.constEnd(); ++it) {
        streamStats[it.key()] = it.value();
    }
    trailer["streamStats"] = streamStats;

    return writeFrame(MsgType::MSG_RECORD_STOP, QByteArray(), trailer);
}

qint64 VideoRecorder::recordingDuration() const {
    if (!m_isRecording) return 0;
    return m_startTime.msecsTo(QDateTime::currentDateTime());
}

qint64 VideoRecorder::recordedSize() const {
    return m_totalDataSize;
}

void VideoRecorder::updateStats() {
    emit recordingStatsUpdated(recordingDuration(), m_totalDataSize,
                             m_videoFrameCount + m_screenFrameCount, m_audioFrameCount);
}

QJsonObject VideoRecorder::createFrameMetadata(uint32_t roomId, uint64_t timestamp, const QSize& size) {
    QJsonObject metadata;
    metadata["roomId"] = static_cast<int>(roomId);
    metadata["timestamp"] = static_cast<qint64>(timestamp);
    if (size.isValid()) {
        metadata["width"] = size.width();
        metadata["height"] = size.height();
    }
    return metadata;
}

void VideoRecorder::recordVideoFrame(const QImage& frame, uint32_t roomId, uint64_t timestamp,
                                   StreamType streamType, const QString& streamName) {
    if (!m_isRecording) return;

    QByteArray frameData;
    QBuffer buffer(&frameData);
    buffer.open(QIODevice::WriteOnly);
    frame.save(&buffer, "JPEG", 80);

    QJsonObject metadata = createFrameMetadata(roomId, timestamp, frame.size());
    metadata["type"] = "video";
    metadata["streamType"] = static_cast<int>(streamType);
    metadata["streamName"] = getStreamName(streamType, streamName);
    metadata["isScreen"] = (streamType == StreamType::LOCAL_SCREEN || streamType == StreamType::REMOTE_SCREEN);

    if (writeFrame(MsgType::MSG_VIDEO_FRAME, frameData, metadata)) {
        updateStreamStats(metadata["streamName"].toString());
        m_totalDataSize += frameData.size();
    }
}

void VideoRecorder::recordRemoteVideoFrame(const QImage& frame, uint32_t roomId, uint64_t timestamp,
                                         const QString& streamName, bool isScreen) {
    StreamType type = isScreen ? StreamType::REMOTE_SCREEN : StreamType::REMOTE_VIDEO;
    recordVideoFrame(frame, roomId, timestamp, type, streamName);
}

void VideoRecorder::recordLocalVideoFrame(const QImage& frame, uint32_t roomId, uint64_t timestamp,
                                        bool isScreen) {
    StreamType type = isScreen ? StreamType::LOCAL_SCREEN : StreamType::LOCAL_VIDEO;
    recordVideoFrame(frame, roomId, timestamp, type);
}

QString VideoRecorder::getStreamName(StreamType type, const QString& customName) const {
    if (!customName.isEmpty()) {
        return customName;
    }

    switch (type) {
    case StreamType::LOCAL_VIDEO: return "本地摄像头";
    case StreamType::LOCAL_SCREEN: return "本地屏幕";
    case StreamType::REMOTE_VIDEO: return "远程视频";
    case StreamType::REMOTE_SCREEN: return "远程屏幕";
    default: return "未知流";
    }
}

void VideoRecorder::updateStreamStats(const QString& streamName) {
    QMutexLocker locker(&m_statsMutex);
    m_streamStats[streamName]++;

    // 每10帧更新一次统计
    if (m_streamStats[streamName] % 10 == 0) {
        emit streamStatsUpdated(m_streamStats);
    }
}

QMap<QString, int> VideoRecorder::getStreamStats() const {
    QMutexLocker locker(&m_statsMutex);
    return m_streamStats;
}


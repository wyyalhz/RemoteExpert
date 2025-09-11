// ===============================================
// recorder/video_recorder.h
// 视频录制器
// ===============================================

#pragma once

#include <QObject>
#include <QFile>
#include <QDateTime>
#include <QImage>
#include <QAudioFormat>
#include <QMutex>
#include "protocol.h"

// 视频流类型
enum class StreamType {
    LOCAL_VIDEO = 0,    // 本地摄像头视频
    LOCAL_SCREEN = 1,   // 本地屏幕共享
    REMOTE_VIDEO = 2,   // 远程视频流
    REMOTE_SCREEN = 3   // 远程屏幕共享
};

class VideoRecorder : public QObject {
    Q_OBJECT

public:
    explicit VideoRecorder(QObject* parent = nullptr);
    ~VideoRecorder();

    bool startRecording(const QString& basePath = "");
    void stopRecording();
    bool isRecording() const { return m_isRecording; }

    void recordVideoFrame(const QImage& frame, uint32_t roomId, uint64_t timestamp);
    void recordAudioFrame(const QByteArray& audioData, const AudioFormatInfo& format, uint32_t roomId, uint64_t timestamp);
    void recordScreenFrame(const QImage& frame, uint32_t roomId, uint64_t timestamp, ScreenCaptureMode mode);
    void recordTextMessage(const TextMessage& message);

    QString currentRecordingPath() const { return m_currentFilePath; }
    qint64 recordingDuration() const;
    qint64 recordedSize() const;
    void recordVideoFrame(const QImage& frame, uint32_t roomId, uint64_t timestamp,
                         StreamType streamType, const QString& streamName = "");
    void recordRemoteVideoFrame(const QImage& frame, uint32_t roomId, uint64_t timestamp,
                               const QString& streamName, bool isScreen = false);
    void recordLocalVideoFrame(const QImage& frame, uint32_t roomId, uint64_t timestamp,
                              bool isScreen = false);

    // 获取录制统计
    QMap<QString, int> getStreamStats() const;


signals:
    void recordingStarted(const QString& filePath);
    void recordingStopped(const QString& filePath, qint64 duration, qint64 size);
    void recordingError(const QString& error);
    void recordingStatsUpdated(qint64 duration, qint64 size, int videoFrames, int audioFrames);
    void streamStatsUpdated(const QMap<QString, int>& stats);
private slots:
    void updateStats();

private:
    QFile* m_recordFile;
    mutable QMutex m_fileMutex;
    bool m_isRecording;
    QString m_currentFilePath;
    QDateTime m_startTime;

    int m_videoFrameCount;
    int m_audioFrameCount;
    int m_screenFrameCount;
    int m_textMessageCount;
    qint64 m_totalDataSize;

    QTimer* m_statsTimer;

    QString generateFileName() const;
    bool writeHeader();
    bool writeFrame(MsgType type, const QByteArray& data, const QJsonObject& metadata = QJsonObject());
    bool writeTrailer();
    QJsonObject createFrameMetadata(uint32_t roomId, uint64_t timestamp, const QSize& size = QSize());
    QMap<QString, int> m_streamStats; // 流统计: 流名称 -> 帧数
    mutable QMutex m_statsMutex;

    QString getStreamName(StreamType type, const QString& customName = "") const;
    void updateStreamStats(const QString& streamName);
};

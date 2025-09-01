// ===============================================
// receiver/av_receiver.h
// 音视频接收器头文件
// ===============================================

#ifndef AV_RECEIVER_H
#define AV_RECEIVER_H

#include <QObject>
#include <QLabel>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QImage>
#include "protocol.h"
#include "audioplayer.h"
#include "videorecorder.h"

class AVReceiver : public QObject
{
    Q_OBJECT

public:
    explicit AVReceiver(QLabel* videoDisplayLabel, QObject* parent = nullptr);
    ~AVReceiver();

    // 接收控制
    void startReceiving(uint32_t roomId = 0);
    void stopReceiving();
    bool isReceiving() const { return m_isReceiving; }

    // 数据处理
    void processReceivedData(const QByteArray& data);

    // 音频控制
    void setAudioEnabled(bool enabled);
    bool isAudioEnabled() const { return m_audioEnabled; }

    // 视频控制
    void setVideoEnabled(bool enabled);
    bool isVideoEnabled() const { return m_videoEnabled; }
    void setRecorder(VideoRecorder* recorder) { m_recorder = recorder; }
    VideoRecorder* recorder() const { return m_recorder; }


signals:
    // 视频相关信号
    void videoFrameReceived(const QImage& frame);
    void screenFrameReceived(const QImage& frame);

    // 状态信号
    void statusChanged(const QString& status);
    void errorOccurred(const QString& error);

    // 统计信号
    void videoStatsChanged(int fps, int totalFrames);
    void audioStatsChanged(int totalPackets);
    void screenStatsChanged(int fps, int totalFrames);

    // 文字消息信号
    void textMessageReceived(const TextMessage& message);
    void systemMessageReceived(const QString& message);

private slots:
    void updateDisplay();

private:
    // 显示控件
    QLabel* m_videoDisplayLabel;

    // 音频播放器
    AudioPlayer* m_audioPlayer;

    // 定时器
    QTimer* m_displayTimer;

    VideoRecorder* m_recorder;

    // 视频帧队列
    QQueue<QImage> m_videoFrameQueue;
    QMutex m_queueMutex;

    // 状态标志
    bool m_isReceiving;
    bool m_audioEnabled;
    bool m_videoEnabled;


    // 房间和统计信息
    uint32_t m_currentRoomId;
    int m_videoFps;
    int m_receivedVideoFrameCount;
    int m_receivedAudioFrameCount;
    int m_receivedScreenFrameCount;
    int m_receivedTextMessageCount;

    // 时间统计
    qint64 m_lastStatsUpdateTime;
    int m_lastVideoFrameCount;
    int m_lastScreenFrameCount;

    // 私有方法
    void displayFrame(const QImage& frame);
    void processVideoFrame(const QJsonObject& jsonData, const QByteArray& binaryData);
    void processAudioFrame(const QJsonObject& jsonData, const QByteArray& binaryData);
    void processScreenFrame(const QJsonObject& jsonData, const QByteArray& binaryData);
    void processTextMessage(const QJsonObject& jsonData, MsgType1 type);
    void updateStatistics();
    QString formatTimeDuration(qint64 milliseconds) const;
};

#endif // AV_RECEIVER_H

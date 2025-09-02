// ===============================================
// receiver/av_receiver.cpp
// 音视频接收器实现
// ===============================================

#include "avreceiver.h"
#include <QPainter>
#include <QDateTime>
#include <QDebug>
#include <QFont>
#include <QApplication>

AVReceiver::AVReceiver(QLabel* videoDisplayLabel, QObject* parent)
    : QObject(parent)
    , m_videoDisplayLabel(videoDisplayLabel)
    , m_audioPlayer(new AudioPlayer(this))
    , m_isReceiving(false)
    , m_audioEnabled(true)
    , m_videoEnabled(true)
    , m_currentRoomId(0)
    , m_videoFps(0)
    , m_receivedVideoFrameCount(0)
    , m_receivedAudioFrameCount(0)
    , m_receivedScreenFrameCount(0)
    , m_receivedTextMessageCount(0)
    , m_lastStatsUpdateTime(0)
    , m_lastVideoFrameCount(0)
    , m_lastScreenFrameCount(0)
{
    m_displayTimer = new QTimer(this);
    m_displayTimer->setInterval(1000); // 1秒更新一次显示
    connect(m_displayTimer, &QTimer::timeout, this, &AVReceiver::updateDisplay);
}

AVReceiver::~AVReceiver() {
    stopReceiving();
}

void AVReceiver::startReceiving(uint32_t roomId) {
    if (m_isReceiving) {
        stopReceiving();
    }

    m_currentRoomId = roomId;
    m_isReceiving = true;
    m_receivedVideoFrameCount = 0;
    m_receivedAudioFrameCount = 0;
    m_receivedScreenFrameCount = 0;
    m_receivedTextMessageCount = 0;
    m_videoFps = 0;
    m_lastStatsUpdateTime = QDateTime::currentMSecsSinceEpoch();
    m_lastVideoFrameCount = 0;
    m_lastScreenFrameCount = 0;

    m_displayTimer->start();

    if (m_audioEnabled) {
        m_audioPlayer->startPlayback();
    }

    QString status = QString("开始接收房间 %1 的音视频流").arg(roomId);
    if (!m_audioEnabled) status += " (音频已禁用)";
    if (!m_videoEnabled) status += " (视频已禁用)";

    emit statusChanged(status);
    emit systemMessageReceived(status);

    qDebug() << "AVReceiver started for room" << roomId;
}

void AVReceiver::stopReceiving() {
    m_isReceiving = false;
    m_displayTimer->stop();
    m_audioPlayer->stopPlayback();

    QMutexLocker locker(&m_queueMutex);
    m_videoFrameQueue.clear();

    if (m_videoDisplayLabel) {
        m_videoDisplayLabel->setText("音视频流已停止");
        m_videoDisplayLabel->setStyleSheet("border: 2px solid gray; background-color: black; color: white;");
    }

    QString status = "音视频流接收已停止";
    emit statusChanged(status);
    emit systemMessageReceived(status);

    qDebug() << "AVReceiver stopped";
}

void AVReceiver::setAudioEnabled(bool enabled) {
    m_audioEnabled = enabled;
    if (m_isReceiving) {
        if (enabled) {
            m_audioPlayer->startPlayback();
        } else {
            m_audioPlayer->stopPlayback();
        }
        emit statusChanged(QString("音频 %1").arg(enabled ? "已启用" : "已禁用"));
    }
}

void AVReceiver::setVideoEnabled(bool enabled) {
    m_videoEnabled = enabled;
    if (!enabled && m_videoDisplayLabel) {
        m_videoDisplayLabel->setText("视频已禁用");
    }
    emit statusChanged(QString("视频 %1").arg(enabled ? "已启用" : "已禁用"));
}

void AVReceiver::processReceivedData(const QByteArray& data) {
    if (!m_isReceiving) {
        return;
    }

    try {
        MsgType type;
        QJsonObject jsonData;
        QByteArray binaryData;

        if (!ProtocolPackager::unpackMessage(data, type, jsonData, binaryData)) {
            emit errorOccurred("协议解析失败");
            return;
        }

        switch (type) {
        case MsgType::MSG_VIDEO_FRAME:
            if (m_videoEnabled) {
                processVideoFrame(jsonData, binaryData);
            }
            break;
        case MsgType::MSG_AUDIO_FRAME:
            if (m_audioEnabled) {
                processAudioFrame(jsonData, binaryData);
            }
            break;
        case MsgType::MSG_SCREEN_FRAME:
            if (m_videoEnabled) {
                processScreenFrame(jsonData, binaryData);
            }
            break;
        case MsgType::MSG_TEXT:
        case MsgType::MSG_SYSTEM_MESSAGE:
            processTextMessage(jsonData, type);
            break;
        default:
            qDebug() << "Received unknown message type:" << static_cast<int>(type);
            break;
        }

        // 更新统计信息
        updateStatistics();

    } catch (const std::exception& e) {
        emit errorOccurred(QString("处理数据时出错: %1").arg(e.what()));
    }
}

void AVReceiver::processVideoFrame(const QJsonObject& jsonData, const QByteArray& binaryData) {
    uint32_t roomId;
    uint64_t timestamp;
    int width, height, frameSize, fps;
    std::string format;

    if (ProtocolPackager::parseVideoFrameInfo(jsonData, roomId, timestamp,
                                            width, height, format, frameSize, fps)) {

        if (m_currentRoomId != 0 && roomId != m_currentRoomId) {
            return;
        }

        QImage frame;
        if (format == "jpeg" || format == "jpg") {
            frame.loadFromData(binaryData, "JPEG");
        } else {
            frame.loadFromData(binaryData);
        }

        if (!frame.isNull()) {
            QMutexLocker locker(&m_queueMutex);

            if (fps > 0) {
                m_videoFps = fps;
            }

            m_videoFrameQueue.enqueue(frame);
            m_receivedVideoFrameCount++;

            if (m_videoFrameQueue.size() > m_videoFps * 2) {
                m_videoFrameQueue.dequeue();
            }

            displayFrame(frame);
            emit videoFrameReceived(frame);

            if (m_recorder && m_recorder->isRecording()) {
                m_recorder->recordVideoFrame(frame, roomId, timestamp);
            }
            if (m_recorder && m_recorder->isRecording()) {
                        m_recorder->recordRemoteVideoFrame(frame, roomId, timestamp, "远程用户视频", false);
                    }
        }
    }
}

void AVReceiver::processAudioFrame(const QJsonObject& jsonData, const QByteArray& binaryData) {
    uint32_t roomId;
    uint64_t timestamp;
    AudioFormatInfo format;
    int frameSize;

    if (ProtocolPackager::parseAudioFrameInfo(jsonData, roomId, timestamp, format, frameSize)) {

        if (m_currentRoomId != 0 && roomId != m_currentRoomId) {
            return;
        }

        m_receivedAudioFrameCount++;
        m_audioPlayer->processAudioData(binaryData, format);
    }
    if (m_recorder && m_recorder->isRecording()) {
           m_recorder->recordAudioFrame(binaryData, format, roomId, timestamp);
       }
}

void AVReceiver::processScreenFrame(const QJsonObject& jsonData, const QByteArray& binaryData) {
    uint32_t roomId;
    uint64_t timestamp;
    int width, height, frameSize, fps;
    std::string format;
    ScreenCaptureMode mode;
    QRect area;

    if (ProtocolPackager::parseScreenFrameInfo(jsonData, roomId, timestamp,
                                             width, height, format, frameSize, fps, mode, area)) {

        if (m_currentRoomId != 0 && roomId != m_currentRoomId) {
            return;
        }

        QImage frame;
        if (format == "jpeg" || format == "jpg") {
            frame.loadFromData(binaryData, "JPEG");
        } else {
            frame.loadFromData(binaryData);
        }

        if (!frame.isNull()) {
            QMutexLocker locker(&m_queueMutex);

            if (fps > 0) {
                m_videoFps = fps;
            }

            // 在屏幕上添加屏幕共享标识
            QPainter painter(&frame);
            painter.setPen(Qt::red);
            painter.setFont(QFont("Arial", 16));
            painter.drawText(10, 30, "屏幕共享");

            // 添加模式信息
            QString modeText;
            switch (mode) {
            case ScreenCaptureMode::FULL_SCREEN:
                modeText = "全屏";
                break;
            case ScreenCaptureMode::ACTIVE_WINDOW:
                modeText = "活动窗口";
                break;
            case ScreenCaptureMode::SELECTED_AREA:
                modeText = QString("区域: %1x%2").arg(area.width()).arg(area.height());
                break;
            }
            painter.drawText(10, 60, modeText);
            painter.end();

            m_videoFrameQueue.enqueue(frame);
            m_receivedScreenFrameCount++;

            if (m_videoFrameQueue.size() > m_videoFps * 2) {
                m_videoFrameQueue.dequeue();
            }

            displayFrame(frame);
            emit screenFrameReceived(frame);
            if (m_recorder && m_recorder->isRecording()) {
                        m_recorder->recordScreenFrame(frame, roomId, timestamp, mode);
                    }

        }
    }
}

void AVReceiver::processTextMessage(const QJsonObject& jsonData, MsgType type) {
    TextMessage message;

    if (ProtocolPackager::parseTextMessage(jsonData, message)) {
        if (m_currentRoomId != 0 && message.roomId != m_currentRoomId) {
            return;
        }

        // 如果是系统消息，设置正确的类型
        if (type == MsgType::MSG_SYSTEM_MESSAGE) {
            message.type = TextMessageType::SYSTEM;
        }

        m_receivedTextMessageCount++;
        emit textMessageReceived(message);

        // 更新状态显示
        if (message.type == TextMessageType::SYSTEM) {
            emit statusChanged(QString("系统消息: %1").arg(message.content));
        } else {
            emit statusChanged(QString("收到来自 %1 的消息").arg(message.userName));
        }
        if (m_recorder && m_recorder->isRecording()) {
                   m_recorder->recordTextMessage(message);
               }
    }
}

void AVReceiver::displayFrame(const QImage& frame) {
    if (!m_videoDisplayLabel || frame.isNull()) {
        return;
    }

    QImage scaledFrame = frame.scaled(m_videoDisplayLabel->size(),
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);

    QPixmap pixmap = QPixmap::fromImage(scaledFrame);

    // 在图像上绘制统计信息
    QPainter painter(&pixmap);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制半透明背景
    painter.fillRect(5, 5, 250, 100, QColor(0, 0, 0, 128));

    QString info = QString("视频: %1 FPS\n帧数: %2\n音频包: %3\n消息: %4\n分辨率: %5x%6")
                  .arg(m_videoFps)
                  .arg(m_receivedVideoFrameCount + m_receivedScreenFrameCount)
                  .arg(m_receivedAudioFrameCount)
                  .arg(m_receivedTextMessageCount)
                  .arg(frame.width())
                  .arg(frame.height());

    painter.drawText(10, 20, info);
    painter.end();

    m_videoDisplayLabel->setPixmap(pixmap);
}

void AVReceiver::updateDisplay() {
    QMutexLocker locker(&m_queueMutex);
    if (!m_videoFrameQueue.isEmpty()) {
        displayFrame(m_videoFrameQueue.last());
    }

    // 更新统计信息
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsed = currentTime - m_lastStatsUpdateTime;

    if (elapsed >= 1000) {
        int currentVideoFrames = m_receivedVideoFrameCount + m_receivedScreenFrameCount;
        int videoFps = (currentVideoFrames - m_lastVideoFrameCount) * 1000 / elapsed;

        emit videoStatsChanged(videoFps, currentVideoFrames);
        emit audioStatsChanged(m_receivedAudioFrameCount);
        emit screenStatsChanged(videoFps, m_receivedScreenFrameCount);

        QString status = QString("视频: %1 FPS, 帧数: %2, 音频包: %3, 消息: %4, 运行: %5")
                      .arg(videoFps)
                      .arg(currentVideoFrames)
                      .arg(m_receivedAudioFrameCount)
                      .arg(m_receivedTextMessageCount)
                      .arg(formatTimeDuration(currentTime - m_lastStatsUpdateTime));

        emit statusChanged(status);

        m_lastStatsUpdateTime = currentTime;
        m_lastVideoFrameCount = currentVideoFrames;
        m_lastScreenFrameCount = m_receivedScreenFrameCount;
    }
}

void AVReceiver::updateStatistics() {
    // 实时统计更新可以在处理每个数据包时调用
    emit videoStatsChanged(m_videoFps, m_receivedVideoFrameCount + m_receivedScreenFrameCount);
    emit audioStatsChanged(m_receivedAudioFrameCount);
    emit screenStatsChanged(m_videoFps, m_receivedScreenFrameCount);
}

QString AVReceiver::formatTimeDuration(qint64 milliseconds) const {
    qint64 seconds = milliseconds / 1000;
    qint64 minutes = seconds / 60;
    qint64 hours = minutes / 60;

    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes % 60, 2, 10, QLatin1Char('0'))
            .arg(seconds % 60, 2, 10, QLatin1Char('0'));
    } else {
        return QString("%1:%2")
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds % 60, 2, 10, QLatin1Char('0'));
    }
}

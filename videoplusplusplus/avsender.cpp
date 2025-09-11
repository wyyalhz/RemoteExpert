// ===============================================
// sender/av_sender.cpp
// 添加屏幕共享支持实现
// ===============================================

#include "avsender.h"
#include <QBuffer>
#include <QDateTime>
#include <QDebug>

AVSender::AVSender(QObject* parent)
    : QObject(parent)
    , m_camera(nullptr)
    , m_imageCapture(nullptr)
    , m_audioCapture(new AudioCapture(this))
    , m_screenCapture(new ScreenCapture(this))
    , m_isStreaming(false)
    , m_isScreenSharing(false)
    , m_roomId(0)
    , m_videoFps(15)
    , m_audioSampleRate(44100)
    , m_videoFrameCounter(0)
    , m_lastFpsTime(0)
    , m_actualVideoFps(0)
{
    m_videoTimer = new QTimer(this);
    m_videoTimer->setSingleShot(false);
    connect(m_videoTimer, &QTimer::timeout, this, &AVSender::captureVideoFrame);

    connect(m_audioCapture, &AudioCapture::audioPackaged, this, &AVSender::onAudioPackaged);
    connect(m_audioCapture, &AudioCapture::errorOccurred, this, &AVSender::onAudioError);

    connect(m_screenCapture, &ScreenCapture::screenFramePackaged, this, &AVSender::onScreenFramePackaged);
    connect(m_screenCapture, &ScreenCapture::errorOccurred, this, &AVSender::onScreenError);
    connect(m_screenCapture, &ScreenCapture::fpsChanged, this, &AVSender::screenFpsChanged);
}

AVSender::~AVSender() {
    stopStreaming();
}

void AVSender::setCamera(QCamera* camera) {
    if (m_camera != camera) {
        stopStreaming();
        m_camera = camera;
        if (m_camera) {
            setupImageCapture();
        }
    }
}

void AVSender::setVideoFps(int fps) {
    if (fps > 0 && fps <= 60) {
        m_videoFps = fps;
        if (m_isStreaming) {
            m_videoTimer->setInterval(1000 / m_videoFps);
        }
        emit videoFpsChanged(m_videoFps);
    }
}

void AVSender::setAudioSampleRate(int sampleRate) {
    if (sampleRate >= 8000 && sampleRate <= 48000) {
        m_audioSampleRate = sampleRate;
        if (m_isStreaming && m_audioCapture->isCapturing()) {
            m_audioCapture->stopCapture();
            m_audioCapture->startCapture(m_audioSampleRate);
        }
    }
}

void AVSender::setupImageCapture() {
    if (m_imageCapture) {
        delete m_imageCapture;
    }

    m_imageCapture = new QCameraImageCapture(m_camera, this);
    connect(m_imageCapture, &QCameraImageCapture::imageCaptured,
            this, &AVSender::onImageCaptured);
}

bool AVSender::startStreaming(uint32_t roomId, int videoFps, int audioSampleRate) {
    if (!m_camera || m_camera->state() != QCamera::ActiveState) {
        emit errorOccurred("摄像头未启动");
        return false;
    }

    if (m_isStreaming) {
        stopStreaming();
    }

    m_roomId = roomId;
    if (videoFps > 0) {
        m_videoFps = videoFps;
    }
    if (audioSampleRate > 0) {
        m_audioSampleRate = audioSampleRate;
    }

    m_isStreaming = true;
    m_videoFrameCounter = 0;
    m_lastFpsTime = QDateTime::currentMSecsSinceEpoch();
    m_actualVideoFps = 0;

    // 启动视频捕获
    m_videoTimer->setInterval(1000 / m_videoFps);
    m_videoTimer->start();

    // 启动音频捕获
    if (!m_audioCapture->startCapture(m_audioSampleRate)) {
        emit errorOccurred("音频捕获启动失败");
        m_videoTimer->stop();
        m_isStreaming = false;
        return false;
    }

    m_audioCapture->setRoomId(m_roomId);

    emit streamingStarted();
    qDebug() << "AV streaming started. Video:" << m_videoFps << "FPS, Audio:" << m_audioSampleRate << "Hz";
    return true;
}

void AVSender::stopStreaming() {
    if (m_isStreaming) {
        m_videoTimer->stop();
        m_audioCapture->stopCapture();
        m_isStreaming = false;
        emit streamingStopped();
        qDebug() << "AV streaming stopped";
    }
}

void AVSender::captureVideoFrame() {
    if (m_isStreaming && m_imageCapture) {
        m_imageCapture->capture();
    }
}

void AVSender::onImageCaptured(int id, const QImage& image) {
    Q_UNUSED(id)

    if (!m_isStreaming || image.isNull()) {
        return;
    }

    // 计算实际视频FPS
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - m_lastFpsTime >= 1000) {
        m_actualVideoFps = m_videoFrameCounter;
        m_videoFrameCounter = 0;
        m_lastFpsTime = currentTime;
        emit videoFpsChanged(m_actualVideoFps);
    }
    m_videoFrameCounter++;

    // 打包视频帧
    QByteArray frameData;
    QBuffer buffer(&frameData);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPEG", 80);

    QByteArray packet = ProtocolPackager::packVideoFrame(
        m_roomId,
        frameData,
        currentTime,
        image.width(),
        image.height(),
        "jpeg",
        m_actualVideoFps
    );



    emit dataPackaged(packet);

    // 录制本地视频帧
    if (m_recorder && m_recorder->isRecording()) {
        m_recorder->recordLocalVideoFrame(image, m_roomId, currentTime, false);
    }
}

void AVSender::onAudioPackaged(const QByteArray& packet) {
    if (m_isStreaming) {
        emit dataPackaged(packet);
    }
}

void AVSender::onAudioError(const QString& error) {
    emit errorOccurred("音频错误: " + error);
}


bool AVSender::startScreenSharing(uint32_t roomId, const ScreenCaptureConfig& config) {
    if (m_isScreenSharing) {
        stopScreenSharing();
    }

    m_roomId = roomId;
    m_screenCapture->setRoomId(roomId);

    if (m_screenCapture->startCapture(config)) {
        m_isScreenSharing = true;
        emit screenSharingStarted();
        qDebug() << "Screen sharing started. Mode:" << static_cast<int>(config.mode)
                 << ", FPS:" << config.fps;
        return true;
    }
    return false;
}

void AVSender::stopScreenSharing() {
    if (m_isScreenSharing) {
        m_screenCapture->stopCapture();
        m_isScreenSharing = false;
        emit screenSharingStopped();
        qDebug() << "Screen sharing stopped";
    }
}

void AVSender::updateScreenConfig(const ScreenCaptureConfig& config) {
    if (m_screenCapture) {
        m_screenCapture->updateConfig(config);
    }
}

void AVSender::onScreenFramePackaged(const QByteArray& packet) {
    if (m_isScreenSharing) {
        emit dataPackaged(packet);

        // 解析包以获取图像数据用于录制
        MsgType type;
        QJsonObject jsonData;
        QByteArray binaryData;

        if (ProtocolPackager::unpackMessage(packet, type, jsonData, binaryData)) {
            if (type == MsgType::SCREEN_FRAME && m_recorder && m_recorder->isRecording()) {
                QImage frame;
                if (frame.loadFromData(binaryData, "JPEG")) {
                    uint64_t timestamp = jsonData["ts"].toVariant().toULongLong();
                    uint32_t roomId = static_cast<uint32_t>(jsonData["roomId"].toInt());
                    m_recorder->recordLocalVideoFrame(frame, roomId, timestamp, true);
                }
            }
        }
    }
}


void AVSender::onScreenError(const QString& error) {
    emit errorOccurred("屏幕共享错误: " + error);
}


// ===============================================
// sender/av_sender.h
// 添加屏幕共享支持
// ===============================================

#pragma once

#include <QObject>
#include <QTimer>
#include <QCamera>
#include <QCameraImageCapture>
#include "protocol.h"
#include "audiocapture.h"
#include "screencapture.h"
#include "videorecorder.h"

class AVSender : public QObject {
    Q_OBJECT

public:
    explicit AVSender(QObject* parent = nullptr);
    ~AVSender();

    bool startStreaming(uint32_t roomId, int videoFps = 15, int audioSampleRate = 44100);
    bool startScreenSharing(uint32_t roomId, const ScreenCaptureConfig& config);
    void stopStreaming();
    void stopScreenSharing();
    bool isStreaming() const { return m_isStreaming; }
    bool isScreenSharing() const { return m_isScreenSharing; }

    void setCamera(QCamera* camera);
    void setVideoFps(int fps);
    void setAudioSampleRate(int sampleRate);
    void updateScreenConfig(const ScreenCaptureConfig& config);
    void setRecorder(VideoRecorder* recorder) { m_recorder = recorder; }
    VideoRecorder* recorder() const { return m_recorder; }

signals:
    void dataPackaged(const QByteArray& packet);
    void streamingStarted();
    void streamingStopped();
    void screenSharingStarted();
    void screenSharingStopped();
    void errorOccurred(const QString& error);
    void videoFpsChanged(int fps);
    void screenFpsChanged(int fps);
    void audioStatusChanged(const QString& status);
private slots:
    void onImageCaptured(int id, const QImage& image);
    void onAudioPackaged(const QByteArray& packet);
    void onScreenFramePackaged(const QByteArray& packet);
    void captureVideoFrame();
    void onAudioError(const QString& error);
    void onScreenError(const QString& error);

private:
    QCamera* m_camera;
    QCameraImageCapture* m_imageCapture;
    AudioCapture* m_audioCapture;
    ScreenCapture* m_screenCapture;
    VideoRecorder* m_recorder;
    QTimer* m_videoTimer;
    bool m_isStreaming;
    bool m_isScreenSharing;
    uint32_t m_roomId;
    int m_videoFps;
    int m_audioSampleRate;
    int m_videoFrameCounter;
    qint64 m_lastFpsTime;
    int m_actualVideoFps;

    void setupImageCapture();
};

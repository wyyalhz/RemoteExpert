// ===============================================
// screen/screen_capture.h
// 屏幕捕获器
// ===============================================

#pragma once

#include <QObject>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
#include <QDesktopWidget>
#include <QWindow>
#include <QPixmap>
#include "protocol/packager/protocol_packager.h"

class ScreenCapture : public QObject {
    Q_OBJECT

public:
    explicit ScreenCapture(QObject* parent = nullptr);
    ~ScreenCapture();

    bool startCapture(const ScreenCaptureConfig& config);
    void stopCapture();
    bool isCapturing() const { return m_isCapturing; }

    void setRoomId(uint32_t roomId) { m_roomId = roomId; }
    void updateConfig(const ScreenCaptureConfig& config);

signals:
    void screenFramePackaged(const QByteArray& packet);
    void captureStarted();
    void captureStopped();
    void errorOccurred(const QString& error);
    void fpsChanged(int fps);

private slots:
    void captureFrame();

private:
    QTimer* m_captureTimer;
    bool m_isCapturing;
    uint32_t m_roomId;
    ScreenCaptureConfig m_config;
    int m_frameCounter;
    qint64 m_lastFpsTime;
    int m_actualFps;
    QScreen* m_currentScreen;
    QWindow* m_activeWindow;

    QPixmap captureFullScreen();
    QPixmap captureActiveWindow();
    QPixmap captureSelectedArea();
    QRect findActiveWindowGeometry();
};

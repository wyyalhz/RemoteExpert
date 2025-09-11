// ===============================================
// screen/screen_capture.cpp
// 屏幕捕获器实现
// ===============================================

#include "screencapture.h"
#include <QBuffer>
#include <QDateTime>
#include <QDebug>
#include <QApplication>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

ScreenCapture::ScreenCapture(QObject* parent)
    : QObject(parent)
    , m_isCapturing(false)
    , m_roomId(0)
    , m_frameCounter(0)
    , m_lastFpsTime(0)
    , m_actualFps(0)
    , m_currentScreen(nullptr)
    , m_activeWindow(nullptr)
{
    m_captureTimer = new QTimer(this);
    m_captureTimer->setSingleShot(false);
    connect(m_captureTimer, &QTimer::timeout, this, &ScreenCapture::captureFrame);
}

ScreenCapture::~ScreenCapture() {
    stopCapture();
}

bool ScreenCapture::startCapture(const ScreenCaptureConfig& config) {
    if (m_isCapturing) {
        stopCapture();
    }

    m_config = config;
    m_isCapturing = true;
    m_frameCounter = 0;
    m_lastFpsTime = QDateTime::currentMSecsSinceEpoch();
    m_actualFps = 0;

    // 设置捕获定时器
    m_captureTimer->setInterval(1000 / m_config.fps);
    m_captureTimer->start();

    // 获取当前屏幕
    m_currentScreen = QGuiApplication::primaryScreen();

    emit captureStarted();
    qDebug() << "Screen capture started. Mode:" << static_cast<int>(m_config.mode)
             << ", FPS:" << m_config.fps;
    return true;
}

void ScreenCapture::stopCapture() {
    if (m_isCapturing) {
        m_captureTimer->stop();
        m_isCapturing = false;
        emit captureStopped();
        qDebug() << "Screen capture stopped";
    }
}

void ScreenCapture::updateConfig(const ScreenCaptureConfig& config) {
    m_config = config;
    if (m_isCapturing) {
        m_captureTimer->setInterval(1000 / m_config.fps);
    }
}

void ScreenCapture::captureFrame() {
    if (!m_isCapturing) return;

    QPixmap screenshot;
    try {
        switch (m_config.mode) {
        case ScreenCaptureMode::FULL_SCREEN:
            screenshot = captureFullScreen();
            break;
        case ScreenCaptureMode::ACTIVE_WINDOW:
            screenshot = captureActiveWindow();
            break;
        case ScreenCaptureMode::SELECTED_AREA:
            screenshot = captureSelectedArea();
            break;
        default:
            screenshot = captureFullScreen();
            break;
        }

        if (screenshot.isNull()) {
            emit errorOccurred("屏幕捕获失败");
            return;
        }

        // 计算实际FPS
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        if (currentTime - m_lastFpsTime >= 1000) {
            m_actualFps = m_frameCounter;
            m_frameCounter = 0;
            m_lastFpsTime = currentTime;
            emit fpsChanged(m_actualFps);
        }
        m_frameCounter++;

        // 转换为JPEG
        QByteArray frameData;
        QBuffer buffer(&frameData);
        buffer.open(QIODevice::WriteOnly);
        screenshot.save(&buffer, "JPEG", 80);

        // 打包屏幕帧
        QByteArray packet = ProtocolPackager::packScreenFrame(
            m_roomId,
            frameData,
            currentTime,
            screenshot.width(),
            screenshot.height(),
            "jpeg",
            m_actualFps,
            m_config.mode,
            m_config.captureArea
        );

        emit screenFramePackaged(packet);

    } catch (const std::exception& e) {
        emit errorOccurred(QString("屏幕捕获异常: %1").arg(e.what()));
    }
}

QPixmap ScreenCapture::captureFullScreen() {
    if (!m_currentScreen) {
        m_currentScreen = QGuiApplication::primaryScreen();
    }
    return m_currentScreen->grabWindow(0);
}

QPixmap ScreenCapture::captureActiveWindow() {
    QRect windowGeometry = findActiveWindowGeometry();
    if (windowGeometry.isValid()) {
        if (!m_currentScreen) {
            m_currentScreen = QGuiApplication::primaryScreen();
        }
        return m_currentScreen->grabWindow(0,
                                         windowGeometry.x(),
                                         windowGeometry.y(),
                                         windowGeometry.width(),
                                         windowGeometry.height());
    }
    return captureFullScreen();
}

QPixmap ScreenCapture::captureSelectedArea() {
    if (m_config.captureArea.isValid()) {
        if (!m_currentScreen) {
            m_currentScreen = QGuiApplication::primaryScreen();
        }
        return m_currentScreen->grabWindow(0,
                                         m_config.captureArea.x(),
                                         m_config.captureArea.y(),
                                         m_config.captureArea.width(),
                                         m_config.captureArea.height());
    }
    return captureFullScreen();
}

QRect ScreenCapture::findActiveWindowGeometry() {
#ifdef Q_OS_WIN
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        RECT rect;
        if (GetWindowRect(hwnd, &rect)) {
            return QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
        }
    }
#endif

    // 跨平台备用方案
    QWidget* activeWindow = QApplication::activeWindow();
    if (activeWindow && activeWindow->isVisible()) {
        return activeWindow->geometry();
    }

    return QRect();
}

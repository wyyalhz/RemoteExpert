// ===============================================
// mainwindow.h
// 音视频聊天系统 - 主窗口头文件
// ===============================================

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QListView>
#include <QScrollArea>
#include <QFrame>
#include <QSplitter>
#include "avsender.h"
#include "avreceiver.h"
#include "chatmodel.h"
#include "videorecorder.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 摄像头控制
    void onStartCamera();
    void onStopCamera();
    void onCameraChanged(int index);
    void onRoomIdChanged(int roomId);

    // 视频参数控制
    void onVideoFpsSliderChanged(int fps);
    void onAudioSampleRateChanged(const QString& sampleRate);

    // 流控制
    void onStartStreaming();
    void onStopStreaming();

    // 屏幕共享控制
    void onStartScreenSharing();
    void onStopScreenSharing();
    void onScreenModeChanged(int index);
    void onScreenFpsChanged(int fps);

    // 接收控制
    void onStartReceiver();
    void onStopReceiver();
    void onReceiverRoomIdChanged(int roomId);

    // 聊天控制
    void onSendMessage();
    void onMessageTextChanged(const QString& text);
    void onUserNameChanged(const QString& name);

    // 音频视频控制
    void onAudioEnabledToggled(bool enabled);
    void onVideoEnabledToggled(bool enabled);

    // 发送器信号处理
    void onDataPackaged(const QByteArray& packet);
    void onStreamingStarted();
    void onStreamingStopped();
    void onScreenSharingStarted();
    void onScreenSharingStopped();
    void onSenderErrorOccurred(const QString& error);
    void onVideoFpsChanged(int fps);
    void onScreenFpsReported(int fps);
    void onAudioStatusChanged(const QString& status);

    // 接收器信号处理
    void onReceiverStatusChanged(const QString &status);
    void onReceiverErrorOccurred(const QString &error);
    void onTextMessageReceived(const TextMessage& message);
    void onSystemMessageReceived(const QString& message);
    void onVideoStatsChanged(int fps, int totalFrames);
    void onAudioStatsChanged(int totalPackets);
    void onScreenStatsChanged(int fps, int totalFrames);

    void onStartRecording();
    void onStopRecording();
    void onAutoRecordToggled(bool checked);
    void onRecordingStarted(const QString& filePath);
    void onRecordingStopped(const QString& filePath, qint64 duration, qint64 size);
    void onRecordingError(const QString& error);
    void onRecordingStatsUpdated(qint64 duration, qint64 size, int videoFrames, int audioFrames);
    void onStreamStatsUpdated(const QMap<QString, int>& stats);
private:
    Ui::MainWindow *ui;

    // 摄像头相关
    QCamera *m_camera;
    QCameraViewfinder *m_viewfinder;
    QCameraImageCapture *m_imageCapture;
    QList<QCameraInfo> m_availableCameras;

    // 音视频处理器
    AVSender *m_avSender;
    AVReceiver *m_avReceiver;

    // 聊天相关
    ChatModel *m_chatModel;

    // 添加录制器
        VideoRecorder* m_videoRecorder;

        // 添加录制控件
        QGroupBox* m_recordGroup;
        QPushButton* m_startRecordButton;
        QPushButton* m_stopRecordButton;
        QLabel* m_recordStatusLabel;
        QLabel* m_recordStatsLabel;
        QCheckBox* m_autoRecordCheck;

        // 添加录制状态
        bool m_isRecording;
        qint64 m_recordStartTime;

    // 界面控件
    // 左侧面板
    QGroupBox *m_senderGroup;
    QLabel *m_senderVideoLabel;
    QLabel *m_senderStatusLabel;
    QLabel *m_videoFpsLabel;
    QLabel *m_audioStatusLabel;
    QLabel *m_screenFpsLabel;
    QLabel *m_statsLabel;

    // 控制控件
    QComboBox *m_cameraComboBox;
    QComboBox *m_audioSampleRateCombo;
    QComboBox *m_screenModeCombo;
    QSpinBox *m_roomIdSpinBox;
    QSpinBox *m_receiverRoomIdSpinBox;
    QSpinBox *m_screenFpsSpin;
    QSpinBox *m_userIdSpin;
    QSlider *m_videoFpsSlider;
    QCheckBox *m_includeCursorCheck;
    QCheckBox *m_audioEnabledCheck;
    QCheckBox *m_videoEnabledCheck;

    // 按钮
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_startStreamButton;
    QPushButton *m_stopStreamButton;
    QPushButton *m_startScreenButton;
    QPushButton *m_stopScreenButton;
    QPushButton *m_startReceiverButton;
    QPushButton *m_stopReceiverButton;

    // 右侧面板
    QGroupBox *m_receiverGroup;
    QLabel *m_receiverVideoLabel;
    QLabel *m_receiverStatusLabel;

    // 聊天区域
    QGroupBox *m_chatGroup;
    QTextEdit *m_chatDisplay;
    QLineEdit *m_messageEdit;
    QLineEdit *m_userNameEdit;
    QPushButton *m_sendMessageButton;

    // 统计信息
    QGroupBox *m_statsGroup;
    QLabel *m_videoStatsLabel;
    QLabel *m_audioStatsLabel;
    QLabel *m_screenStatsLabel;
    QLabel *m_messageStatsLabel;

    // 数据
    uint32_t m_roomId;
    uint32_t m_userId;
    QString m_userName;
    int m_receivedMessages;
    int m_sentMessages;

    void setupUI();
    void setupCamera();
    void setupConnections();
    void updateStatus(const QString &message);
    void addChatMessage(const TextMessage& message, bool isOwnMessage = false);
    void addSystemMessage(const QString& message);
    void scrollChatToBottom();
    void updateStatsDisplay();
    QString getMessageStyle(const TextMessage& message, bool isOwnMessage) const;
    QMap<QString, int> m_streamStats;
    void updateRecordStatsDisplay();
};

#endif // MAINWINDOW_H

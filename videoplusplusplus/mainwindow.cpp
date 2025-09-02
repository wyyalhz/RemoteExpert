// ===============================================
// mainwindow.cpp
// 音视频聊天系统 - 主窗口实现（第一部分）
// ===============================================

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QScrollBar>
#include <QFrame>
#include <QSplitter>
#include <QScrollArea>
#include <QFont>
#include <QPalette>
#include <QRandomGenerator>
#include <QCameraInfo>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_camera(nullptr)
    , m_viewfinder(nullptr)
    , m_imageCapture(nullptr)
    , m_avSender(nullptr)
    , m_avReceiver(nullptr)
    , m_chatModel(new ChatModel(this))
    , m_roomId(1001)
    , m_userId(1000 + QRandomGenerator::global()->bounded(9000))
    , m_userName("用户" + QString::number(m_userId))
    , m_receivedMessages(0)
    , m_sentMessages(0)
    , m_videoRecorder(new VideoRecorder(this))
    , m_isRecording(false)
    , m_recordStartTime(0)
{
    ui->setupUi(this);
    setWindowTitle("音视频聊天系统");
    resize(1600, 900);

    setupUI();
    setupCamera();
    setupConnections();

    // 创建音视频发送器
    m_avSender = new AVSender(this);
    connect(m_avSender, &AVSender::dataPackaged, this, &MainWindow::onDataPackaged);
    connect(m_avSender, &AVSender::streamingStarted, this, &MainWindow::onStreamingStarted);
    connect(m_avSender, &AVSender::streamingStopped, this, &MainWindow::onStreamingStopped);
    connect(m_avSender, &AVSender::screenSharingStarted, this, &MainWindow::onScreenSharingStarted);
    connect(m_avSender, &AVSender::screenSharingStopped, this, &MainWindow::onScreenSharingStopped);
    connect(m_avSender, &AVSender::errorOccurred, this, &MainWindow::onSenderErrorOccurred);
    connect(m_avSender, &AVSender::videoFpsChanged, this, &MainWindow::onVideoFpsChanged);
    connect(m_avSender, &AVSender::screenFpsChanged, this, &MainWindow::onScreenFpsReported);
    connect(m_avSender, &AVSender::audioStatusChanged, this, &MainWindow::onAudioStatusChanged);

    // 创建音视频接收器
    m_avReceiver = new AVReceiver(m_receiverVideoLabel, this);
    connect(m_avReceiver, &AVReceiver::statusChanged, this, &MainWindow::onReceiverStatusChanged);
    connect(m_avReceiver, &AVReceiver::errorOccurred, this, &MainWindow::onReceiverErrorOccurred);
    connect(m_avReceiver, &AVReceiver::textMessageReceived, this, &MainWindow::onTextMessageReceived);
    connect(m_avReceiver, &AVReceiver::systemMessageReceived, this, &MainWindow::onSystemMessageReceived);
    connect(m_avReceiver, &AVReceiver::videoStatsChanged, this, &MainWindow::onVideoStatsChanged);
    connect(m_avReceiver, &AVReceiver::audioStatsChanged, this, &MainWindow::onAudioStatsChanged);
    connect(m_avReceiver, &AVReceiver::screenStatsChanged, this, &MainWindow::onScreenStatsChanged);

    // 连接录制器信号
    connect(m_videoRecorder, &VideoRecorder::recordingStarted, this, &MainWindow::onRecordingStarted);
    connect(m_videoRecorder, &VideoRecorder::recordingStopped, this, &MainWindow::onRecordingStopped);
    connect(m_videoRecorder, &VideoRecorder::recordingError, this, &MainWindow::onRecordingError);
    connect(m_videoRecorder, &VideoRecorder::recordingStatsUpdated, this, &MainWindow::onRecordingStatsUpdated);
    connect(m_videoRecorder, &VideoRecorder::streamStatsUpdated,
               this, &MainWindow::onStreamStatsUpdated);
    // 设置接收器的录制器
    m_avReceiver->setRecorder(m_videoRecorder);
    m_avSender->setRecorder(m_videoRecorder);

    // 添加欢迎消息
    addSystemMessage("🚀 欢迎使用音视频聊天系统！");
    addSystemMessage(QString("👤 您的用户ID: <b>%1</b>，名称: <b>%2</b>").arg(m_userId).arg(m_userName));
    addSystemMessage("💡 提示：请先启动摄像头，然后开始推流或屏幕共享");
}

MainWindow::~MainWindow()
{
    onStopCamera();
    onStopScreenSharing();
    onStopReceiver();
    delete ui;
}

void MainWindow::setupUI()
{
    // 设置主窗口样式
    setStyleSheet(R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2c3e50, stop:1 #34495e);
        }
        QGroupBox {
            font-weight: bold;
            border: 2px solid #3498db;
            border-radius: 8px;
            margin-top: 1ex;
            padding-top: 10px;
            background: rgba(255, 255, 255, 15);
            color: #ecf0f1;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 0 5px;
            color: #3498db;
        }
        QLabel {
            color: #ecf0f1;
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9);
            border: none;
            border-radius: 6px;
            color: white;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5dade2, stop:1 #3498db);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2980b9, stop:1 #2471a3);
        }
        QPushButton:disabled {
            background: #95a5a6;
            color: #7f8c8d;
        }
        QComboBox, QSpinBox, QLineEdit {
            background: white;
            border: 2px solid #bdc3c7;
            border-radius: 4px;
            padding: 5px;
            selection-background-color: #3498db;
        }
        QComboBox:focus, QSpinBox:focus, QLineEdit:focus {
            border-color: #3498db;
        }
        QSlider::groove:horizontal {
            border: 1px solid #bbb;
            background: white;
            height: 6px;
            border-radius: 3px;
        }
        QSlider::sub-page:horizontal {
            background: #3498db;
            border: 1px solid #777;
            height: 6px;
            border-radius: 3px;
        }
        QSlider::add-page:horizontal {
            background: #fff;
            border: 1px solid #777;
            height: 6px;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #eee, stop:1 #ccc);
            border: 1px solid #777;
            width: 16px;
            margin-top: -5px;
            margin-bottom: -5px;
            border-radius: 8px;
        }
        QCheckBox {
            color: #ecf0f1;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 2px solid #bdc3c7;
            border-radius: 3px;
            background: white;
        }
        QCheckBox::indicator:checked {
            background: #3498db;
            border-color: #3498db;
        }
        QTextEdit {
            background: white;
            border: 2px solid #bdc3c7;
            border-radius: 6px;
            font-family: 'Microsoft YaHei', 'Segoe UI';
            font-size: 11px;
        }
    )");

    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 左侧面板 - 发送端和聊天
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(10);

    // 发送端组
    m_senderGroup = new QGroupBox("🎥 视频发送端", this);
    QVBoxLayout *senderLayout = new QVBoxLayout(m_senderGroup);

    // 视频显示标签
    m_senderVideoLabel = new QLabel(this);
    m_senderVideoLabel->setMinimumSize(400, 300);
    m_senderVideoLabel->setAlignment(Qt::AlignCenter);
    m_senderVideoLabel->setStyleSheet(R"(
        border: 2px solid #3498db;
        border-radius: 8px;
        background: #2c3e50;
        color: #ecf0f1;
        font-weight: bold;
    )");
    m_senderVideoLabel->setText("📷 摄像头未启动");

    // 用户信息
    QHBoxLayout *userInfoLayout = new QHBoxLayout();
    userInfoLayout->addWidget(new QLabel("🆔 用户ID:"));
    m_userIdSpin = new QSpinBox(this);
    m_userIdSpin->setRange(1000, 9999);
    m_userIdSpin->setValue(m_userId);
    m_userIdSpin->setEnabled(false);
    userInfoLayout->addWidget(m_userIdSpin);

    userInfoLayout->addWidget(new QLabel("👤 名称:"));
    m_userNameEdit = new QLineEdit(m_userName, this);
    m_userNameEdit->setMaxLength(20);
    m_userNameEdit->setPlaceholderText("输入您的名称");
    userInfoLayout->addWidget(m_userNameEdit);

    // 发送端控制网格
    QGridLayout *senderControlLayout = new QGridLayout();
    senderControlLayout->setSpacing(8);

    // 第一行
    senderControlLayout->addWidget(new QLabel("🏠 房间ID:"), 0, 0);
    m_roomIdSpinBox = new QSpinBox(this);
    m_roomIdSpinBox->setRange(1, 9999);
    m_roomIdSpinBox->setValue(m_roomId);
    m_roomIdSpinBox->setSuffix(" 号房间");
    senderControlLayout->addWidget(m_roomIdSpinBox, 0, 1);

    senderControlLayout->addWidget(new QLabel("📊 视频帧率:"), 0, 2);
    m_videoFpsSlider = new QSlider(Qt::Horizontal, this);
    m_videoFpsSlider->setRange(1, 30);
    m_videoFpsSlider->setValue(15);
    m_videoFpsSlider->setTickPosition(QSlider::TicksBelow);
    m_videoFpsSlider->setTickInterval(5);
    senderControlLayout->addWidget(m_videoFpsSlider, 0, 3);

    m_videoFpsLabel = new QLabel("15 FPS", this);
    m_videoFpsLabel->setAlignment(Qt::AlignCenter);
    m_videoFpsLabel->setStyleSheet("color: #3498db; font-weight: bold;");
    senderControlLayout->addWidget(m_videoFpsLabel, 0, 4);

    // 第二行
    senderControlLayout->addWidget(new QLabel("🎵 音频采样率:"), 1, 0);
    m_audioSampleRateCombo = new QComboBox(this);
    m_audioSampleRateCombo->addItems({"8000", "16000", "22050", "44100", "48000"});
    m_audioSampleRateCombo->setCurrentText("44100");
    m_audioSampleRateCombo->setToolTip("音频采样率（Hz）");
    senderControlLayout->addWidget(m_audioSampleRateCombo, 1, 1);

    m_audioStatusLabel = new QLabel("🔈 音频就绪", this);
    m_audioStatusLabel->setAlignment(Qt::AlignCenter);
    senderControlLayout->addWidget(m_audioStatusLabel, 1, 2, 1, 3);

    // 第三行
    senderControlLayout->addWidget(new QLabel("📷 摄像头:"), 2, 0);
    m_cameraComboBox = new QComboBox(this);
    senderControlLayout->addWidget(m_cameraComboBox, 2, 1, 1, 4);

    // 按钮行
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_startButton = new QPushButton("🚀 启动摄像头", this);
    m_stopButton = new QPushButton("⏹️ 停止摄像头", this);
    m_startStreamButton = new QPushButton("📡 开始推流", this);
    m_stopStreamButton = new QPushButton("⏹️ 停止推流", this);

    m_stopButton->setEnabled(false);
    m_startStreamButton->setEnabled(false);
    m_stopStreamButton->setEnabled(false);

    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addWidget(m_startStreamButton);
    buttonLayout->addWidget(m_stopStreamButton);

    // 状态标签
    m_senderStatusLabel = new QLabel("✅ 发送端就绪", this);
    m_senderStatusLabel->setStyleSheet(R"(
        background: rgba(52, 152, 219, 0.2);
        border: 1px solid #3498db;
        border-radius: 4px;
        padding: 8px;
        color: #ecf0f1;
        font-weight: bold;
    )");
    m_senderStatusLabel->setAlignment(Qt::AlignCenter);

    // 组装发送端布局
    senderLayout->addWidget(m_senderVideoLabel);
    senderLayout->addLayout(userInfoLayout);
    senderLayout->addLayout(senderControlLayout);
    senderLayout->addLayout(buttonLayout);
    senderLayout->addWidget(m_senderStatusLabel);

    // 屏幕共享组
    QGroupBox *screenGroup = new QGroupBox("🖥️ 屏幕共享", this);
    QVBoxLayout *screenLayout = new QVBoxLayout(screenGroup);

    QGridLayout *screenControlLayout = new QGridLayout();
    screenControlLayout->addWidget(new QLabel("📋 共享模式:"), 0, 0);
    m_screenModeCombo = new QComboBox(this);
    m_screenModeCombo->addItems({"🖥️ 全屏共享", "📁 活动窗口", "🔲 选择区域"});
    screenControlLayout->addWidget(m_screenModeCombo, 0, 1);

    screenControlLayout->addWidget(new QLabel("📊 屏幕帧率:"), 1, 0);
    m_screenFpsSpin = new QSpinBox(this);
    m_screenFpsSpin->setRange(1, 30);
    m_screenFpsSpin->setValue(10);
    screenControlLayout->addWidget(m_screenFpsSpin, 1, 1);

    m_screenFpsLabel = new QLabel("10 FPS", this);
    m_screenFpsLabel->setStyleSheet("color: #e74c3c; font-weight: bold;");
    screenControlLayout->addWidget(m_screenFpsLabel, 1, 2);

    m_includeCursorCheck = new QCheckBox("🖱️ 包含鼠标指针", this);
    m_includeCursorCheck->setChecked(true);
    screenControlLayout->addWidget(m_includeCursorCheck, 2, 0, 1, 2);

    QHBoxLayout *screenButtonLayout = new QHBoxLayout();
    m_startScreenButton = new QPushButton("📺 开始共享", this);
    m_stopScreenButton = new QPushButton("⏹️ 停止共享", this);
    m_stopScreenButton->setEnabled(false);

    screenButtonLayout->addWidget(m_startScreenButton);
    screenButtonLayout->addWidget(m_stopScreenButton);

    screenLayout->addLayout(screenControlLayout);
    screenLayout->addLayout(screenButtonLayout);

    // 控制选项组
    QGroupBox *controlGroup = new QGroupBox("⚙️ 控制选项", this);
    QHBoxLayout *controlLayout = new QHBoxLayout(controlGroup);

    m_audioEnabledCheck = new QCheckBox("🔊 启用音频", this);
    m_audioEnabledCheck->setChecked(true);
    m_videoEnabledCheck = new QCheckBox("📹 启用视频", this);
    m_videoEnabledCheck->setChecked(true);

    controlLayout->addWidget(m_audioEnabledCheck);
    controlLayout->addWidget(m_videoEnabledCheck);

    // 组装左侧布局
    leftLayout->addWidget(m_senderGroup);
    leftLayout->addWidget(screenGroup);
    leftLayout->addWidget(controlGroup);
    leftLayout->setStretch(0, 3);
    leftLayout->setStretch(1, 1);
    leftLayout->setStretch(2, 1);

    // 中间面板 - 聊天区域
    QVBoxLayout *centerLayout = new QVBoxLayout();
    centerLayout->setSpacing(10);

    // 聊天组
    m_chatGroup = new QGroupBox("💬 文字聊天", this);
    QVBoxLayout *chatLayout = new QVBoxLayout(m_chatGroup);

    // 聊天显示区域
    m_chatDisplay = new QTextEdit(this);
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setStyleSheet(R"(
        QTextEdit {
            background: white;
            border: 2px solid #bdc3c7;
            border-radius: 8px;
            font-family: 'Microsoft YaHei', 'Segoe UI';
            font-size: 11px;
            padding: 8px;
        }
    )");
    m_chatDisplay->setHtml(R"(
        <div style='text-align: center; color: #7f8c8d; font-style: italic;'>
            💬 欢迎使用聊天功能！消息将在这里显示...
        </div>
    )");

    // 消息输入区域
    QHBoxLayout *messageInputLayout = new QHBoxLayout();
    m_messageEdit = new QLineEdit(this);
    m_messageEdit->setPlaceholderText("💭 输入消息内容...");
    m_messageEdit->setStyleSheet(R"(
        QLineEdit {
            background: white;
            border: 2px solid #bdc3c7;
            border-radius: 6px;
            padding: 8px;
            font-size: 11px;
        }
        QLineEdit:focus {
            border-color: #3498db;
        }
    )");

    m_sendMessageButton = new QPushButton("📤 发送", this);
    m_sendMessageButton->setEnabled(false);
    m_sendMessageButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #27ae60, stop:1 #229954);
            min-width: 80px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #2ecc71, stop:1 #27ae60);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #229954, stop:1 #1e8449);
        }
    )");

    messageInputLayout->addWidget(m_messageEdit);
    messageInputLayout->addWidget(m_sendMessageButton);

    chatLayout->addWidget(m_chatDisplay);
    chatLayout->addLayout(messageInputLayout);

    // 统计信息组
    m_statsGroup = new QGroupBox("📊 统计信息", this);
    QGridLayout *statsLayout = new QGridLayout(m_statsGroup);

    m_videoStatsLabel = new QLabel("📹 视频: 0 FPS, 总帧数: 0", this);
    m_audioStatsLabel = new QLabel("🎵 音频: 总包数: 0", this);
    m_screenStatsLabel = new QLabel("🖥️ 屏幕: 0 FPS, 总帧数: 0", this);
    m_messageStatsLabel = new QLabel("💬 消息: 收到: 0, 发送: 0", this);

    QFont statsFont = m_videoStatsLabel->font();
    statsFont.setPointSize(9);
    m_videoStatsLabel->setFont(statsFont);
    m_audioStatsLabel->setFont(statsFont);
    m_screenStatsLabel->setFont(statsFont);
    m_messageStatsLabel->setFont(statsFont);

    statsLayout->addWidget(m_videoStatsLabel, 0, 0);
    statsLayout->addWidget(m_audioStatsLabel, 1, 0);
    statsLayout->addWidget(m_screenStatsLabel, 0, 1);
    statsLayout->addWidget(m_messageStatsLabel, 1, 1);

    centerLayout->addWidget(m_chatGroup);
    centerLayout->addWidget(m_statsGroup);
    centerLayout->setStretch(0, 3);
    centerLayout->setStretch(1, 1);

    // 右侧面板 - 接收端
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(10);

    m_receiverGroup = new QGroupBox("📺 视频接收端", this);
    QVBoxLayout *receiverLayout = new QVBoxLayout(m_receiverGroup);

    m_receiverVideoLabel = new QLabel(this);
    m_receiverVideoLabel->setMinimumSize(400, 300);
    m_receiverVideoLabel->setAlignment(Qt::AlignCenter);
    m_receiverVideoLabel->setStyleSheet(R"(
        border: 2px solid #e74c3c;
        border-radius: 8px;
        background: #2c3e50;
        color: #ecf0f1;
        font-weight: bold;
    )");
    m_receiverVideoLabel->setText("📡 接收端未启动");

    QHBoxLayout *receiverControlLayout = new QHBoxLayout();
    receiverControlLayout->addWidget(new QLabel("🏠 房间ID:"));
    m_receiverRoomIdSpinBox = new QSpinBox(this);
    m_receiverRoomIdSpinBox->setRange(0, 9999);
    m_receiverRoomIdSpinBox->setValue(0);
    m_receiverRoomIdSpinBox->setSpecialValueText("所有房间");
    receiverControlLayout->addWidget(m_receiverRoomIdSpinBox);

    m_startReceiverButton = new QPushButton("📡 启动接收", this);
    m_stopReceiverButton = new QPushButton("⏹️ 停止接收", this);
    m_stopReceiverButton->setEnabled(false);

    receiverControlLayout->addWidget(m_startReceiverButton);
    receiverControlLayout->addWidget(m_stopReceiverButton);

    m_receiverStatusLabel = new QLabel("✅ 接收端就绪", this);
    m_receiverStatusLabel->setStyleSheet(R"(
        background: rgba(231, 76, 60, 0.2);
        border: 1px solid #e74c3c;
        border-radius: 4px;
        padding: 8px;
        color: #ecf0f1;
        font-weight: bold;
    )");
    m_receiverStatusLabel->setAlignment(Qt::AlignCenter);

    receiverLayout->addWidget(m_receiverVideoLabel);
    receiverLayout->addLayout(receiverControlLayout);
    receiverLayout->addWidget(m_receiverStatusLabel);

    rightLayout->addWidget(m_receiverGroup);

    // 录制组
        m_recordGroup = new QGroupBox("🔴 会议录制", this);
        QVBoxLayout *recordLayout = new QVBoxLayout(m_recordGroup);

        m_autoRecordCheck = new QCheckBox("⚡ 自动录制（开始推流时自动录制）", this);
        m_autoRecordCheck->setChecked(true);

        QHBoxLayout *recordButtonLayout = new QHBoxLayout();
        m_startRecordButton = new QPushButton("🔴 开始录制", this);
        m_stopRecordButton = new QPushButton("⏹️ 停止录制", this);
        m_stopRecordButton->setEnabled(false);

        recordButtonLayout->addWidget(m_startRecordButton);
        recordButtonLayout->addWidget(m_stopRecordButton);

        m_recordStatusLabel = new QLabel("⏸️ 录制未开始", this);
        m_recordStatusLabel->setStyleSheet(R"(
            background: rgba(231, 76, 60, 0.2);
            border: 1px solid #e74c3c;
            border-radius: 4px;
            padding: 6px;
            color: #ecf0f1;
            font-weight: bold;
        )");

        m_recordStatsLabel = new QLabel("时长: 00:00, 大小: 0 MB, 帧数: 0", this);
        QFont statsFont1 = m_recordStatsLabel->font();
        statsFont.setPointSize(9);
        m_recordStatsLabel->setFont(statsFont);

        recordLayout->addWidget(m_autoRecordCheck);
        recordLayout->addLayout(recordButtonLayout);
        recordLayout->addWidget(m_recordStatusLabel);
        recordLayout->addWidget(m_recordStatsLabel);

        // 将录制组添加到左侧布局
        leftLayout->addWidget(m_recordGroup);

        // 连接录制信号
        connect(m_startRecordButton, &QPushButton::clicked, this, &MainWindow::onStartRecording);
        connect(m_stopRecordButton, &QPushButton::clicked, this, &MainWindow::onStopRecording);
        connect(m_autoRecordCheck, &QCheckBox::toggled, this, &MainWindow::onAutoRecordToggled);

    // 主布局
    mainLayout->addLayout(leftLayout, 2);
    mainLayout->addLayout(centerLayout, 3);
    mainLayout->addLayout(rightLayout, 2);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

// 添加录制控制函数
void MainWindow::onStartRecording() {
    QString savePath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    if (savePath.isEmpty()) {
        savePath = QDir::currentPath();
    }
    m_streamStats.clear();

    if (m_videoRecorder->startRecording(savePath)) {
        m_isRecording = true;
        m_recordStartTime = QDateTime::currentMSecsSinceEpoch();
        m_startRecordButton->setEnabled(false);
        m_stopRecordButton->setEnabled(true);
        m_autoRecordCheck->setEnabled(false);
    }
}

void MainWindow::onStopRecording() {
    m_videoRecorder->stopRecording();
    m_isRecording = false;
    m_startRecordButton->setEnabled(true);
    m_stopRecordButton->setEnabled(false);
    m_autoRecordCheck->setEnabled(true);
}

void MainWindow::onAutoRecordToggled(bool checked) {
    m_senderStatusLabel->setText(checked ? "✅ 自动录制已启用" : "⏸️ 自动录制已禁用");
}

void MainWindow::onRecordingStarted(const QString& filePath) {
    m_recordStatusLabel->setText("🔴 正在录制: " + QFileInfo(filePath).fileName());
    addSystemMessage("🔴 开始会议录制: " + QFileInfo(filePath).fileName());
}

void MainWindow::onRecordingStopped(const QString& filePath, qint64 duration, qint64 size) {
    QString durationStr = QString("%1:%2")
        .arg(duration / 60000, 2, 10, QLatin1Char('0'))
        .arg((duration % 60000) / 1000, 2, 10, QLatin1Char('0'));

    QString sizeStr = QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 2);

    m_recordStatusLabel->setText("✅ 录制已保存");
    m_recordStatsLabel->setText(QString("时长: %1, 大小: %2").arg(durationStr).arg(sizeStr));

    addSystemMessage(QString("✅ 会议录制已保存 (时长: %1, 大小: %2)").arg(durationStr).arg(sizeStr));
    addSystemMessage("💾 文件位置: " + filePath);
}

void MainWindow::onRecordingError(const QString& error) {
    m_recordStatusLabel->setText("❌ 录制错误: " + error);
    addSystemMessage("❌ 录制错误: " + error);
    QMessageBox::warning(this, "录制错误", error);
}

void MainWindow::onRecordingStatsUpdated(qint64 duration, qint64 size, int videoFrames, int audioFrames) {
    QString durationStr = QString("%1:%2")
        .arg(duration / 60000, 2, 10, QLatin1Char('0'))
        .arg((duration % 60000) / 1000, 2, 10, QLatin1Char('0'));

    QString sizeStr = QString("%1 MB").arg(size / (1024.0 * 1024.0), 0, 'f', 1);

    QString statsText = QString("时长: %1 | 大小: %2 | 音频: %3帧")
                      .arg(durationStr).arg(sizeStr).arg(audioFrames);

    m_recordStatsLabel->setText(statsText);
}

void MainWindow::setupCamera()
{
    qDebug() << "Setting up camera...";

    m_availableCameras = QCameraInfo::availableCameras();
    m_cameraComboBox->clear();

    qDebug() << "Available cameras count:" << m_availableCameras.size();

    for (const QCameraInfo &cameraInfo : m_availableCameras) {
        qDebug() << "Camera:" << cameraInfo.description()
                 << "Device:" << cameraInfo.deviceName()
                 << "Position:" << cameraInfo.position();
        m_cameraComboBox->addItem(cameraInfo.description());
    }

    if (m_availableCameras.isEmpty()) {
        m_cameraComboBox->addItem("❌ 无可用摄像头");
        m_startButton->setEnabled(false);
        m_senderStatusLabel->setText("❌ 未检测到摄像头");
        addSystemMessage("⚠️ 警告：未检测到摄像头设备");
    } else {
        m_senderStatusLabel->setText(QString("✅ 检测到 %1 个摄像头").arg(m_availableCameras.size()));
        addSystemMessage(QString("📷 检测到 %1 个摄像头设备").arg(m_availableCameras.size()));
    }
}

void MainWindow::setupConnections()
{
    // 摄像头控制
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartCamera);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopCamera);
    connect(m_cameraComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCameraChanged);
    connect(m_roomIdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onRoomIdChanged);

    // 参数控制
    connect(m_videoFpsSlider, &QSlider::valueChanged, this, &MainWindow::onVideoFpsSliderChanged);
    connect(m_audioSampleRateCombo, &QComboBox::currentTextChanged, this, &MainWindow::onAudioSampleRateChanged);
    connect(m_screenModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onScreenModeChanged);
    connect(m_screenFpsSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onScreenFpsChanged);

    // 流控制
    connect(m_startStreamButton, &QPushButton::clicked, this, &MainWindow::onStartStreaming);
    connect(m_stopStreamButton, &QPushButton::clicked, this, &MainWindow::onStopStreaming);
    connect(m_startScreenButton, &QPushButton::clicked, this, &MainWindow::onStartScreenSharing);
    connect(m_stopScreenButton, &QPushButton::clicked, this, &MainWindow::onStopScreenSharing);

    // 接收控制
    connect(m_startReceiverButton, &QPushButton::clicked, this, &MainWindow::onStartReceiver);
    connect(m_stopReceiverButton, &QPushButton::clicked, this, &MainWindow::onStopReceiver);
    connect(m_receiverRoomIdSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onReceiverRoomIdChanged);

    // 聊天控制
    connect(m_messageEdit, &QLineEdit::returnPressed, this, &MainWindow::onSendMessage);
    connect(m_sendMessageButton, &QPushButton::clicked, this, &MainWindow::onSendMessage);
    connect(m_messageEdit, &QLineEdit::textChanged, this, &MainWindow::onMessageTextChanged);
    connect(m_userNameEdit, &QLineEdit::textChanged, this, &MainWindow::onUserNameChanged);

    // 音频视频控制
    connect(m_audioEnabledCheck, &QCheckBox::toggled, this, &MainWindow::onAudioEnabledToggled);
    connect(m_videoEnabledCheck, &QCheckBox::toggled, this, &MainWindow::onVideoEnabledToggled);
}

void MainWindow::onStartCamera()
{
    qDebug() << "onStartCamera called";

    if (m_availableCameras.isEmpty()) {
        QMessageBox::warning(this, "错误", "没有可用的摄像头");
        m_senderStatusLabel->setText("❌ 错误: 没有检测到摄像头");
        return;
    }

    int selectedIndex = m_cameraComboBox->currentIndex();
    if (selectedIndex < 0 || selectedIndex >= m_availableCameras.size()) {
        m_senderStatusLabel->setText("❌ 错误: 无效的摄像头选择");
        return;
    }

    qDebug() << "Selected camera:" << m_availableCameras[selectedIndex].description();

    // 清理之前的摄像头
    onStopCamera();
    onStopScreenSharing();

    try {
        // 创建新的摄像头实例
        m_camera = new QCamera(m_availableCameras[selectedIndex], this);
        m_viewfinder = new QCameraViewfinder(this);
        m_imageCapture = new QCameraImageCapture(m_camera, this);

        // 设置取景器
        m_camera->setViewfinder(m_viewfinder);
        m_viewfinder->setMinimumSize(400, 300);

        // 将取景器添加到视频标签
        QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(m_senderVideoLabel->layout());
        if (!layout) {
            layout = new QVBoxLayout(m_senderVideoLabel);
            m_senderVideoLabel->setLayout(layout);
        }

        // 清空之前的控件
        QLayoutItem *child;
        while ((child = layout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }

        layout->addWidget(m_viewfinder);

        // 连接错误信号
        connect(m_camera, QOverload<QCamera::Error>::of(&QCamera::error),
                this, [this](QCamera::Error error) {
                    qCritical() << "Camera error:" << error << m_camera->errorString();
                    m_senderStatusLabel->setText("❌ 摄像头错误: " + m_camera->errorString());
                    onStopCamera();
                });

        // 设置音视频发送器的摄像头
        if (m_avSender) {
            m_avSender->setCamera(m_camera);
        }

        // 启动摄像头
        qDebug() << "Starting camera...";
        m_camera->start();

        // 给摄像头一些时间初始化
        QTimer::singleShot(100, this, [this]() {
            if (m_camera && m_camera->state() == QCamera::ActiveState) {
                // 更新按钮状态
                m_startButton->setEnabled(false);
                m_stopButton->setEnabled(true);
                m_startStreamButton->setEnabled(true);
                m_cameraComboBox->setEnabled(false);

                m_senderStatusLabel->setText("✅ 摄像头已启动");
                addSystemMessage("📷 摄像头启动成功");
                qDebug() << "Camera started successfully";
            } else {
                m_senderStatusLabel->setText("❌ 摄像头启动失败");
                addSystemMessage("❌ 摄像头启动失败");
                qWarning() << "Camera failed to start";
                onStopCamera();
            }
        });

    } catch (const std::exception &e) {
        qCritical() << "Exception while starting camera:" << e.what();
        m_senderStatusLabel->setText(QString("❌ 启动失败: %1").arg(e.what()));
        onStopCamera();
        QMessageBox::critical(this, "错误", QString("摄像头启动失败: %1").arg(e.what()));
    }
}

void MainWindow::onStopCamera()
{
    qDebug() << "onStopCamera called";

    // 先停止推流和屏幕共享
    onStopStreaming();
    onStopScreenSharing();

    if (m_camera) {
        qDebug() << "Stopping camera...";
        m_camera->stop();
        disconnect(m_camera, nullptr, this, nullptr);
        delete m_camera;
        m_camera = nullptr;
        qDebug() << "Camera stopped";
    }

    if (m_viewfinder) {
        m_viewfinder->deleteLater();
        m_viewfinder = nullptr;
    }

    if (m_imageCapture) {
        disconnect(m_imageCapture, nullptr, this, nullptr);
        m_imageCapture->deleteLater();
        m_imageCapture = nullptr;
    }

    // 恢复视频标签显示
    m_senderVideoLabel->setText("📷 摄像头未启动");
    QLayoutItem *child;
    if (m_senderVideoLabel->layout()) {
        while ((child = m_senderVideoLabel->layout()->takeAt(0)) != nullptr) {
            if (child->widget()) {
                child->widget()->deleteLater();
            }
            delete child;
        }
    }

    // 更新按钮状态
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_startStreamButton->setEnabled(false);
    m_stopStreamButton->setEnabled(false);
    m_cameraComboBox->setEnabled(true);

    m_senderStatusLabel->setText("⏹️ 摄像头已停止");
    addSystemMessage("📷 摄像头已停止");
}

void MainWindow::onCameraChanged(int index)
{
    Q_UNUSED(index)
    qDebug() << "Camera selection changed to index:" << index;
}

void MainWindow::onRoomIdChanged(int roomId)
{
    m_roomId = static_cast<uint32_t>(roomId);
    qDebug() << "Room ID changed to:" << m_roomId;
    m_senderStatusLabel->setText(QString("🏠 房间ID已设置为: %1").arg(m_roomId));
    addSystemMessage(QString("🏠 切换到房间 %1").arg(m_roomId));
}

void MainWindow::onVideoFpsSliderChanged(int fps)
{
    m_videoFpsLabel->setText(QString("%1 FPS").arg(fps));
    if (m_avSender) {
        m_avSender->setVideoFps(fps);
    }
    m_senderStatusLabel->setText(QString("📊 视频帧率设置为: %1 FPS").arg(fps));
}

void MainWindow::onAudioSampleRateChanged(const QString& sampleRate)
{
    int rate = sampleRate.toInt();
    if (rate > 0 && m_avSender) {
        m_avSender->setAudioSampleRate(rate);
        m_audioStatusLabel->setText(QString("🔊 %1 Hz").arg(sampleRate));
    }
}

void MainWindow::onScreenModeChanged(int index)
{
    qDebug() << "Screen mode changed to:" << index;
    QString modeText;
    switch (index) {
    case 0: modeText = "全屏共享"; break;
    case 1: modeText = "活动窗口"; break;
    case 2: modeText = "选择区域"; break;
    default: modeText = "未知模式"; break;
    }
    m_senderStatusLabel->setText(QString("🖥️ 屏幕模式设置为: %1").arg(modeText));
}

void MainWindow::onScreenFpsChanged(int fps)
{
    m_screenFpsLabel->setText(QString("%1 FPS").arg(fps));
    m_senderStatusLabel->setText(QString("📊 屏幕帧率设置为: %1 FPS").arg(fps));

    // 更新屏幕共享配置
    if (m_avSender && m_avSender->isScreenSharing()) {
        ScreenCaptureConfig config;
        config.mode = static_cast<ScreenCaptureMode>(m_screenModeCombo->currentIndex());
        config.fps = fps;
        config.includeCursor = m_includeCursorCheck->isChecked();
        m_avSender->updateScreenConfig(config);
    }
}

void MainWindow::onStartStreaming()
{
    if (!m_camera || m_camera->state() != QCamera::ActiveState) {
        QMessageBox::warning(this, "错误", "请先启动摄像头");
        return;
    }

    // 先停止屏幕共享
    onStopScreenSharing();

    int audioSampleRate = m_audioSampleRateCombo->currentText().toInt();
    if (m_avSender->startStreaming(m_roomId, m_videoFpsSlider->value(), audioSampleRate)) {
        m_startStreamButton->setEnabled(false);
        m_stopStreamButton->setEnabled(true);
        m_startScreenButton->setEnabled(false);
        m_senderStatusLabel->setText(QString("📡 正在推流 - 视频: %1 FPS, 音频: %2 Hz")
                                   .arg(m_videoFpsSlider->value())
                                   .arg(audioSampleRate));
        addSystemMessage(QString("📡 开始音视频推流 (视频: %1 FPS, 音频: %2 Hz)")
                       .arg(m_videoFpsSlider->value()).arg(audioSampleRate));
    }
    if (m_autoRecordCheck->isChecked() && !m_isRecording) {
            onStartRecording();
        }
}

void MainWindow::onStopStreaming()
{
    if (m_avSender) {
        m_avSender->stopStreaming();
    }
    m_startStreamButton->setEnabled(true);
    m_stopStreamButton->setEnabled(false);
    m_startScreenButton->setEnabled(true);
    m_senderStatusLabel->setText("⏹️ 推流已停止");
    addSystemMessage("⏹️ 音视频推流已停止");
    if (m_isRecording) {
            onStopRecording();
        }
}

void MainWindow::onStartScreenSharing()
{
    // 先停止摄像头推流
    onStopStreaming();

    ScreenCaptureConfig config;
    config.mode = static_cast<ScreenCaptureMode>(m_screenModeCombo->currentIndex());
    config.fps = m_screenFpsSpin->value();
    config.includeCursor = m_includeCursorCheck->isChecked();

    if (m_avSender->startScreenSharing(m_roomId, config)) {
        m_startScreenButton->setEnabled(false);
        m_stopScreenButton->setEnabled(true);
        m_startStreamButton->setEnabled(false);
        m_senderStatusLabel->setText(QString("📺 屏幕共享已开始 - %1 FPS").arg(config.fps));
        addSystemMessage(QString("📺 开始屏幕共享 (%1 FPS)").arg(config.fps));
    }
    if (m_autoRecordCheck->isChecked() && !m_isRecording) {
            onStartRecording();
        }
}

void MainWindow::onStopScreenSharing()
{
    if (m_avSender) {
        m_avSender->stopScreenSharing();
    }
    m_startScreenButton->setEnabled(true);
    m_stopScreenButton->setEnabled(false);
    m_startStreamButton->setEnabled(true);
    m_senderStatusLabel->setText("⏹️ 屏幕共享已停止");
    addSystemMessage("⏹️ 屏幕共享已停止");
    if (m_isRecording) {
            onStopRecording();
        }
}

void MainWindow::onStartReceiver()
{
    uint32_t roomId = static_cast<uint32_t>(m_receiverRoomIdSpinBox->value());
    m_avReceiver->startReceiving(roomId);

    m_startReceiverButton->setEnabled(false);
    m_stopReceiverButton->setEnabled(true);
    m_receiverRoomIdSpinBox->setEnabled(false);

    m_receiverStatusLabel->setText(QString("📡 正在接收房间 %1").arg(roomId));
    addSystemMessage(QString("📡 开始接收房间 %1 的内容").arg(roomId));

    qDebug() << "Receiver started for room:" << roomId;
}

void MainWindow::onStopReceiver()
{
    if (m_avReceiver) {
        m_avReceiver->stopReceiving();
    }

    m_startReceiverButton->setEnabled(true);
    m_stopReceiverButton->setEnabled(false);
    m_receiverRoomIdSpinBox->setEnabled(true);

    m_receiverStatusLabel->setText("⏹️ 接收已停止");
    addSystemMessage("⏹️ 停止接收内容");

    qDebug() << "Receiver stopped";
}

void MainWindow::onReceiverRoomIdChanged(int roomId)
{
    if (m_avReceiver && m_avReceiver->isReceiving()) {
        m_avReceiver->stopReceiving();
        m_avReceiver->startReceiving(static_cast<uint32_t>(roomId));
        m_receiverStatusLabel->setText(QString("📡 切换到房间 %1").arg(roomId));
        addSystemMessage(QString("📡 切换到接收房间 %1").arg(roomId));
        qDebug() << "Receiver room ID changed to:" << roomId;
    }
}

void MainWindow::onSendMessage()
{
    QString messageText = m_messageEdit->text().trimmed();
    if (messageText.isEmpty()) {
        return;
    }

    // 打包文字消息
    QByteArray packet = ProtocolPackager::packTextMessage(
        m_roomId,
        m_userId,
        m_userName,
        messageText
    );

    // 发送到接收端
    if (m_avReceiver && m_avReceiver->isReceiving()) {
        m_avReceiver->processReceivedData(packet);
    }

    // 在本地显示
    TextMessage message;
    message.roomId = m_roomId;
    message.userId = m_userId;
    message.userName = m_userName;
    message.content = messageText;
    message.timestamp = QDateTime::currentDateTime();
    message.type = TextMessageType::NORMAL;

    addChatMessage(message, true);
    m_sentMessages++;
    updateStatsDisplay();

    m_messageEdit->clear();
}

void MainWindow::onMessageTextChanged(const QString& text)
{
    m_sendMessageButton->setEnabled(!text.trimmed().isEmpty());
}

void MainWindow::onUserNameChanged(const QString& name)
{
    m_userName = name.isEmpty() ? "用户" + QString::number(m_userId) : name;
    addSystemMessage(QString("👤 用户名改为: %1").arg(m_userName));
}

void MainWindow::onAudioEnabledToggled(bool enabled)
{
    if (m_avReceiver) {
        m_avReceiver->setAudioEnabled(enabled);
    }
    m_audioStatusLabel->setText(enabled ? "🔊 音频已启用" : "🔇 音频已禁用");
    addSystemMessage(enabled ? "🔊 音频已启用" : "🔇 音频已禁用");
}

void MainWindow::onVideoEnabledToggled(bool enabled)
{
    if (m_avReceiver) {
        m_avReceiver->setVideoEnabled(enabled);
    }
    addSystemMessage(enabled ? "📹 视频已启用" : "📹 视频已禁用");
}

void MainWindow::onDataPackaged(const QByteArray& packet)
{
    // 自动发送到接收端
    if (m_avReceiver && m_avReceiver->isReceiving()) {
        m_avReceiver->processReceivedData(packet);
    }
}

void MainWindow::onStreamingStarted()
{
    m_senderStatusLabel->setText("📡 音视频推流已开始");
    addSystemMessage("📡 音视频推流已开始");
}

void MainWindow::onStreamingStopped()
{
    m_senderStatusLabel->setText("⏹️ 音视频推流已停止");
}

void MainWindow::onScreenSharingStarted()
{
    m_senderStatusLabel->setText("📺 屏幕共享已开始");
}

void MainWindow::onScreenSharingStopped()
{
    m_senderStatusLabel->setText("⏹️ 屏幕共享已停止");
}

void MainWindow::onSenderErrorOccurred(const QString& error)
{
    m_senderStatusLabel->setText("❌ 错误: " + error);
    addSystemMessage("❌ 发送错误: " + error);
    QMessageBox::warning(this, "发送错误", error);
}

void MainWindow::onVideoFpsChanged(int fps)
{
    m_senderStatusLabel->setText(QString("📊 实际视频帧率: %1 FPS").arg(fps));
}

void MainWindow::onScreenFpsReported(int fps)
{
    m_screenFpsLabel->setText(QString("%1 FPS").arg(fps));
    m_senderStatusLabel->setText(QString("📊 实际屏幕帧率: %1 FPS").arg(fps));
}

void MainWindow::onAudioStatusChanged(const QString& status)
{
    m_audioStatusLabel->setText(status);
}

void MainWindow::onReceiverStatusChanged(const QString &status)
{
    m_receiverStatusLabel->setText(status);
}

void MainWindow::onReceiverErrorOccurred(const QString &error)
{
    m_receiverStatusLabel->setText("❌ 错误: " + error);
    addSystemMessage("❌ 接收错误: " + error);
    QMessageBox::warning(this, "接收错误", error);
}

void MainWindow::onTextMessageReceived(const TextMessage& message)
{
    addChatMessage(message, false);
    m_receivedMessages++;
    updateStatsDisplay();
}

void MainWindow::onSystemMessageReceived(const QString& message)
{
    addSystemMessage(message);
}

void MainWindow::onVideoStatsChanged(int fps, int totalFrames)
{
    m_videoStatsLabel->setText(QString("📹 视频: %1 FPS, 总帧数: %2").arg(fps).arg(totalFrames));
}

void MainWindow::onAudioStatsChanged(int totalPackets)
{
    m_audioStatsLabel->setText(QString("🎵 音频: 总包数: %1").arg(totalPackets));
}

void MainWindow::onScreenStatsChanged(int fps, int totalFrames)
{
    m_screenStatsLabel->setText(QString("🖥️ 屏幕: %1 FPS, 总帧数: %2").arg(fps).arg(totalFrames));
}

void MainWindow::addChatMessage(const TextMessage& message, bool isOwnMessage)
{
    QString html = getMessageStyle(message, isOwnMessage);

    // 保存当前滚动位置
    QScrollBar *scrollBar = m_chatDisplay->verticalScrollBar();
    bool atBottom = scrollBar->value() == scrollBar->maximum();

    // 添加消息
    m_chatDisplay->append(html);

    // 如果之前在最底部，保持在最底部
    if (atBottom) {
        scrollChatToBottom();
    }

    // 播放提示音
    QApplication::beep();
}

void MainWindow::addSystemMessage(const QString& message)
{
    QString html = QString(R"(
        <div style="margin: 5px 0; padding: 8px;
                   background: linear-gradient(135deg, #6c5ce7, #a29bfe);
                   border-radius: 10px; color: black; text-align: center;
                   font-style: italic; border: 1px solid #6c5ce7;">
            <span style="font-weight: bold;">⚡ %1</span>
        </div>
    )").arg(message);

    m_chatDisplay->append(html);
    scrollChatToBottom();
}

QString MainWindow::getMessageStyle(const TextMessage& message, bool isOwnMessage) const
{
    QString align = isOwnMessage ? "right" : "left";
    QString bgColor = isOwnMessage ? "#3498db" : "#ecf0f1";
    QString textColor = isOwnMessage ? "white" : "#2c3e50";
    QString borderColor = isOwnMessage ? "#2980b9" : "#bdc3c7";

    return QString(R"(
        <div style="margin: 8px 0; text-align: %1;">
            <div style="display: inline-block; max-width: 80%%;
                       background: %2; color: %3;
                       border: 2px solid %4; border-radius: 15px;
                       padding: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1);">
                <div style="font-weight: bold; font-size: 12px; margin-bottom: 3px;">
                    %5 <span style="color: %6; font-size: 10px;">%7</span>
                </div>
                <div style="font-size: 13px; line-height: 1.4; word-wrap: break-word;">
                    %8
                </div>
            </div>
        </div>
    )").arg(align)
      .arg(bgColor)
      .arg(textColor)
      .arg(borderColor)
      .arg(message.userName)
      .arg(isOwnMessage ? "#ecf0f1" : "#7f8c8d")
      .arg(message.timestamp.toString("hh:mm:ss"))
      .arg(message.content.toHtmlEscaped());
}

void MainWindow::scrollChatToBottom()
{
    QScrollBar *scrollBar = m_chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::updateStatsDisplay()
{
    m_messageStatsLabel->setText(QString("💬 消息: 收到: %1, 发送: %2")
                               .arg(m_receivedMessages).arg(m_sentMessages));
}

void MainWindow::updateStatus(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_senderStatusLabel->setText(QString("[%1] %2").arg(timestamp).arg(message));
}
// 添加流统计更新函数
void MainWindow::onStreamStatsUpdated(const QMap<QString, int>& stats) {
    m_streamStats = stats;
    updateRecordStatsDisplay();
}

void MainWindow::updateRecordStatsDisplay() {
    if (!m_isRecording) return;

    QStringList statsText;
    for (auto it = m_streamStats.constBegin(); it != m_streamStats.constEnd(); ++it) {
        statsText.append(QString("%1: %2帧").arg(it.key()).arg(it.value()));
    }

    if (!statsText.isEmpty()) {
        m_recordStatsLabel->setText(statsText.join(" | "));
    }
}

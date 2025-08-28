#include "mainwindow.h"

/** 构造函数：初始化UI控件、连接信号槽 */
MainWindow::MainWindow() {
    QWidget* w = new QWidget;
    auto lay = new QVBoxLayout(w);

    auto row1 = new QHBoxLayout;
    edHost = new QLineEdit("127.0.0.1");
    edPort = new QLineEdit("9000");
    edPort->setMaximumWidth(80);
    QPushButton* btnConn = new QPushButton("连接");
    row1->addWidget(new QLabel("Host:")); row1->addWidget(edHost);
    row1->addWidget(new QLabel("Port:")); row1->addWidget(edPort);
    row1->addWidget(btnConn);
    lay->addLayout(row1);

    auto row2 = new QHBoxLayout;
    edUser = new QLineEdit("factory-A");
    edRoom = new QLineEdit("R123");
    QPushButton* btnJoin = new QPushButton("加入工单");
    row2->addWidget(new QLabel("User:")); row2->addWidget(edUser);
    row2->addWidget(new QLabel("RoomId:")); row2->addWidget(edRoom);
    row2->addWidget(btnJoin);
    lay->addLayout(row2);

    txtLog = new QTextEdit; txtLog->setReadOnly(true);
    lay->addWidget(txtLog);

    auto row3 = new QHBoxLayout;
    edInput = new QLineEdit;
    QPushButton* btnSend = new QPushButton("发送文本");
    row3->addWidget(edInput); row3->addWidget(btnSend);
    lay->addLayout(row3);

    setCentralWidget(w);
    setWindowTitle("Factory Client (最小骨架)");

    // signals
    connect(btnConn, &QPushButton::clicked, this, &MainWindow::onConnect);
    connect(btnJoin, &QPushButton::clicked, this, &MainWindow::onJoin);
    connect(btnSend, &QPushButton::clicked, this, &MainWindow::onSendText);
    connect(&conn_, &ClientConn::packetArrived, this, &MainWindow::onPkt);
}

// 连接到服务器（使用Host/Port）
/** 槽：连接服务器 */
void MainWindow::onConnect() {
    conn_.connectTo(edHost->text(), edPort->text().toUShort());
    txtLog->append("Connecting...");
}
// 加入工单会议（发送房间号与用户名到服务器）
/** 槽：加入工单（发送房间与用户名） */
void MainWindow::onJoin() {
    QJsonObject j{{"roomId", edRoom->text()},
                  {"user", edUser->text()}};
    conn_.send(MSG_JOIN_WORKORDER, j);
}
/** 槽：发送文本（并在本端日志回显） */
void MainWindow::onSendText() {
    // 发送文本：构造消息JSON并通过网络发送；同时在本端日志中也显示这条消息，
    // 这样双方界面都能看到完整对话（包括自己发送的消息）。
QJsonObject j{{"roomId", edRoom->text()},
                  {"sender", edUser->text()},
                  {"content", edInput->text()},
                  {"ts", QDateTime::currentMSecsSinceEpoch()}};
    // 本端追加显示（sender 使用本端用户名，格式与接收端一致）
    do {
        QString s = QString("[%1] %2: %3")
            .arg(edRoom->text())
            .arg(edUser->text())
            .arg(edInput->text());
        txtLog->append(s);
    } while(0);

    conn_.send(MSG_TEXT, j);
    edInput->clear();
}

// 处理网络收到的数据包（文本消息、服务器事件等）
/** 槽：处理收到的Packet（文本、服务器事件等） */
void MainWindow::onPkt(Packet p) {
    if (p.type == MSG_TEXT) {
        QString s = QString("[%1] %2: %3")
            .arg(p.json.value("roomId").toString())
            .arg(p.json.value("sender").toString())
            .arg(p.json.value("content").toString());
        txtLog->append(s);
    } else if (p.type == MSG_SERVER_EVENT) {
        txtLog->append(QString("[server] %1").arg(QString::fromUtf8(QJsonDocument(p.json).toJson(QJsonDocument::Compact))));
    } else {
        txtLog->append(QString("[type %1] recv").arg(p.type));
    }
}

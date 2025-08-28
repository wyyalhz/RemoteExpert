#include "mainwindow.h"

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

void MainWindow::onConnect() {
    conn_.connectTo(edHost->text(), edPort->text().toUShort());
    txtLog->append("Connecting...");
}
void MainWindow::onJoin() {
    QJsonObject j{{"roomId", edRoom->text()},
                  {"user", edUser->text()}};
    conn_.send(MSG_JOIN_WORKORDER, j);
}
void MainWindow::onSendText() {
    QJsonObject j{{"roomId", edRoom->text()},
                  {"sender", edUser->text()},
                  {"content", edInput->text()},
                  {"ts", QDateTime::currentMSecsSinceEpoch()}};
    conn_.send(MSG_TEXT, j);
    edInput->clear();
}

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

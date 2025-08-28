#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonDocument>

MainWindow::MainWindow()
    : QMainWindow(nullptr), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // bind ui elements to members to keep existing member names usable
    edHost  = ui->edHost;
    edPort  = ui->edPort;
    edUser  = ui->edUser;
    edRoom  = ui->edRoom;
    edInput = ui->edInput;
    txtLog  = ui->txtLog;

    // wire buttons
    connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::onConnect);
    connect(ui->btnJoin,    &QPushButton::clicked, this, &MainWindow::onJoin);
    connect(ui->btnSend,    &QPushButton::clicked, this, &MainWindow::onSendText);

    // network packets
    connect(&conn_, &ClientConn::packetArrived, this, &MainWindow::onPkt);
}

void MainWindow::onConnect() {
    conn_.connectTo(edHost->text(), edPort->text().toUShort());
    txtLog->append(QStringLiteral("Connecting..."));
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
    // 本地也追加一条消息，避免等待服务器回显
    {
        QString s = QString("[%1] %2: %3")
            .arg(edRoom->text())
            .arg(edUser->text())
            .arg(ui->edInput->text());
        txtLog->append(s);
    }
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

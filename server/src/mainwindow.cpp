#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonDocument>
#include <QDateTime>
#include <QMetaObject>

static ServerWindow* gServerWin = nullptr;

ServerWindow::ServerWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::ServerWindow) {
    ui->setupUi(this);
    connect(ui->btnStartStop, &QPushButton::clicked, this, &ServerWindow::onStartStop);
    gServerWin = this;
    qInstallMessageHandler(ServerWindow::messageHandler);
}

ServerWindow::~ServerWindow(){
    qInstallMessageHandler(nullptr);
    delete ui;
}

void ServerWindow::onStartStop() {
    if (!started_) {
        bool ok=false;
        quint16 port = ui->edPort->text().toUShort(&ok);
        if (!ok) { QMessageBox::warning(this, "Error", "端口号无效"); return; }
        if (!hub_.start(port)) {
            QMessageBox::critical(this, "启动失败", "监听失败，请检查端口是否被占用。");
            return;
        }
        started_ = true;
        ui->btnStartStop->setText(QStringLiteral("停止"));
        ui->txtLog->append(QString("[UI] Server started on port %1").arg(port));
    } else {
        // 简化：RoomHub 没有 stop()，这里直接关闭进程或留空
        QMessageBox::information(this, "提示", "当前最小实现不支持停止后再次启动。\n请重启程序以更改端口。");
    }
}

void ServerWindow::messageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    Q_UNUSED(ctx);
    if (!gServerWin) return;
    QString prefix;
    switch (type) {
    case QtDebugMsg: prefix = "[DBG] "; break;
    case QtInfoMsg: prefix  = "[INF] "; break;
    case QtWarningMsg: prefix = "[WRN] "; break;
    case QtCriticalMsg: prefix = "[CRT] "; break;
    case QtFatalMsg: prefix = "[FTL] "; break;
    }
    const QString line = QString("%1%2").arg(prefix, msg);
    QMetaObject::invokeMethod(gServerWin->ui->txtLog, "append", Qt::QueuedConnection, Q_ARG(QString, line));
}

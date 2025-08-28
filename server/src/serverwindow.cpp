#include "serverwindow.h"
#include "ui_serverwindow.h"

QTextEdit* ServerWindow::s_logView = nullptr;

/** 构造：加载 .ui，连接按钮信号，安装日志回调 */
ServerWindow::ServerWindow(QWidget* parent) : QMainWindow(parent),
    ui(new Ui::ServerWindow) {
    ui->setupUi(this);
    connect(ui->btnStart, &QPushButton::clicked, this, &ServerWindow::onStart);
    connect(ui->btnStop,  &QPushButton::clicked, this, &ServerWindow::onStop);
    installLogHandler();
}

/** 析构 */
ServerWindow::~ServerWindow() { delete ui; }

/** 槽：启动服务器监听 */
void ServerWindow::onStart() {
    bool ok=false;
    quint16 port = ui->edPort->text().toUShort(&ok);
    if (!ok || port==0) { QMessageBox::warning(this, "提示", "端口无效"); return; }

    if (!hub_.start(port)) {
        QMessageBox::critical(this, "启动失败", "监听失败，请检查端口占用或权限。");
        return;
    }
    statusBar()->showMessage(QString("已启动：%1").arg(port));
}

/** 槽：停止服务
 *  保持原有 RoomHub 简实现（未实现 stop()），这里仅给出提示。
 */
void ServerWindow::onStop() {
    QMessageBox::information(this, "提示", "当前版本未实现平滑停止，请重启程序以修改端口。");
}

/** 安装日志处理，把 Qt 日志写入到 UI 文本框 */
void ServerWindow::installLogHandler() {
    s_logView = ui->txtLog;
    qInstallMessageHandler(&ServerWindow::qtLogHandler);
}

/** 静态日志回调：线程不安全的简实现（用于演示），将日志追加到文本框 */
void ServerWindow::qtLogHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    Q_UNUSED(ctx);
    if (!s_logView) return;
    QString pfx;
    switch (type) {
    case QtDebugMsg: pfx = "[debug] "; break;
    case QtInfoMsg: pfx = "[info] "; break;
    case QtWarningMsg: pfx = "[warn] "; break;
    case QtCriticalMsg: pfx = "[crit] "; break;
    case QtFatalMsg: pfx = "[fatal] "; break;
    }
    s_logView->append(pfx + msg);
}

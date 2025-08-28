#pragma once
#include <QtWidgets>
#include "roomhub.h"

namespace Ui { class ServerWindow; }

/**
 * ServerWindow
 * 中文说明：服务端可视化面板（基于 .ui），保留原有 RoomHub 逻辑。
 *  - 输入端口 -> 启动监听
 *  - 全局日志重定向到窗口
 */
class ServerWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit ServerWindow(QWidget* parent=nullptr);
    ~ServerWindow();
private slots:
    void onStart();   // 启动服务（listen）
    void onStop();    // 停止服务（简单释放重启提示）
private:
    Ui::ServerWindow* ui;
    RoomHub hub_;
    void installLogHandler();   // 把 qInfo/qWarning 等重定向到 txtLog
    static void qtLogHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg);
    static QTextEdit* s_logView;
};

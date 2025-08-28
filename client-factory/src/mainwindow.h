#pragma once
#include <QtWidgets>
#include "clientconn.h"

// 中文注释：UI主窗口——完成 连接服务器 → 加入工单 → 发送文本 的最小闭环
/**
 * MainWindow
 * 中文说明：
 *  - 提供连接服务器、加入工单、收发消息的图形界面
 *  - 槽函数：onConnect/onJoin/onSendText/onPkt
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
private slots:
    void onConnect();   // 连接服务器
    void onJoin();      // 加入工单（房间）
    void onSendText(); // 发送文本消息（并在本端回显）
    void onPkt(Packet p); // 处理收到的数据包
private:
    ClientConn conn_;
    // UI控件
    QLineEdit *edHost, *edPort, *edUser, *edRoom, *edInput;
    QTextEdit *txtLog;
};

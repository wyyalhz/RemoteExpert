#pragma once
#include <QtWidgets>
#include "clientconn.h"

// 一个最简的UI：连接->加入工单->发文字
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
private slots:
    void onConnect();
    void onJoin();
    void onSendText();
    void onPkt(Packet p);
private:
    ClientConn conn_;
    // UI控件
    QLineEdit *edHost, *edPort, *edUser, *edRoom, *edInput;
    QTextEdit *txtLog;
};

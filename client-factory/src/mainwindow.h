#pragma once
#include <QtWidgets>
#include "clientconn.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

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
    
    Ui::MainWindow *ui;
ClientConn conn_;
    // UI控件
    QLineEdit *edHost, *edPort, *edUser, *edRoom, *edInput;
    QTextEdit *txtLog;
};

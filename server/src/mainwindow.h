#pragma once
#include <QtWidgets>
#include "roomhub.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ServerWindow; }
QT_END_NAMESPACE

class ServerWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit ServerWindow(QWidget* parent=nullptr);
    ~ServerWindow();
private slots:
    void onStartStop();
private:
    Ui::ServerWindow* ui;
    RoomHub hub_;
    bool started_ = false;
    static void messageHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& msg);
};

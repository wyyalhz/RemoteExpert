// ===============================================
// main.cpp
// 程序入口
// ===============================================

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序信息
    QApplication::setApplicationName("摄像头协议打包工具");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("YourCompany");

    MainWindow w;
    w.show();

    return a.exec();
}

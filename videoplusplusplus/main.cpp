// ===============================================
// main.cpp
// 程序入口
// ===============================================

#include <QApplication>
#include "mainwindow.h"
#include <QCoreApplication>
#include <QDebug>
#include "networkmanager.h"

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

//int main(int argc, char *argv[])
//{
//    QCoreApplication a(argc, argv);

//    NetworkManager net;

//    QObject::connect(&net, &NetworkManager::connected, []() {
//        qDebug() << "✅ 已连接服务器";
//    });

//    QObject::connect(&net, &NetworkManager::errorOccurred, [](const QString& err) {
//        qDebug() << "❌ 出错:" << err;
//    });

//    QObject::connect(&net, &NetworkManager::packetReceived, [](const Packet& pkt) {
//        qDebug() << "📥 收到包 type=" << pkt.type << " json=" << pkt.json << " bin.size=" << pkt.bin.size();
//    });

//    // 连接到服务器
//    net.connectToServer("0.0.0.0", 8080);

//    // 延时一点再发登录消息
//    QTimer::singleShot(1000, [&net]() {
//        QJsonObject login = MessageBuilder::buildLoginMessage("user1", "123456", 1);
//        net.sendPacket(2, login);  // 假设 1001 = 登录请求
//        qDebug() << "🚀 已发送登录请求";
//    });

//    return a.exec();
//}

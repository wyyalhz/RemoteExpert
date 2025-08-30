#include <QtCore>    //Qt基本功能支持
#include <QtNetwork> //网络相关功能支持
#include "roomhub.h" //该类实现服务器核心功能
#include <QSqlDatabase>
#include <QDebug>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser; //解析用户输入的命令行参数
    parser.addHelpOption();    // 为解析器添加帮助选项（-h或--help），自动生成帮助信息

    // 定义端口选项：短参数-p，长参数-port，描述为"监听端口"
    // 第三个参数是参数值的占位符，第四个参数是默认值9000
    QCommandLineOption portOpt(
        QStringList() << "p" << "port",  // 短选项和长选项
        "Listen port",                   // 选项描述
        "port",                          // 参数值名称（用于帮助信息）
        "9000"                           // 默认值
    );
    // 将端口选项添加到解析器中
    parser.addOption(portOpt);
    // 处理命令行参数，应用到应用程序中
    parser.process(app);

    // 从命令行解析器中获取端口值
    quint16 port = parser.value(portOpt).toUShort();

    RoomHub hub;

    // 启动服务器，尝试在指定端口上监听连接
    if (!hub.start(port))
    {
        qWarning()<<"Listen failed on port"<<port<<":"<<hub.lastError();
        return 1;
    }

    // 告知用户客户端应连接的服务器端口
    qInfo() << "Usage: clients connect to server_ip:" << port;

    // 进入Qt应用程序的事件循环，等待并处理各种事件（如网络连接、数据传输等）
    // 程序将一直运行直到调用quit()或exit()
    return app.exec();
}

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QHostAddress>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QTimer>

// 数据层
#include "data/databasemanager.h"

// 业务逻辑层
#include "business/services/user_service.h"
#include "business/services/workorder_service.h"

// 网络层
#include "network/network_server.h"

// 日志系统
#include "logging/managers/log_manager.h"

// 公共协议
#include "../../common/protocol/protocol.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("RemoteExpert Server");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CSFI");
    
    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("RemoteExpert远程专家系统服务器");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加自定义选项
    QCommandLineOption portOption(QStringList() << "p" << "port",
                                 "服务器端口号 (默认: 8080)",
                                 "port", "8080");
    parser.addOption(portOption);
    
    QCommandLineOption hostOption(QStringList() << "h" << "host",
                                 "服务器监听地址 (默认: 0.0.0.0)",
                                 "host", "0.0.0.0");
    parser.addOption(hostOption);
    
    QCommandLineOption dbPathOption(QStringList() << "d" << "database",
                                   "数据库文件路径",
                                   "path", "data/remote_expert.db");
    parser.addOption(dbPathOption);
    
    QCommandLineOption logLevelOption(QStringList() << "l" << "log-level",
                                     "日志级别 (DEBUG, INFO, WARNING, ERROR, CRITICAL)",
                                     "level", "INFO");
    parser.addOption(logLevelOption);
    
    QCommandLineOption logFileOption(QStringList() << "f" << "log-file",
                                    "日志文件路径",
                                    "path", "logs/server.log");
    parser.addOption(logFileOption);
    
    parser.process(app);
    
    // 获取参数值
    quint16 port = parser.value(portOption).toUShort();
    QString hostStr = parser.value(hostOption);
    QString dbPath = parser.value(dbPathOption);
    QString logLevelStr = parser.value(logLevelOption);
    QString logFilePath = parser.value(logFileOption);
    
    // 解析日志级别
    LogLevel logLevel = LogLevel::INFO;
    if (logLevelStr.toUpper() == "DEBUG") logLevel = LogLevel::DEBUG;
    else if (logLevelStr.toUpper() == "WARNING") logLevel = LogLevel::WARNING;
    else if (logLevelStr.toUpper() == "ERROR") logLevel = LogLevel::ERROR;
    else if (logLevelStr.toUpper() == "CRITICAL") logLevel = LogLevel::CRITICAL;
    
    // 解析主机地址
    QHostAddress hostAddress;
    if (hostStr == "0.0.0.0" || hostStr.isEmpty()) {
        hostAddress = QHostAddress::Any;
    } else {
        hostAddress = QHostAddress(hostStr);
        if (hostAddress.isNull()) {
            qCritical() << "无效的主机地址:" << hostStr;
            return 1;
        }
    }
    
    // 确保日志目录存在
    QFileInfo logFileInfo(logFilePath);
    QDir logDir = logFileInfo.absoluteDir();
    if (!logDir.exists()) {
        logDir.mkpath(".");
    }
    
    // 初始化日志系统
    LogManager* logManager = LogManager::getInstance();
    logManager->initialize(logLevel, logFilePath);
    
    qInfo() << "=== RemoteExpert 服务器启动 ===";
    qInfo() << "版本:" << app.applicationVersion();
    qInfo() << "监听地址:" << hostAddress.toString() << ":" << port;
    qInfo() << "数据库路径:" << dbPath;
    qInfo() << "日志级别:" << logLevelStr;
    qInfo() << "日志文件:" << logFilePath;
    
    // 创建数据库管理器
    DatabaseManager* dbManager = new DatabaseManager(&app);
    if (!dbManager->initialize()) {
        qCritical() << "数据库初始化失败";
        return 1;
    }
    qInfo() << "数据库初始化成功";
    
    // 创建业务服务
    UserService* userService = new UserService(dbManager, &app);
    WorkOrderService* workOrderService = new WorkOrderService(dbManager, &app);
    qInfo() << "业务服务初始化成功";
    
    // 创建网络服务器
    NetworkServer* networkServer = new NetworkServer(&app);
    if (!networkServer->initialize(userService, workOrderService)) {
        qCritical() << "网络服务器初始化失败";
        return 1;
    }
    qInfo() << "网络服务器初始化成功";
    
    // 启动网络服务器
    if (!networkServer->start(hostAddress, port)) {
        qCritical() << "网络服务器启动失败";
        return 1;
    }
    qInfo() << "网络服务器启动成功，开始监听连接...";
    
    // 设置优雅关闭
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [=]() {
        qInfo() << "正在关闭服务器...";
        networkServer->stop();
        logManager->cleanup();
        qInfo() << "服务器已关闭";
    });
    
    // 处理Ctrl+C信号
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&app]() {
        app.quit();
    });
    
    qInfo() << "服务器运行中，按 Ctrl+C 退出...";
    
    // 启动事件循环
    int result = app.exec();
    
    // 清理资源
    delete networkServer;
    delete workOrderService;
    delete userService;
    delete dbManager;
    
    return result;
}

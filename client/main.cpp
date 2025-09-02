#include "Presentation/main_window/home_main_window.h"
#include "Presentation/dialogs/LoginDialog/login_dialog.h"
#include "Presentation/utils/theme.h"
#include "Logger/log_manager.h"
#include "Network/client/network_client.h"
#include "Business/services/AuthService.h"
#include "Business/services/TicketService.h"

#include <QApplication>
#include <QDebug>
#include <QStandardPaths>
#include <QObject>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QApplication::setStyle("fusion"); // 建议，基底更统一

    // 初始化日志系统
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/RemoteExpert/logs";
    QString logFile = logDir + "/client.log";
    LogManager::getInstance()->initialize(LogLevel::DEBUG, logFile);
    
    LogManager::getInstance()->info(LogModule::SYSTEM, LogLayer::BUSINESS, 
                                   "Main", "客户端应用程序启动");

    // 初始化网络客户端
    NetworkClient* networkClient = new NetworkClient();
    
    // 初始化业务服务
    AuthService* authService = new AuthService();
    TicketService* ticketService = new TicketService();
    
    // 设置网络客户端到业务服务
    authService->setNetworkClient(networkClient);
    ticketService->setNetworkClient(networkClient);
    
    // 连接网络客户端信号到业务服务槽
    QObject::connect(networkClient, &NetworkClient::loginResponse, 
                    authService, &AuthService::onLoginResponse);
    QObject::connect(networkClient, &NetworkClient::registerResponse, 
                    authService, &AuthService::onRegisterResponse);
    QObject::connect(networkClient, &NetworkClient::logoutResponse, 
                    authService, &AuthService::onLogoutResponse);
    
    QObject::connect(networkClient, &NetworkClient::createTicketResponse, 
                    ticketService, &TicketService::onCreateTicketResponse);
    QObject::connect(networkClient, &NetworkClient::joinTicketResponse, 
                    ticketService, &TicketService::onJoinTicketResponse);
    QObject::connect(networkClient, &NetworkClient::leaveTicketResponse, 
                    ticketService, &TicketService::onLeaveTicketResponse);
    QObject::connect(networkClient, &NetworkClient::getTicketListResponse, 
                    ticketService, &TicketService::onGetTicketListResponse);
    QObject::connect(networkClient, &NetworkClient::updateTicketResponse, 
                    ticketService, &TicketService::onUpdateTicketResponse);
    QObject::connect(networkClient, &NetworkClient::updateStatusResponse, 
                    ticketService, &TicketService::onUpdateStatusResponse);
    QObject::connect(networkClient, &NetworkClient::assignTicketResponse, 
                    ticketService, &TicketService::onAssignTicketResponse);
    QObject::connect(networkClient, &NetworkClient::deleteTicketResponse, 
                    ticketService, &TicketService::onDeleteTicketResponse);
    
    // 从配置文件读取服务器连接信息
    QSettings config(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    QString serverHost = config.value("Server/host", "localhost").toString();
    quint16 serverPort = config.value("Server/port", 8080).toUInt();
    
    // 连接到服务器
    bool connected = networkClient->connectToServer(serverHost, serverPort);
    if (connected) {
        LogManager::getInstance()->info(LogModule::SYSTEM, LogLayer::BUSINESS, 
                                       "Main", "网络客户端连接成功");
        // 启动心跳
        networkClient->startHeartbeat(30000); // 30秒心跳
    } else {
        LogManager::getInstance()->warning(LogModule::SYSTEM, LogLayer::BUSINESS, 
                                          "Main", "网络客户端连接失败");
    }

    LoginDialog lg;
    
    // 设置认证服务到登录对话框
    lg.setAuthService(authService);

    if (lg.exec() == QDialog::Accepted){
        HomeMainWindow *home = new HomeMainWindow(lg.getCurrentUser(), lg.getCurrentUserType());
        // 如果需要把登录信息带给主页：

        QSettings st("YourOrg","YourApp");
        const QString t = st.value("theme","light").toString();
        applyTheme(t == "dark" ? Theme::Dark : Theme::Light);

        // 将网络客户端和业务服务传递给主窗口
        home->setTicketService(ticketService);
        home->setAuthService(authService);

        home->show();
        return a.exec();
    } else {
        // 用户取消/失败
        return 0;
    }
}

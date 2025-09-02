#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <QObject>
#include <QJsonObject>
#include <QByteArray>
#include "../../../Logger/log_manager.h"

// 前向声明
class NetworkClient;
class UserMessageHandler;
class WorkOrderMessageHandler;
class OtherMessageHandler;

// 主消息处理器 - 负责消息路由和分发到各个子模块
class MessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit MessageHandler(QObject *parent = nullptr);
    ~MessageHandler();

    // 设置网络客户端引用
    void setNetworkClient(NetworkClient* client);
    
    // 消息处理入口
    void handleMessage(quint16 type, const QJsonObject& data, const QByteArray& binary);

private:
    // 初始化子模块
    void initializeSubHandlers();
    
    // 消息路由分发
    void routeUserMessage(quint16 type, const QJsonObject& data);
    void routeWorkOrderMessage(quint16 type, const QJsonObject& data);
    void routeOtherMessage(quint16 type, const QJsonObject& data, const QByteArray& binary);
    
    // 辅助方法
    void logMessageHandling(quint16 type, const QString& action);

private:
    NetworkClient* networkClient_;
    
    // 子模块处理器
    UserMessageHandler* userHandler_;
    WorkOrderMessageHandler* workOrderHandler_;
    OtherMessageHandler* otherHandler_;
};

#endif // MESSAGE_HANDLER_H

#ifndef WORKORDER_MESSAGE_HANDLER_H
#define WORKORDER_MESSAGE_HANDLER_H

#include <QObject>
#include <QJsonObject>
#include "../../../Logger/log_manager.h"

// 前向声明
class NetworkClient;

// 工单管理消息处理器 - 负责处理工单相关的消息
class WorkOrderMessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit WorkOrderMessageHandler(QObject *parent = nullptr);
    ~WorkOrderMessageHandler();

    // 设置网络客户端引用
    void setNetworkClient(NetworkClient* client);
    
    // 工单消息处理
    void handleCreateWorkOrderResponse(const QJsonObject& data);
    void handleJoinWorkOrderResponse(const QJsonObject& data);
    void handleLeaveWorkOrderResponse(const QJsonObject& data);
    void handleUpdateWorkOrderResponse(const QJsonObject& data);
    void handleListWorkOrdersResponse(const QJsonObject& data);

private:
    // 辅助方法
    bool validateMessageData(const QJsonObject& data, const QStringList& requiredFields);
    QString extractErrorMessage(const QJsonObject& data);

private:
    NetworkClient* networkClient_;
};

#endif // WORKORDER_MESSAGE_HANDLER_H

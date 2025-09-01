#ifndef WORKORDER_HANDLER_H
#define WORKORDER_HANDLER_H

#include "../protocol_handler.h"
#include "../../../business/services/workorder_service.h"

// 工单协议处理器 - 处理工单相关的消息
class WorkOrderHandler : public ProtocolHandler
{
    Q_OBJECT
public:
    explicit WorkOrderHandler(WorkOrderService* workOrderService, QObject *parent = nullptr);
    ~WorkOrderHandler();

    // 实现基类的消息处理方法
    void handleMessage(QTcpSocket* socket, const Packet& packet) override;

private:
    WorkOrderService* workOrderService_;
    
    // 处理具体的工单消息
    void handleCreateWorkOrder(QTcpSocket* socket, const QJsonObject& data);
    void handleJoinWorkOrder(QTcpSocket* socket, const QJsonObject& data);
    void handleLeaveWorkOrder(QTcpSocket* socket, const QJsonObject& data);
    void handleUpdateWorkOrder(QTcpSocket* socket, const QJsonObject& data);
    void handleListWorkOrders(QTcpSocket* socket, const QJsonObject& data);
    void handleGetWorkOrderInfo(QTcpSocket* socket, const QJsonObject& data);
    void handleUpdateWorkOrderStatus(QTcpSocket* socket, const QJsonObject& data);
    void handleAssignWorkOrder(QTcpSocket* socket, const QJsonObject& data);
    void handleCloseWorkOrder(QTcpSocket* socket, const QJsonObject& data);
    void handleGetWorkOrderList(QTcpSocket* socket, const QJsonObject& data);
    
    // 辅助方法
    int getUserIdFromContext(QTcpSocket* socket);
};

#endif // WORKORDER_HANDLER_H

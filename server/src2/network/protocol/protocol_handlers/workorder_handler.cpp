#include "workorder_handler.h"
#include "../connection_manager.h"
#include "../logging/network_logger.h"
#include "../../../common/protocol.h"

WorkOrderHandler::WorkOrderHandler(WorkOrderService* workOrderService, QObject *parent)
    : ProtocolHandler(parent)
    , workOrderService_(workOrderService)
{
}

WorkOrderHandler::~WorkOrderHandler()
{
}

void WorkOrderHandler::handleMessage(QTcpSocket* socket, const Packet& packet)
{
    // 检查认证状态
    if (!checkAuthentication(socket)) {
        return;
    }
    
    switch (packet.type) {
        case MSG_CREATE_WORKORDER:
            handleCreateWorkOrder(socket, packet.json);
            break;
        case MSG_JOIN_WORKORDER:
            handleJoinWorkOrder(socket, packet.json);
            break;
        default:
            sendErrorResponse(socket, 404, QString("Unknown work order message type: %1").arg(packet.type));
            break;
    }
}

void WorkOrderHandler::handleCreateWorkOrder(QTcpSocket* socket, const QJsonObject& data)
{
    QString title = data.value("title").toString();
    QString description = data.value("description").toString();
    QString priority = data.value("priority").toString();
    QString category = data.value("category").toString();
    
    // 基本验证
    if (title.isEmpty() || description.isEmpty()) {
        sendErrorResponse(socket, 400, "Title and description cannot be empty");
        return;
    }
    
    int creatorId = getUserIdFromContext(socket);
    if (creatorId <= 0) {
        sendErrorResponse(socket, 400, "Invalid user context");
        return;
    }
    
    // 调用业务服务创建工单
    QString generatedTicketId;
    bool success = workOrderService_->createWorkOrder(title, description, creatorId, priority, category, generatedTicketId);
    
    if (success) {
        QJsonObject responseData{
            {"ticket_id", generatedTicketId},
            {"title", title},
            {"priority", priority},
            {"category", category}
        };
        sendSuccessResponse(socket, "Work order created successfully", responseData);
        
        NetworkLogger::info("Work Order Handler", 
                           QString("Work order '%1' created successfully by user %2")
                           .arg(generatedTicketId).arg(creatorId));
    } else {
        sendErrorResponse(socket, 500, "Failed to create work order");
        
        NetworkLogger::error("Work Order Handler", 
                            QString("Failed to create work order for user %1")
                            .arg(creatorId));
    }
}

void WorkOrderHandler::handleJoinWorkOrder(QTcpSocket* socket, const QJsonObject& data)
{
    QString roomId = data.value("roomId").toString();
    
    // 基本验证
    if (roomId.isEmpty()) {
        sendErrorResponse(socket, 400, "Room ID cannot be empty");
        return;
    }
    
    // 检查工单是否存在
    WorkOrderModel workOrder = workOrderService_->getWorkOrderByTicketId(roomId);
    if (!workOrder.isValid()) {
        sendErrorResponse(socket, 404, "Work order not found");
        return;
    }
    
    // 加入房间
    if (getConnectionManager()) {
        getConnectionManager()->joinRoom(socket, roomId);
        
        QJsonObject responseData{
            {"roomId", roomId},
            {"work_order_info", workOrder.toJson()}
        };
        sendSuccessResponse(socket, "Joined work order successfully", responseData);
        
        NetworkLogger::info("Work Order Handler", 
                           QString("User joined work order '%1'")
                           .arg(roomId));
    } else {
        sendErrorResponse(socket, 500, "Connection manager not available");
    }
}

void WorkOrderHandler::handleGetWorkOrderInfo(QTcpSocket* socket, const QJsonObject& data)
{
    QString ticketId = data.value("ticket_id").toString();
    int workOrderId = data.value("work_order_id").toInt();
    
    WorkOrderModel workOrder;
    
    if (!ticketId.isEmpty()) {
        workOrder = workOrderService_->getWorkOrderByTicketId(ticketId);
    } else if (workOrderId > 0) {
        workOrder = workOrderService_->getWorkOrderById(workOrderId);
    } else {
        sendErrorResponse(socket, 400, "Either ticket_id or work_order_id must be provided");
        return;
    }
    
    if (!workOrder.isValid()) {
        sendErrorResponse(socket, 404, "Work order not found");
        return;
    }
    
    sendSuccessResponse(socket, "Work order info retrieved", workOrder.toJson());
}

void WorkOrderHandler::handleUpdateWorkOrderStatus(QTcpSocket* socket, const QJsonObject& data)
{
    int workOrderId = data.value("work_order_id").toInt();
    QString newStatus = data.value("status").toString();
    
    if (workOrderId <= 0 || newStatus.isEmpty()) {
        sendErrorResponse(socket, 400, "Work order ID and status are required");
        return;
    }
    
    int userId = getUserIdFromContext(socket);
    bool success = workOrderService_->updateWorkOrderStatus(workOrderId, newStatus, userId);
    
    if (success) {
        sendSuccessResponse(socket, "Work order status updated successfully");
    } else {
        sendErrorResponse(socket, 500, "Failed to update work order status");
    }
}

void WorkOrderHandler::handleAssignWorkOrder(QTcpSocket* socket, const QJsonObject& data)
{
    int workOrderId = data.value("work_order_id").toInt();
    int assigneeId = data.value("assignee_id").toInt();
    
    if (workOrderId <= 0 || assigneeId <= 0) {
        sendErrorResponse(socket, 400, "Work order ID and assignee ID are required");
        return;
    }
    
    int assignerId = getUserIdFromContext(socket);
    bool success = workOrderService_->assignWorkOrder(workOrderId, assigneeId, assignerId);
    
    if (success) {
        sendSuccessResponse(socket, "Work order assigned successfully");
    } else {
        sendErrorResponse(socket, 500, "Failed to assign work order");
    }
}

void WorkOrderHandler::handleCloseWorkOrder(QTcpSocket* socket, const QJsonObject& data)
{
    int workOrderId = data.value("work_order_id").toInt();
    
    if (workOrderId <= 0) {
        sendErrorResponse(socket, 400, "Work order ID is required");
        return;
    }
    
    int userId = getUserIdFromContext(socket);
    bool success = workOrderService_->closeWorkOrder(workOrderId, userId);
    
    if (success) {
        sendSuccessResponse(socket, "Work order closed successfully");
    } else {
        sendErrorResponse(socket, 500, "Failed to close work order");
    }
}

void WorkOrderHandler::handleGetWorkOrderList(QTcpSocket* socket, const QJsonObject& data)
{
    QString status = data.value("status").toString();
    int limit = data.value("limit").toInt(50);
    int offset = data.value("offset").toInt(0);
    
    QList<WorkOrderModel> workOrders;
    
    if (!status.isEmpty()) {
        workOrders = workOrderService_->getWorkOrdersByStatus(status, limit, offset);
    } else {
        workOrders = workOrderService_->getAllWorkOrders(limit, offset);
    }
    
    QJsonArray workOrderArray;
    for (const WorkOrderModel& workOrder : workOrders) {
        workOrderArray.append(workOrder.toJson());
    }
    
    QJsonObject responseData{
        {"work_orders", workOrderArray},
        {"total_count", workOrderArray.size()}
    };
    
    sendSuccessResponse(socket, "Work order list retrieved", responseData);
}

int WorkOrderHandler::getUserIdFromContext(QTcpSocket* socket)
{
    // 这里需要根据用户名获取用户ID
    // 由于简化设计，暂时返回一个固定值或从其他地方获取
    // 在实际实现中，应该从数据库或缓存中获取用户ID
    
    ClientContext* context = getClientContext(socket);
    if (context && !context->username.isEmpty()) {
        // 这里应该调用UserService获取用户ID
        // 暂时返回0表示需要实现
        return 0;
    }
    
    return -1;
}

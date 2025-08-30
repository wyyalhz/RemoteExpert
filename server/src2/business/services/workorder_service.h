#ifndef WORKORDER_SERVICE_H
#define WORKORDER_SERVICE_H

#include "../exceptions/business_exception.h"
#include "../validators/workorder_validator.h"
#include "../managers/workorder_status_manager.h"
#include "../logging/business_logger.h"
#include "../../data/databasemanager.h"
#include "../../data/models/workorder_model.h"
#include "../../data/repositories/workorder_repository.h"
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QRandomGenerator>

// 工单业务服务
class WorkOrderService : public QObject
{
    Q_OBJECT
public:
    explicit WorkOrderService(DatabaseManager* dbManager, QObject *parent = nullptr);
    ~WorkOrderService();

    // 工单创建和管理
    bool createWorkOrder(const QString& title, const QString& description, 
                        int creatorId, const QString& priority, const QString& category, QString& generatedTicketId);
    bool updateWorkOrder(const WorkOrderModel& workOrder);
    bool deleteWorkOrder(int workOrderId, int userId);
    
    // 工单查询
    WorkOrderModel getWorkOrderById(int workOrderId);
    WorkOrderModel getWorkOrderByTicketId(const QString& ticketId);
    QList<WorkOrderModel> getWorkOrdersByStatus(const QString& status, int limit = -1, int offset = 0);
    QList<WorkOrderModel> getWorkOrdersByCreator(int creatorId, int limit = -1, int offset = 0);
    QList<WorkOrderModel> getWorkOrdersByAssignee(int assigneeId, int limit = -1, int offset = 0);
    QList<WorkOrderModel> getAllWorkOrders(int limit = -1, int offset = 0);
    
    // 工单状态管理
    bool updateWorkOrderStatus(int workOrderId, const QString& newStatus, int userId);
    bool closeWorkOrder(int workOrderId, int userId);
    bool startProcessing(int workOrderId, int userId);
    bool refuseWorkOrder(int workOrderId, int userId, const QString& reason = QString());
    
    // 工单分配
    bool assignWorkOrder(int workOrderId, int assigneeId, int assignerId);
    bool unassignWorkOrder(int workOrderId, int assignerId);
    
    // 工单参与者管理
    bool addParticipant(int workOrderId, int userId, const QString& role, const QString& permissions = "");
    bool removeParticipant(int workOrderId, int userId);
    bool updateParticipantRole(int workOrderId, int userId, const QString& newRole);
    QList<ParticipantModel> getWorkOrderParticipants(int workOrderId);
    bool isParticipant(int workOrderId, int userId);
    
    // 工单统计
    int getWorkOrderCount(const QString& status = QString());
    int getWorkOrderCountByCreator(int creatorId);
    int getWorkOrderCountByAssignee(int assigneeId);
    
    // 权限检查
    bool canModifyWorkOrder(int workOrderId, int userId);
    bool canCloseWorkOrder(int workOrderId, int userId);
    bool canAssignWorkOrder(int workOrderId, int userId);
    
    // 状态查询
    QStringList getNextPossibleStatuses(int workOrderId);
    bool canTransitionTo(int workOrderId, const QString& targetStatus);

private:
    DatabaseManager* dbManager_;
    WorkOrderRepository* workOrderRepo_;
    
    // 私有辅助方法
    bool validateWorkOrderExists(int workOrderId);
    bool validateUserPermissions(int workOrderId, int userId, const QString& operation);
    void logWorkOrderActivity(const QString& operation, int workOrderId, bool success, const QString& details = QString());
    QString generateTicketId();
    
    // 业务验证方法
    bool validateWorkOrderCreation(const QString& title, const QString& description, int creatorId);
    bool validateStatusTransition(int workOrderId, const QString& newStatus, int userId);
    bool validateAssignment(int workOrderId, int assigneeId, int assignerId);
    bool validateParticipantAddition(int workOrderId, int userId, const QString& role);
    
    // 权限检查方法
    bool checkUserPermission(int userId, const QString& operation, int workOrderId);
    bool checkWorkOrderAccess(int userId, int workOrderId);
    
    // 业务事件触发方法
    void triggerWorkOrderCreatedEvent(const WorkOrderModel& workOrder);
    void triggerWorkOrderStatusChangedEvent(const WorkOrderModel& workOrder, const QString& oldStatus);
    void triggerWorkOrderAssignedEvent(const WorkOrderModel& workOrder, int assigneeId);
    void triggerWorkOrderClosedEvent(const WorkOrderModel& workOrder);
};

#endif // WORKORDER_SERVICE_H

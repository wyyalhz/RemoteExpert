#ifndef WORKORDER_REPOSITORY_H
#define WORKORDER_REPOSITORY_H

#include "../base/db_base.h"
#include "../models/workorder_model.h"
#include <QList>

class WorkOrderRepository : public DBBase
{
    Q_OBJECT
public:
    explicit WorkOrderRepository(QObject *parent = nullptr);
    ~WorkOrderRepository();

    // 基础CRUD操作
    bool create(const WorkOrderModel& workOrder, int& workOrderId);
    bool findById(int workOrderId, WorkOrderModel& workOrder);
    bool findByTicketId(const QString& ticketId, WorkOrderModel& workOrder);
    bool update(const WorkOrderModel& workOrder);
    bool remove(int workOrderId);
    
    // 查询操作
    QList<WorkOrderModel> findByStatus(const QString& status);
    QList<WorkOrderModel> findByCreator(int creatorId);
    QList<WorkOrderModel> findByAssignee(int assigneeId);
    QList<WorkOrderModel> findByPriority(const QString& priority);
    QList<WorkOrderModel> findByCategory(const QString& category);
    QList<WorkOrderModel> findAll(int limit = -1, int offset = 0);
    
    // 数据字段更新操作
    bool updateField(int workOrderId, const QString& field, const QVariant& value);
    bool updateStatus(int workOrderId, const QString& status);
    bool updateAssignee(int workOrderId, int assigneeId);
    bool updateClosedAt(int workOrderId, const QDateTime& closedAt);
    
    // 参与者数据操作
    bool addParticipant(int workOrderId, int userId, const QString& role, const QString& permissions = "");
    bool removeParticipant(int workOrderId, int userId);
    bool updateParticipantRole(int workOrderId, int userId, const QString& role);
    QList<ParticipantModel> getParticipants(int workOrderId);
    bool isParticipant(int workOrderId, int userId);
    
    // 统计查询
    int countByStatus(const QString& status);
    int countByCreator(int creatorId);
    int countByAssignee(int assigneeId);
    int countAll();

private:
    // 私有辅助方法
    WorkOrderModel mapToModel(const QSqlRecord& record);
    ParticipantModel mapToParticipantModel(const QSqlRecord& record);
    bool executeWorkOrderQuery(QSqlQuery& query, const QString& operation);
    bool executeParticipantQuery(QSqlQuery& query, const QString& operation);
};

#endif // WORKORDER_REPOSITORY_H

#ifndef WORKORDER_MANAGER_H
#define WORKORDER_MANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>

class WorkOrderManager : public QObject
{
    Q_OBJECT
public:
    explicit WorkOrderManager(QObject *parent = nullptr);
    ~WorkOrderManager();

    // 设置数据库连接
    void setDatabase(QSqlDatabase &db) { db_ = db; }
    
    // 工单创建
    bool createWorkOrder(const QString &title, const QString &description,
                        int creatorId, const QString &priority, const QString &category, QString &generatedTicketId);
    
    // 工单参与管理
    bool joinWorkOrder(int workOrderId, int userId, const QString &role);
    bool leaveWorkOrder(int workOrderId, int userId);
    QJsonArray getWorkOrderParticipants(int workOrderId);
    
    // 工单状态管理
    bool updateWorkOrderStatus(int workOrderId, const QString &status);
    bool closeWorkOrder(int workOrderId, int userId);
    
    // 工单指派
    bool assignWorkOrder(int workOrderId, int expertId);
    
    // 工单信息查询
    QJsonObject getWorkOrderInfo(int workOrderId);

private:
    QSqlDatabase db_;
};

#endif // WORKORDER_MANAGER_H

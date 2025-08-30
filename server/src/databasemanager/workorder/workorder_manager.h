#ifndef WORKORDER_MANAGER_H
#define WORKORDER_MANAGER_H

#include "../base/db_base.h"

class WorkOrderManager : public DBBase
{
    Q_OBJECT
public:
    // 工单状态管理嵌套类
    class WorkOrderStatus {
    public:
        // 状态定义
        static const QString OPEN;
        static const QString PROCESSING;
        static const QString REFUSED;
        static const QString CLOSED;
        
        // 状态等级映射
        static int getStatusLevel(const QString &status);
        
        // 状态转换合法性验证
        static bool isValidTransition(const QString &fromStatus, const QString &toStatus);
        
        // 获取所有有效状态
        static QStringList getValidStatuses();
        
        // 验证状态是否有效
        static bool isValidStatus(const QString &status);
    };

    explicit WorkOrderManager(QObject *parent = nullptr);
    ~WorkOrderManager();

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
};

#endif // WORKORDER_MANAGER_H

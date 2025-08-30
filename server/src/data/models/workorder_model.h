#ifndef WORKORDER_MODEL_H
#define WORKORDER_MODEL_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

struct WorkOrderModel {
    int id = -1;
    QString ticketId;
    QString title;
    QString description;
    int creatorId = -1;
    QString status;
    QString priority;
    QString category;
    int assignedTo = -1;
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime closedAt;
    
    // 辅助方法
    bool isValid() const;
    QJsonObject toJson() const;
    static WorkOrderModel fromJson(const QJsonObject& json);
    
    // 状态常量
    static const QString STATUS_OPEN;
    static const QString STATUS_PROCESSING;
    static const QString STATUS_REFUSED;
    static const QString STATUS_CLOSED;
    
    // 优先级常量
    static const QString PRIORITY_LOW;
    static const QString PRIORITY_NORMAL;
    static const QString PRIORITY_HIGH;
    static const QString PRIORITY_URGENT;
};

struct ParticipantModel {
    int id = -1;
    int workOrderId = -1;
    int userId = -1;
    QString role;
    QDateTime joinedAt;
    QDateTime leftAt;
    QString permissions;
    
    // 辅助方法
    bool isValid() const;
    QJsonObject toJson() const;
    static ParticipantModel fromJson(const QJsonObject& json);
    
    // 角色常量
    static const QString ROLE_CREATOR;
    static const QString ROLE_EXPERT;
    static const QString ROLE_OPERATOR;
    static const QString ROLE_VIEWER;
};

#endif // WORKORDER_MODEL_H

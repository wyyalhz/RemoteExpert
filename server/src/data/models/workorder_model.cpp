#include "workorder_model.h"

// 静态常量定义
const QString WorkOrderModel::STATUS_OPEN = "open";
const QString WorkOrderModel::STATUS_PROCESSING = "processing";
const QString WorkOrderModel::STATUS_REFUSED = "refused";
const QString WorkOrderModel::STATUS_CLOSED = "closed";

const QString WorkOrderModel::PRIORITY_LOW = "low";
const QString WorkOrderModel::PRIORITY_NORMAL = "normal";
const QString WorkOrderModel::PRIORITY_HIGH = "high";
const QString WorkOrderModel::PRIORITY_URGENT = "urgent";

const QString ParticipantModel::ROLE_CREATOR = "creator";
const QString ParticipantModel::ROLE_EXPERT = "expert";
const QString ParticipantModel::ROLE_OPERATOR = "operator";
const QString ParticipantModel::ROLE_VIEWER = "viewer";

// WorkOrderModel 实现
bool WorkOrderModel::isValid() const
{
    return !ticketId.isEmpty() && !title.isEmpty() && creatorId > 0;
}

QJsonObject WorkOrderModel::toJson() const
{
    QJsonObject json;
    json["id"] = id;
    json["ticket_id"] = ticketId;
    json["title"] = title;
    json["description"] = description;
    json["creator_id"] = creatorId;
    json["status"] = status;
    json["priority"] = priority;
    json["category"] = category;
    json["assigned_to"] = assignedTo;
    json["created_at"] = createdAt.toString(Qt::ISODate);
    json["updated_at"] = updatedAt.toString(Qt::ISODate);
    if (closedAt.isValid()) {
        json["closed_at"] = closedAt.toString(Qt::ISODate);
    }
    return json;
}

WorkOrderModel WorkOrderModel::fromJson(const QJsonObject& json)
{
    WorkOrderModel model;
    model.id = json["id"].toInt(-1);
    model.ticketId = json["ticket_id"].toString();
    model.title = json["title"].toString();
    model.description = json["description"].toString();
    model.creatorId = json["creator_id"].toInt(-1);
    model.status = json["status"].toString();
    model.priority = json["priority"].toString();
    model.category = json["category"].toString();
    model.assignedTo = json["assigned_to"].toInt(-1);
    model.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODate);
    model.updatedAt = QDateTime::fromString(json["updated_at"].toString(), Qt::ISODate);
    if (json.contains("closed_at") && !json["closed_at"].isNull()) {
        model.closedAt = QDateTime::fromString(json["closed_at"].toString(), Qt::ISODate);
    }
    return model;
}

// ParticipantModel 实现
bool ParticipantModel::isValid() const
{
    return workOrderId > 0 && userId > 0 && !role.isEmpty();
}

QJsonObject ParticipantModel::toJson() const
{
    QJsonObject json;
    json["id"] = id;
    json["work_order_id"] = workOrderId;
    json["user_id"] = userId;
    json["role"] = role;
    json["joined_at"] = joinedAt.toString(Qt::ISODate);
    if (leftAt.isValid()) {
        json["left_at"] = leftAt.toString(Qt::ISODate);
    }
    json["permissions"] = permissions;
    return json;
}

ParticipantModel ParticipantModel::fromJson(const QJsonObject& json)
{
    ParticipantModel model;
    model.id = json["id"].toInt(-1);
    model.workOrderId = json["work_order_id"].toInt(-1);
    model.userId = json["user_id"].toInt(-1);
    model.role = json["role"].toString();
    model.joinedAt = QDateTime::fromString(json["joined_at"].toString(), Qt::ISODate);
    if (json.contains("left_at") && !json["left_at"].isNull()) {
        model.leftAt = QDateTime::fromString(json["left_at"].toString(), Qt::ISODate);
    }
    model.permissions = json["permissions"].toString();
    return model;
}

#include "session_model.h"

// 静态常量定义
const QString SessionModel::STATUS_ACTIVE = "active";
const QString SessionModel::STATUS_INACTIVE = "inactive";
const QString SessionModel::STATUS_EXPIRED = "expired";

bool SessionModel::isValid() const
{
    return !sessionId.isEmpty() && userId > 0;
}

QJsonObject SessionModel::toJson() const
{
    QJsonObject json;
    json["id"] = id;
    json["session_id"] = sessionId;
    json["user_id"] = userId;
    json["room_id"] = roomId;
    json["status"] = status;
    json["created_at"] = createdAt.toString(Qt::ISODate);
    json["last_activity"] = lastActivity.toString(Qt::ISODate);
    if (expiresAt.isValid()) {
        json["expires_at"] = expiresAt.toString(Qt::ISODate);
    }
    return json;
}

SessionModel SessionModel::fromJson(const QJsonObject& json)
{
    SessionModel model;
    model.id = json["id"].toInt(-1);
    model.sessionId = json["session_id"].toString();
    model.userId = json["user_id"].toInt(-1);
    model.roomId = json["room_id"].toString();
    model.status = json["status"].toString();
    model.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODate);
    model.lastActivity = QDateTime::fromString(json["last_activity"].toString(), Qt::ISODate);
    if (json.contains("expires_at") && !json["expires_at"].isNull()) {
        model.expiresAt = QDateTime::fromString(json["expires_at"].toString(), Qt::ISODate);
    }
    return model;
}

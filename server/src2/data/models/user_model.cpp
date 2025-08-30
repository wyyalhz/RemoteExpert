#include "user_model.h"

// 静态常量定义
const int UserModel::TYPE_FACTORY = FACTORY_USER;
const int UserModel::TYPE_EXPERT = EXPERT_USER;

bool UserModel::isValid() const
{
    return !username.isEmpty() && !passwordHash.isEmpty() && userType >= 0;
}

QJsonObject UserModel::toJson() const
{
    QJsonObject json;
    json["id"] = id;
    json["username"] = username;
    json["password_hash"] = passwordHash;
    json["email"] = email;
    json["phone"] = phone;
    json["user_type"] = userType;
    json["created_at"] = createdAt.toString(Qt::ISODate);
    return json;
}

UserModel UserModel::fromJson(const QJsonObject& json)
{
    UserModel model;
    model.id = json["id"].toInt(-1);
    model.username = json["username"].toString();
    model.passwordHash = json["password_hash"].toString();
    model.email = json["email"].toString();
    model.phone = json["phone"].toString();
    model.userType = json["user_type"].toInt(FACTORY_USER);
    model.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODate);
    return model;
}

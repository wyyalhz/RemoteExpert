#include "User.h"
#include <QJsonArray>

User::User()
    : id_(-1)
    , userType_(0)
    , isOnline_(false)
{
}

User::User(const QString& username, int userType)
    : id_(-1)
    , username_(username)
    , userType_(userType)
    , isOnline_(false)
{
}

User::User(const QJsonObject& json)
    : id_(-1)
    , userType_(0)
    , isOnline_(false)
{
    fromJson(json);
}

QJsonObject User::toJson() const
{
    QJsonObject json;
    json["id"] = id_;
    json["username"] = username_;
    json["email"] = email_;
    json["phone"] = phone_;
    json["userType"] = userType_;
    json["createdTime"] = createdTime_.toString(Qt::ISODate);
    json["lastLoginTime"] = lastLoginTime_.toString(Qt::ISODate);
    json["isOnline"] = isOnline_;
    json["sessionId"] = sessionId_;
    return json;
}

User User::fromJson(const QJsonObject& json)
{
    User user;
    
    if (json.contains("id")) {
        user.id_ = json["id"].toInt();
    }
    
    if (json.contains("username")) {
        user.username_ = json["username"].toString();
    }
    
    if (json.contains("email")) {
        user.email_ = json["email"].toString();
    }
    
    if (json.contains("phone")) {
        user.phone_ = json["phone"].toString();
    }
    
    if (json.contains("userType")) {
        user.userType_ = json["userType"].toInt();
    }
    
    if (json.contains("createdTime")) {
        user.createdTime_ = QDateTime::fromString(json["createdTime"].toString(), Qt::ISODate);
    }
    
    if (json.contains("lastLoginTime")) {
        user.lastLoginTime_ = QDateTime::fromString(json["lastLoginTime"].toString(), Qt::ISODate);
    }
    
    if (json.contains("isOnline")) {
        user.isOnline_ = json["isOnline"].toBool();
    }
    
    if (json.contains("sessionId")) {
        user.sessionId_ = json["sessionId"].toString();
    }
    
    return user;
}

bool User::isValid() const
{
    return !username_.isEmpty() && username_.length() >= 3;
}

QString User::getValidationError() const
{
    if (username_.isEmpty()) {
        return "用户名不能为空";
    }
    
    if (username_.length() < 3) {
        return "用户名长度至少3个字符";
    }
    
    return QString();
}

QString User::getDisplayName() const
{
    if (isExpert()) {
        return QString("%1 (专家)").arg(username_);
    } else {
        return QString("%1 (工厂用户)").arg(username_);
    }
}

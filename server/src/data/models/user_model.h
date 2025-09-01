#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include "../../../../common/protocol/types/enums.h"

// 使用common/protocol中的UserType枚举定义
// 为了向后兼容，提供常量别名
const int FACTORY_USER = USER_TYPE_NORMAL;
const int EXPERT_USER = USER_TYPE_EXPERT;

struct UserModel {
    int id = -1;
    QString username;
    QString passwordHash;
    QString email;
    QString phone;
    int userType = USER_TYPE_NORMAL;  // 使用common/protocol中的枚举值
    QDateTime createdAt;
    
    // 辅助方法
    bool isValid() const;
    QJsonObject toJson() const;
    static UserModel fromJson(const QJsonObject& json);
    
    // 用户类型常量
    static const int TYPE_FACTORY;
    static const int TYPE_EXPERT;
};

#endif // USER_MODEL_H

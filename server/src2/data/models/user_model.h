#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

// 用户类型枚举
enum UserType {
    FACTORY_USER = 0,    // 工厂端用户
    EXPERT_USER = 1      // 专家端用户
};

struct UserModel {
    int id = -1;
    QString username;
    QString passwordHash;
    QString email;
    QString phone;
    int userType = FACTORY_USER;  // 替换原来的role字段
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

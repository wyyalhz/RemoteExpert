#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>

// 用户业务模型
class User
{
public:
    User();
    User(const QString& username, int userType);
    User(const QJsonObject& json);
    
    // 基本属性
    int getId() const { return id_; }
    void setId(int id) { id_ = id; }
    
    QString getUsername() const { return username_; }
    void setUsername(const QString& username) { username_ = username; }
    
    QString getEmail() const { return email_; }
    void setEmail(const QString& email) { email_ = email; }
    
    QString getPhone() const { return phone_; }
    void setPhone(const QString& phone) { phone_ = phone; }
    
    int getUserType() const { return userType_; }
    void setUserType(int userType) { userType_ = userType; }
    
    QDateTime getCreatedTime() const { return createdTime_; }
    void setCreatedTime(const QDateTime& time) { createdTime_ = time; }
    
    QDateTime getLastLoginTime() const { return lastLoginTime_; }
    void setLastLoginTime(const QDateTime& time) { lastLoginTime_ = time; }
    
    bool isOnline() const { return isOnline_; }
    void setOnline(bool online) { isOnline_ = online; }
    
    QString getSessionId() const { return sessionId_; }
    void setSessionId(const QString& sessionId) { sessionId_ = sessionId; }
    
    // 序列化方法
    QJsonObject toJson() const;
    static User fromJson(const QJsonObject& json);
    
    // 验证方法
    bool isValid() const;
    QString getValidationError() const;
    
    // 用户类型判断
    bool isExpert() const { return userType_ == 1; }
    bool isFactoryUser() const { return userType_ == 0; }
    
    // 显示名称
    QString getDisplayName() const;
    
private:
    int id_;
    QString username_;
    QString email_;
    QString phone_;
    int userType_;  // 0: 工厂用户, 1: 技术专家
    QDateTime createdTime_;
    QDateTime lastLoginTime_;
    bool isOnline_;
    QString sessionId_;
};

#endif // USER_H

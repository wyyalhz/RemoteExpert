#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include "../models/User.h"
#include "../../Logger/log_manager.h"
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QTimer>

// 会话管理器 - 负责管理用户会话状态
class SessionManager : public QObject
{
    Q_OBJECT

public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager();

    // 会话管理
    bool createSession(const User& user, const QString& sessionId, int timeoutMinutes = 120);
    bool updateSessionActivity();
    bool expireSession();
    bool isSessionValid() const;
    
    // 会话信息
    User getCurrentUser() const { return currentUser_; }
    QString getSessionId() const { return sessionId_; }
    QDateTime getSessionStartTime() const { return sessionStartTime_; }
    QDateTime getLastActivityTime() const { return lastActivityTime_; }
    int getSessionTimeout() const { return sessionTimeoutMinutes_; }
    
    // 会话状态
    bool isLoggedIn() const { return isLoggedIn_; }
    bool isSessionExpired() const;
    int getRemainingTime() const; // 返回剩余时间（分钟）
    
    // 会话配置
    void setSessionTimeout(int minutes) { sessionTimeoutMinutes_ = minutes; }
    void setAutoRefresh(bool enabled) { autoRefresh_ = enabled; }
    void setRefreshInterval(int seconds) { refreshInterval_ = seconds; }
    
    // 清理
    void clearSession();

signals:
    // 会话状态变化信号
    void sessionCreated(const User& user, const QString& sessionId);
    void sessionExpired();
    void sessionRefreshed();
    void sessionActivityUpdated();
    void sessionCleared();

private slots:
    // 定时器槽
    void onSessionTimeout();
    void onAutoRefresh();

private:
    // 私有辅助方法
    void startSessionTimer();
    void stopSessionTimer();
    void startAutoRefreshTimer();
    void stopAutoRefreshTimer();
    void updateLastActivity();

private:
    User currentUser_;
    QString sessionId_;
    QDateTime sessionStartTime_;
    QDateTime lastActivityTime_;
    int sessionTimeoutMinutes_;
    bool isLoggedIn_;
    bool autoRefresh_;
    int refreshInterval_;
    
    QTimer* sessionTimer_;
    QTimer* autoRefreshTimer_;
};

#endif // SESSION_MANAGER_H

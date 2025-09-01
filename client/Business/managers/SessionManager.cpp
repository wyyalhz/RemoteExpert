#include "SessionManager.h"

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , sessionTimeoutMinutes_(120)
    , isLoggedIn_(false)
    , autoRefresh_(true)
    , refreshInterval_(60) // 60秒自动刷新
    , sessionTimer_(nullptr)
    , autoRefreshTimer_(nullptr)
{
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "SessionManager", "会话管理器初始化完成");
}

SessionManager::~SessionManager()
{
    stopSessionTimer();
    stopAutoRefreshTimer();
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "SessionManager", "会话管理器销毁");
}

bool SessionManager::createSession(const User& user, const QString& sessionId, int timeoutMinutes)
{
    if (user.getUsername().isEmpty()) {
        LogManager::getInstance()->warning(LogModule::USER, LogLayer::BUSINESS, 
                                          "SessionManager", "创建会话失败: 用户信息无效");
        return false;
    }
    
    if (sessionId.isEmpty()) {
        LogManager::getInstance()->warning(LogModule::USER, LogLayer::BUSINESS, 
                                          "SessionManager", "创建会话失败: 会话ID无效");
        return false;
    }
    
    currentUser_ = user;
    sessionId_ = sessionId;
    sessionStartTime_ = QDateTime::currentDateTime();
    lastActivityTime_ = QDateTime::currentDateTime();
    sessionTimeoutMinutes_ = timeoutMinutes;
    isLoggedIn_ = true;
    
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "SessionManager", QString("创建会话成功: %1, 超时时间: %2分钟")
                                   .arg(user.getUsername()).arg(timeoutMinutes));
    
    // 启动会话定时器
    startSessionTimer();
    
    // 启动自动刷新定时器
    if (autoRefresh_) {
        startAutoRefreshTimer();
    }
    
    emit sessionCreated(user, sessionId);
    return true;
}

bool SessionManager::updateSessionActivity()
{
    if (!isLoggedIn_) {
        LogManager::getInstance()->warning(LogModule::USER, LogLayer::BUSINESS, 
                                          "SessionManager", "更新会话活动失败: 用户未登录");
        return false;
    }
    
    updateLastActivity();
    
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "SessionManager", "会话活动已更新");
    
    emit sessionActivityUpdated();
    return true;
}

bool SessionManager::expireSession()
{
    if (!isLoggedIn_) {
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "SessionManager", QString("会话过期: %1").arg(currentUser_.getUsername()));
    
    clearSession();
    emit sessionExpired();
    return true;
}

bool SessionManager::isSessionValid() const
{
    if (!isLoggedIn_) {
        return false;
    }
    
    if (isSessionExpired()) {
        return false;
    }
    
    return true;
}

bool SessionManager::isSessionExpired() const
{
    if (!isLoggedIn_) {
        return true;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    int elapsedMinutes = lastActivityTime_.secsTo(now) / 60;
    
    return elapsedMinutes >= sessionTimeoutMinutes_;
}

int SessionManager::getRemainingTime() const
{
    if (!isLoggedIn_) {
        return 0;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    int elapsedMinutes = lastActivityTime_.secsTo(now) / 60;
    int remainingMinutes = sessionTimeoutMinutes_ - elapsedMinutes;
    
    return qMax(0, remainingMinutes);
}

void SessionManager::clearSession()
{
    stopSessionTimer();
    stopAutoRefreshTimer();
    
    currentUser_ = User();
    sessionId_.clear();
    sessionStartTime_ = QDateTime();
    lastActivityTime_ = QDateTime();
    isLoggedIn_ = false;
    
    LogManager::getInstance()->info(LogModule::USER, LogLayer::BUSINESS, 
                                   "SessionManager", "会话已清除");
    
    emit sessionCleared();
}

void SessionManager::onSessionTimeout()
{
    LogManager::getInstance()->warning(LogModule::USER, LogLayer::BUSINESS, 
                                      "SessionManager", "会话超时");
    
    expireSession();
}

void SessionManager::onAutoRefresh()
{
    if (!isLoggedIn_) {
        stopAutoRefreshTimer();
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "SessionManager", "自动刷新会话");
    
    updateSessionActivity();
    emit sessionRefreshed();
}

void SessionManager::startSessionTimer()
{
    stopSessionTimer();
    
    sessionTimer_ = new QTimer(this);
    connect(sessionTimer_, &QTimer::timeout, this, &SessionManager::onSessionTimeout);
    
    // 设置定时器为会话超时时间
    int timeoutMs = sessionTimeoutMinutes_ * 60 * 1000;
    sessionTimer_->setSingleShot(true);
    sessionTimer_->start(timeoutMs);
    
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "SessionManager", QString("会话定时器已启动，超时时间: %1分钟")
                                    .arg(sessionTimeoutMinutes_));
}

void SessionManager::stopSessionTimer()
{
    if (sessionTimer_) {
        sessionTimer_->stop();
        sessionTimer_->deleteLater();
        sessionTimer_ = nullptr;
    }
}

void SessionManager::startAutoRefreshTimer()
{
    stopAutoRefreshTimer();
    
    autoRefreshTimer_ = new QTimer(this);
    connect(autoRefreshTimer_, &QTimer::timeout, this, &SessionManager::onAutoRefresh);
    
    // 设置自动刷新间隔
    autoRefreshTimer_->setInterval(refreshInterval_ * 1000);
    autoRefreshTimer_->start();
    
    LogManager::getInstance()->debug(LogModule::USER, LogLayer::BUSINESS, 
                                    "SessionManager", QString("自动刷新定时器已启动，间隔: %1秒")
                                    .arg(refreshInterval_));
}

void SessionManager::stopAutoRefreshTimer()
{
    if (autoRefreshTimer_) {
        autoRefreshTimer_->stop();
        autoRefreshTimer_->deleteLater();
        autoRefreshTimer_ = nullptr;
    }
}

void SessionManager::updateLastActivity()
{
    lastActivityTime_ = QDateTime::currentDateTime();
    
    // 重置会话定时器
    if (sessionTimer_) {
        sessionTimer_->stop();
        int timeoutMs = sessionTimeoutMinutes_ * 60 * 1000;
        sessionTimer_->start(timeoutMs);
    }
}

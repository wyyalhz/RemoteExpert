#include "user_message_handler.h"
#include "../client/network_client.h"
#include "../../../../../common/logging/managers/log_manager.h"
#include <QJsonDocument>

UserMessageHandler::UserMessageHandler(QObject *parent)
    : QObject(parent)
    , networkClient_(nullptr)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "用户消息处理器初始化完成");
}

UserMessageHandler::~UserMessageHandler()
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "用户消息处理器已销毁");
}

void UserMessageHandler::setNetworkClient(NetworkClient* client)
{
    networkClient_ = client;
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "网络客户端引用已设置");
}

void UserMessageHandler::handleLoginResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "处理登录响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "登录响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "登录成功");
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", QString("登录失败: %1").arg(message));
    }
}

void UserMessageHandler::handleRegisterResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "处理注册响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "注册响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "注册成功");
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", QString("注册失败: %1").arg(message));
    }
}

void UserMessageHandler::handleLogoutResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "处理登出响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "登出响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "登出成功");
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", QString("登出失败: %1").arg(message));
    }
}

void UserMessageHandler::handleHeartbeatResponse(const QJsonObject& data)
{
    // 心跳响应通常不需要特殊处理，只是确认连接正常
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", "收到心跳响应");
}

bool UserMessageHandler::validateMessageData(const QJsonObject& data, const QStringList& requiredFields)
{
    for (const QString& field : requiredFields) {
        if (!data.contains(field)) {
            LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "UserMessageHandler", 
                           QString("缺少必需字段: %1").arg(field));
            return false;
        }
    }
    return true;
}

QString UserMessageHandler::extractErrorMessage(const QJsonObject& data)
{
    if (data.contains("message")) {
        return data["message"].toString();
    }
    if (data.contains("error")) {
        return data["error"].toString();
    }
    return "未知错误";
}

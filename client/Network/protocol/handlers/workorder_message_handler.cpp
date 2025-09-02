#include "workorder_message_handler.h"
#include "../client/network_client.h"
#include "../../../Logger/log_manager.h"
#include <QJsonDocument>

WorkOrderMessageHandler::WorkOrderMessageHandler(QObject *parent)
    : QObject(parent)
    , networkClient_(nullptr)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "工单消息处理器初始化完成");
}

WorkOrderMessageHandler::~WorkOrderMessageHandler()
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "工单消息处理器已销毁");
}

void WorkOrderMessageHandler::setNetworkClient(NetworkClient* client)
{
    networkClient_ = client;
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "网络客户端引用已设置");
}

void WorkOrderMessageHandler::handleCreateWorkOrderResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "处理创建工单响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "创建工单响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        QString ticketId = data["ticketId"].toString();
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", 
                        QString("工单创建成功，ID: %1").arg(ticketId));
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", 
                        QString("工单创建失败: %1").arg(message));
    }
}

void WorkOrderMessageHandler::handleJoinWorkOrderResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "处理加入工单响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "加入工单响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        QString roomId = data["roomId"].toString();
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", 
                        QString("成功加入工单房间: %1").arg(roomId));
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", 
                        QString("加入工单失败: %1").arg(message));
    }
}

void WorkOrderMessageHandler::handleLeaveWorkOrderResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "处理离开工单响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "离开工单响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "成功离开工单");
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", 
                        QString("离开工单失败: %1").arg(message));
    }
}

void WorkOrderMessageHandler::handleUpdateWorkOrderResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "处理更新工单响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "更新工单响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "工单更新成功");
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", 
                        QString("工单更新失败: %1").arg(message));
    }
}

void WorkOrderMessageHandler::handleListWorkOrdersResponse(const QJsonObject& data)
{
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "处理获取工单列表响应");
    
    if (!validateMessageData(data, {"success", "message"})) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", "获取工单列表响应数据格式无效");
        return;
    }
    
    bool success = data["success"].toBool();
    QString message = data["message"].toString();
    
    if (success) {
        QJsonArray workOrders = data["workOrders"].toArray();
        int totalCount = data["totalCount"].toInt();
        LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", 
                        QString("成功获取工单列表，共 %1 个工单").arg(totalCount));
    } else {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", 
                        QString("获取工单列表失败: %1").arg(message));
    }
}

bool WorkOrderMessageHandler::validateMessageData(const QJsonObject& data, const QStringList& requiredFields)
{
    for (const QString& field : requiredFields) {
        if (!data.contains(field)) {
            LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "WorkOrderMessageHandler", 
                           QString("缺少必需字段: %1").arg(field));
            return false;
        }
    }
    return true;
}

QString WorkOrderMessageHandler::extractErrorMessage(const QJsonObject& data)
{
    if (data.contains("message")) {
        return data["message"].toString();
    }
    if (data.contains("error")) {
        return data["error"].toString();
    }
    return "未知错误";
}

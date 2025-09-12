#include "message_handler.h"
#include "user_message_handler.h"
#include "workorder_message_handler.h"
#include "other_message_handler.h"
#include "../client/network_client.h"
#include "../../../logging/log_manager.h"
#include "../../../../common/protocol/protocol.h"
#include <QJsonDocument>

MessageHandler::MessageHandler(QObject *parent)
    : QObject(parent)
    , networkClient_(nullptr)
    , userHandler_(nullptr)
    , workOrderHandler_(nullptr)
    , otherHandler_(nullptr)
{
    initializeSubHandlers();
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "主消息处理器初始化完成");
}

MessageHandler::~MessageHandler()
{
    // 删除子模块处理器
    if (userHandler_) {
        delete userHandler_;
        userHandler_ = nullptr;
    }
    if (workOrderHandler_) {
        delete workOrderHandler_;
        workOrderHandler_ = nullptr;
    }
    if (otherHandler_) {
        delete otherHandler_;
        otherHandler_ = nullptr;
    }
    
    LogManager::getInstance()->info(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "主消息处理器已销毁");
}

void MessageHandler::initializeSubHandlers()
{
    // 创建并初始化子模块处理器
    userHandler_ = new UserMessageHandler(this);
    workOrderHandler_ = new WorkOrderMessageHandler(this);
    otherHandler_ = new OtherMessageHandler(this);
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "子模块处理器初始化完成");
}

void MessageHandler::setNetworkClient(NetworkClient* client)
{
    networkClient_ = client;
    
    // 设置子模块的网络客户端引用
    if (userHandler_) {
        userHandler_->setNetworkClient(client);
    }
    if (workOrderHandler_) {
        workOrderHandler_->setNetworkClient(client);
    }
    if (otherHandler_) {
        otherHandler_->setNetworkClient(client);
    }
    
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "网络客户端引用已设置到所有子模块");
}

void MessageHandler::handleMessage(quint16 type, const QJsonObject& data, const QByteArray& binary)
{
    logMessageHandling(type, "开始处理");
    
    // 根据消息类型路由到相应的子模块
    switch (type) {
        // 用户认证消息
        case MSG_LOGIN:
        case MSG_REGISTER:
        case MSG_LOGOUT:
        case MSG_HEARTBEAT:
            routeUserMessage(type, data);
            break;
            
        // 工单相关消息
        case MSG_CREATE_WORKORDER:
        case MSG_JOIN_WORKORDER:
        case MSG_LEAVE_WORKORDER:
        case MSG_UPDATE_WORKORDER:
        case MSG_LIST_WORKORDERS:
            routeWorkOrderMessage(type, data);
            break;
            
        // 其他消息（聊天、音视频、控制等）
        case MSG_TEXT:
        case MSG_DEVICE_DATA:
        case MSG_FILE_TRANSFER:
        case MSG_SCREENSHOT:
        case MSG_VIDEO_FRAME:
        case MSG_AUDIO_FRAME:
        case MSG_VIDEO_CONTROL:
        case MSG_AUDIO_CONTROL:
        case MSG_CONTROL:
        case MSG_DEVICE_CONTROL:
        case MSG_SYSTEM_CONTROL:
        case MSG_SERVER_EVENT:
        case MSG_ERROR:
        case MSG_NOTIFICATION:
            routeOtherMessage(type, data, binary);
            break;
            
        default:
            LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                                             QString("未知消息类型: %1").arg(type));
            break;
    }
    
    logMessageHandling(type, "处理完成");
}

void MessageHandler::routeUserMessage(quint16 type, const QJsonObject& data)
{
    if (!userHandler_) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "用户消息处理器未初始化");
        return;
    }
    
    switch (type) {
        case MSG_LOGIN:
            userHandler_->handleLoginResponse(data);
            break;
        case MSG_REGISTER:
            userHandler_->handleRegisterResponse(data);
            break;
        case MSG_LOGOUT:
            userHandler_->handleLogoutResponse(data);
            break;
        case MSG_HEARTBEAT:
            userHandler_->handleHeartbeatResponse(data);
            break;
        default:
            LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                                             QString("用户消息处理器不支持的消息类型: %1").arg(type));
            break;
    }
}

void MessageHandler::routeWorkOrderMessage(quint16 type, const QJsonObject& data)
{
    if (!workOrderHandler_) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "工单消息处理器未初始化");
        return;
    }
    
    switch (type) {
        case MSG_CREATE_WORKORDER:
            workOrderHandler_->handleCreateWorkOrderResponse(data);
            break;
        case MSG_JOIN_WORKORDER:
            workOrderHandler_->handleJoinWorkOrderResponse(data);
            break;
        case MSG_LEAVE_WORKORDER:
            workOrderHandler_->handleLeaveWorkOrderResponse(data);
            break;
        case MSG_UPDATE_WORKORDER:
            workOrderHandler_->handleUpdateWorkOrderResponse(data);
            break;
        case MSG_LIST_WORKORDERS:
            workOrderHandler_->handleListWorkOrdersResponse(data);
            break;
        default:
            LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                                             QString("工单消息处理器不支持的消息类型: %1").arg(type));
            break;
    }
}

void MessageHandler::routeOtherMessage(quint16 type, const QJsonObject& data, const QByteArray& binary)
{
    if (!otherHandler_) {
        LogManager::getInstance()->error(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", "其他消息处理器未初始化");
        return;
    }
    
    switch (type) {
        case MSG_TEXT:
            otherHandler_->handleTextMessage(data);
            break;
        case MSG_DEVICE_DATA:
            otherHandler_->handleDeviceDataMessage(data);
            break;
        case MSG_FILE_TRANSFER:
            otherHandler_->handleFileTransferMessage(data);
            break;
        case MSG_SCREENSHOT:
            otherHandler_->handleScreenshotMessage(data);
            break;
        case MSG_VIDEO_FRAME:
            otherHandler_->handleVideoFrameMessage(data, binary);
            break;
        case MSG_AUDIO_FRAME:
            otherHandler_->handleAudioFrameMessage(data, binary);
            break;
        case MSG_VIDEO_CONTROL:
            otherHandler_->handleVideoControlMessage(data);
            break;
        case MSG_AUDIO_CONTROL:
            otherHandler_->handleAudioControlMessage(data);
            break;
        case MSG_CONTROL:
            otherHandler_->handleControlMessage(data);
            break;
        case MSG_DEVICE_CONTROL:
            otherHandler_->handleDeviceControlMessage(data);
            break;
        case MSG_SYSTEM_CONTROL:
            otherHandler_->handleSystemControlMessage(data);
            break;
        case MSG_SERVER_EVENT:
            otherHandler_->handleServerEventMessage(data);
            break;
        case MSG_ERROR:
            otherHandler_->handleErrorMessage(data);
            break;
        case MSG_NOTIFICATION:
            otherHandler_->handleNotificationMessage(data);
            break;
        default:
            LogManager::getInstance()->warning(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                                             QString("其他消息处理器不支持的消息类型: %1").arg(type));
            break;
    }
}

void MessageHandler::logMessageHandling(quint16 type, const QString& action)
{
    LogManager::getInstance()->debug(LogModule::NETWORK, LogLayer::NETWORK, "MessageHandler", 
                    QString("消息处理 [%1]: %2").arg(type).arg(action));
}

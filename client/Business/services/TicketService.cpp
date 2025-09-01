#include "TicketService.h"
#include "../../common/protocol/builders/message_builder.h"
#include "../../Network/client/network_client.h"

TicketService::TicketService(QObject *parent)
    : QObject(parent)
    , networkClient_(nullptr)
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", "工单服务初始化完成");
}

void TicketService::setNetworkClient(NetworkClient* client)
{
    networkClient_ = client;
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", "网络客户端已设置");
}

TicketService::~TicketService()
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", "工单服务销毁");
}

bool TicketService::createTicket(const QString& title, const QString& description, 
                               const QString& priority, const QString& category, 
                               const QJsonObject& deviceInfo)
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("创建工单: %1").arg(title));
    
    // 输入验证
    if (title.isEmpty()) {
        setError("工单标题不能为空");
        LogManager::getInstance()->warning(LogModule::TICKET, LogLayer::BUSINESS, 
                                          "TicketService", "创建工单失败: 标题为空");
        emit ticketCreatedFailed(lastError_);
        return false;
    }
    
    if (description.isEmpty()) {
        setError("工单描述不能为空");
        LogManager::getInstance()->warning(LogModule::TICKET, LogLayer::BUSINESS, 
                                          "TicketService", "创建工单失败: 描述为空");
        emit ticketCreatedFailed(lastError_);
        return false;
    }
    
    // 发送创建工单请求（将在网络层实现后调用）
    sendCreateTicketRequest(title, description, priority, category, deviceInfo);
    
    return false;
}

bool TicketService::updateTicket(const Ticket& ticket)
{
    if (!ticket.isValid()) {
        setError(ticket.getValidationError());
        LogManager::getInstance()->warning(LogModule::TICKET, LogLayer::BUSINESS, 
                                          "TicketService", QString("更新工单失败: %1").arg(lastError_));
        emit ticketUpdatedFailed(lastError_);
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("更新工单: %1").arg(ticket.getTitle()));
    
    // 发送更新工单请求（将在网络层实现后调用）
    sendUpdateTicketRequest(ticket);
    
    return false;
}

bool TicketService::deleteTicket(int ticketId)
{
    if (ticketId <= 0) {
        setError("工单ID无效");
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("删除工单: %1").arg(ticketId));
    
    // 发送删除工单请求（将在网络层实现后调用）
    sendDeleteTicketRequest(ticketId);
    
    return false;
}

Ticket TicketService::getTicketById(int ticketId)
{
    if (ticketId <= 0) {
        setError("工单ID无效");
        return Ticket();
    }
    
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取工单: %1").arg(ticketId));
    
    // 发送获取工单请求（将在网络层实现后调用）
    sendGetTicketRequest(ticketId);
    
    return Ticket();
}

Ticket TicketService::getTicketByTicketId(const QString& ticketId)
{
    if (ticketId.isEmpty()) {
        setError("工单编号无效");
        return Ticket();
    }
    
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取工单: %1").arg(ticketId));
    
    // TODO: 实现通过工单编号获取工单
    return Ticket();
}

QList<Ticket> TicketService::getTicketsByStatus(const QString& status, int limit, int offset)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取工单列表，状态: %1").arg(status));
    
    // 发送获取工单列表请求（将在网络层实现后调用）
    sendGetTicketListRequest(status, limit, offset);
    
    return QList<Ticket>();
}

QList<Ticket> TicketService::getTicketsByCreator(int creatorId, int limit, int offset)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取用户创建的工单: %1").arg(creatorId));
    
    // TODO: 实现获取用户创建的工单
    return QList<Ticket>();
}

QList<Ticket> TicketService::getTicketsByAssignee(int assigneeId, int limit, int offset)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取分配给用户的工单: %1").arg(assigneeId));
    
    // TODO: 实现获取分配给用户的工单
    return QList<Ticket>();
}

QList<Ticket> TicketService::getAllTickets(int limit, int offset)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "获取所有工单");
    
    // 发送获取所有工单请求（将在网络层实现后调用）
    sendGetTicketListRequest(QString(), limit, offset);
    
    return QList<Ticket>();
}

bool TicketService::updateTicketStatus(int ticketId, const QString& newStatus)
{
    if (ticketId <= 0) {
        setError("工单ID无效");
        return false;
    }
    
    if (newStatus.isEmpty()) {
        setError("新状态不能为空");
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("更新工单状态: %1 -> %2").arg(ticketId).arg(newStatus));
    
    // 发送更新状态请求（将在网络层实现后调用）
    sendUpdateStatusRequest(ticketId, newStatus);
    
    return false;
}

bool TicketService::closeTicket(int ticketId)
{
    return updateTicketStatus(ticketId, "closed");
}

bool TicketService::startProcessing(int ticketId)
{
    return updateTicketStatus(ticketId, "in_progress");
}

bool TicketService::refuseTicket(int ticketId, const QString& reason)
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("拒绝工单: %1, 原因: %2").arg(ticketId).arg(reason));
    
    // TODO: 实现拒绝工单
    return false;
}

bool TicketService::assignTicket(int ticketId, int assigneeId)
{
    if (ticketId <= 0) {
        setError("工单ID无效");
        return false;
    }
    
    if (assigneeId <= 0) {
        setError("分配用户ID无效");
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("分配工单: %1 -> 用户: %2").arg(ticketId).arg(assigneeId));
    
    // 发送分配工单请求（将在网络层实现后调用）
    sendAssignTicketRequest(ticketId, assigneeId);
    
    return false;
}

bool TicketService::unassignTicket(int ticketId)
{
    return assignTicket(ticketId, -1); // -1 表示取消分配
}

bool TicketService::joinTicket(const QString& ticketId, const QString& role)
{
    if (ticketId.isEmpty()) {
        setError("工单编号无效");
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("加入工单: %1, 角色: %2").arg(ticketId).arg(role));
    
    // 发送加入工单请求（将在网络层实现后调用）
    sendJoinTicketRequest(ticketId, role);
    
    return false;
}

bool TicketService::leaveTicket(const QString& ticketId)
{
    if (ticketId.isEmpty()) {
        setError("工单编号无效");
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("离开工单: %1").arg(ticketId));
    
    // 发送离开工单请求（将在网络层实现后调用）
    sendLeaveTicketRequest(ticketId);
    
    return false;
}

bool TicketService::addParticipant(int ticketId, const QString& username, const QString& role)
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("添加工单参与者: %1, 用户: %2, 角色: %3")
                                   .arg(ticketId).arg(username).arg(role));
    
    // TODO: 实现添加参与者
    return false;
}

bool TicketService::removeParticipant(int ticketId, const QString& username)
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("移除工单参与者: %1, 用户: %2")
                                   .arg(ticketId).arg(username));
    
    // TODO: 实现移除参与者
    return false;
}

QList<QString> TicketService::getTicketParticipants(int ticketId)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取工单参与者: %1").arg(ticketId));
    
    // TODO: 实现获取参与者列表
    return QList<QString>();
}

bool TicketService::isParticipant(int ticketId, const QString& username)
{
    // TODO: 实现检查是否为参与者
    return false;
}

int TicketService::getTicketCount(const QString& status)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取工单数量，状态: %1").arg(status));
    
    // TODO: 实现获取工单数量
    return 0;
}

int TicketService::getTicketCountByCreator(int creatorId)
{
    // TODO: 实现获取用户创建的工单数量
    return 0;
}

int TicketService::getTicketCountByAssignee(int assigneeId)
{
    // TODO: 实现获取分配给用户的工单数量
    return 0;
}

bool TicketService::canModifyTicket(int ticketId)
{
    // TODO: 实现权限检查
    return true;
}

bool TicketService::canCloseTicket(int ticketId)
{
    // TODO: 实现权限检查
    return true;
}

bool TicketService::canAssignTicket(int ticketId)
{
    // TODO: 实现权限检查
    return true;
}

QStringList TicketService::getNextPossibleStatuses(int ticketId)
{
    // TODO: 实现获取下一个可能的状态
    return QStringList();
}

bool TicketService::canTransitionTo(int ticketId, const QString& targetStatus)
{
    // TODO: 实现状态转换检查
    return true;
}

void TicketService::setError(const QString& error)
{
    lastError_ = error;
    LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("错误: %1").arg(error));
}

// 网络请求方法
void TicketService::sendCreateTicketRequest(const QString& title, const QString& description, 
                                         const QString& priority, const QString& category, 
                                         const QJsonObject& deviceInfo)
{
    if (!networkClient_) {
        setError("网络客户端未初始化");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "网络客户端未初始化");
        emit ticketCreatedFailed(lastError_);
        return;
    }
    
    if (!networkClient_->isConnected()) {
        setError("未连接到服务器");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "未连接到服务器");
        emit ticketCreatedFailed(lastError_);
        return;
    }
    
    // 通过网络客户端发送创建工单请求
    bool success = networkClient_->sendCreateTicketRequest(title, description, priority, category, deviceInfo);
    if (!success) {
        setError("发送创建工单请求失败");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "发送创建工单请求失败");
        emit ticketCreatedFailed(lastError_);
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "创建工单请求已发送");
}

void TicketService::sendUpdateTicketRequest(const Ticket& ticket)
{
    // TODO: 构建更新工单消息并发送
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "发送更新工单请求");
}

void TicketService::sendDeleteTicketRequest(int ticketId)
{
    // TODO: 构建删除工单消息并发送
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "发送删除工单请求");
}

void TicketService::sendGetTicketRequest(int ticketId)
{
    // TODO: 构建获取工单消息并发送
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "发送获取工单请求");
}

void TicketService::sendGetTicketListRequest(const QString& status, int limit, int offset)
{
    if (!networkClient_) {
        setError("网络客户端未初始化");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "网络客户端未初始化");
        emit ticketListFailed(lastError_);
        return;
    }
    
    if (!networkClient_->isConnected()) {
        setError("未连接到服务器");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "未连接到服务器");
        emit ticketListFailed(lastError_);
        return;
    }
    
    // 通过网络客户端发送获取工单列表请求
    bool success = networkClient_->sendGetTicketListRequest(status, limit, offset);
    if (!success) {
        setError("发送获取工单列表请求失败");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "发送获取工单列表请求失败");
        emit ticketListFailed(lastError_);
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "获取工单列表请求已发送");
}

void TicketService::sendUpdateStatusRequest(int ticketId, const QString& newStatus)
{
    // TODO: 构建更新状态消息并发送
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "发送更新状态请求");
}

void TicketService::sendAssignTicketRequest(int ticketId, int assigneeId)
{
    // TODO: 构建分配工单消息并发送
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "发送分配工单请求");
}

void TicketService::sendJoinTicketRequest(const QString& ticketId, const QString& role)
{
    if (!networkClient_) {
        setError("网络客户端未初始化");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "网络客户端未初始化");
        return;
    }
    
    if (!networkClient_->isConnected()) {
        setError("未连接到服务器");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "未连接到服务器");
        return;
    }
    
    // 通过网络客户端发送加入工单请求
    bool success = networkClient_->sendJoinTicketRequest(ticketId, role);
    if (!success) {
        setError("发送加入工单请求失败");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "发送加入工单请求失败");
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "加入工单请求已发送");
}

void TicketService::sendLeaveTicketRequest(const QString& ticketId)
{
    if (!networkClient_) {
        setError("网络客户端未初始化");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "网络客户端未初始化");
        return;
    }
    
    if (!networkClient_->isConnected()) {
        setError("未连接到服务器");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "未连接到服务器");
        return;
    }
    
    // 通过网络客户端发送离开工单请求
    bool success = networkClient_->sendLeaveTicketRequest(ticketId);
    if (!success) {
        setError("发送离开工单请求失败");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "发送离开工单请求失败");
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "离开工单请求已发送");
}

// 网络响应处理方法（占位符，将在网络层实现后完善）
void TicketService::onCreateTicketResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "收到创建工单响应");
    
    // TODO: 解析创建工单响应
    Ticket ticket;
    if (parseTicketResponse(response, ticket)) {
        emit ticketCreated(ticket);
    } else {
        emit ticketCreatedFailed(lastError_);
    }
}

void TicketService::onUpdateTicketResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "收到更新工单响应");
    
    // TODO: 解析更新工单响应
    Ticket ticket;
    if (parseTicketResponse(response, ticket)) {
        emit ticketUpdated(ticket);
    } else {
        emit ticketUpdatedFailed(lastError_);
    }
}

void TicketService::onDeleteTicketResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "收到删除工单响应");
    
    // TODO: 解析删除工单响应
    if (response.contains("success") && response["success"].toBool()) {
        int ticketId = response["ticketId"].toInt();
        emit ticketDeleted(ticketId);
    } else {
        emit ticketDeletedFailed(lastError_);
    }
}

void TicketService::onGetTicketResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "收到获取工单响应");
    
    // TODO: 解析获取工单响应
}

void TicketService::onGetTicketListResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "收到获取工单列表响应");
    
    // TODO: 解析获取工单列表响应
    QList<Ticket> tickets;
    if (parseTicketListResponse(response, tickets)) {
        emit ticketListReceived(tickets);
    } else {
        emit ticketListFailed(lastError_);
    }
}

void TicketService::onUpdateStatusResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "收到更新状态响应");
    
    // TODO: 解析更新状态响应
    int ticketId;
    QString newStatus;
    if (parseStatusUpdateResponse(response, ticketId, newStatus)) {
        // 这里需要获取旧状态，暂时使用空字符串
        emit ticketStatusChanged(ticketId, "", newStatus);
    }
}

void TicketService::onAssignTicketResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "收到分配工单响应");
    
    // TODO: 解析分配工单响应
    int ticketId;
    int assigneeId;
    if (parseAssignmentResponse(response, ticketId, assigneeId)) {
        emit ticketAssigned(ticketId, assigneeId);
    }
}

void TicketService::onJoinTicketResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "收到加入工单响应");
    
    // TODO: 解析加入工单响应
}

void TicketService::onLeaveTicketResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "收到离开工单响应");
    
    // TODO: 解析离开工单响应
}

// 响应解析方法（占位符，将在网络层实现后完善）
bool TicketService::parseTicketResponse(const QJsonObject& response, Ticket& ticket)
{
    // TODO: 解析工单响应
    return false;
}

bool TicketService::parseTicketListResponse(const QJsonObject& response, QList<Ticket>& tickets)
{
    // TODO: 解析工单列表响应
    return false;
}

bool TicketService::parseStatusUpdateResponse(const QJsonObject& response, int& ticketId, QString& newStatus)
{
    // TODO: 解析状态更新响应
    return false;
}

bool TicketService::parseAssignmentResponse(const QJsonObject& response, int& ticketId, int& assigneeId)
{
    // TODO: 解析分配响应
    return false;
}

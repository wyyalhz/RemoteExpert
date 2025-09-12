#include "ticket_service.h"
#include "../../common/protocol/builders/message_builder.h"
#include "../../network/client/network_client.h"

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
    
    if (networkClient_) {
        // 连接网络客户端的信号
        connect(networkClient_, &NetworkClient::getTicketDetailResponse, 
                this, &TicketService::onGetTicketDetailResponse);
        
        LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                       "TicketService", "网络客户端已设置，信号已连接");
    } else {
        LogManager::getInstance()->warning(LogModule::TICKET, LogLayer::BUSINESS, 
                                          "TicketService", "网络客户端为空");
    }
}

TicketService::~TicketService()
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", "工单服务销毁");
}

bool TicketService::createTicket(const QString& title, const QString& description, 
                               const QString& priority, const QString& category, 
                               const QString& expertUsername, const QJsonObject& deviceInfo)
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("创建工单: %1, 指派给专家: %2").arg(title).arg(expertUsername));
    
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
    
    if (expertUsername.isEmpty()) {
        setError("专家用户名不能为空");
        LogManager::getInstance()->warning(LogModule::TICKET, LogLayer::BUSINESS, 
                                          "TicketService", "创建工单失败: 专家用户名为空");
        emit ticketCreatedFailed(lastError_);
        return false;
    }
    
    // 发送创建工单请求
    sendCreateTicketRequest(title, description, priority, category, expertUsername, deviceInfo);
    
    return true; // 返回true表示请求已发送，等待响应
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
    
    // 发送更新工单请求
    sendUpdateTicketRequest(ticket);
    
    return true;
}

bool TicketService::deleteTicket(int ticketId)
{
    // 添加调试日志
    qDebug() << "TicketService::deleteTicket - 接收到的工单ID:" << ticketId;
    qDebug() << "工单ID类型:" << QVariant(ticketId).typeName();
    
    if (ticketId <= 0) {
        QString errorMsg = QString("工单ID无效: %1 (必须大于0)").arg(ticketId);
        qDebug() << "TicketService::deleteTicket - 验证失败:" << errorMsg;
        setError(errorMsg);
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("删除工单: %1").arg(ticketId));
    
    // 发送删除工单请求
    sendDeleteTicketRequest(ticketId);
    
    return true;
}

Ticket TicketService::getTicketById(int ticketId)
{
    if (ticketId <= 0) {
        setError("工单ID无效");
        return Ticket();
    }
    
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取工单: %1").arg(ticketId));
    
    // 发送获取工单请求
    sendGetTicketRequest(ticketId);
    
    return Ticket(); // 返回空Ticket，实际数据将通过信号返回
}

Ticket TicketService::getTicketByTicketId(const QString& ticketId)
{
    if (ticketId.isEmpty()) {
        setError("工单编号无效");
        return Ticket();
    }
    
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取工单: %1").arg(ticketId));
    
    // 将字符串ID转换为整数ID
    bool ok;
    int id = ticketId.toInt(&ok);
    if (ok) {
        // 发送获取工单详情请求
        sendGetTicketRequest(id);
    } else {
        setError("工单ID格式无效");
    }
    
    return Ticket(); // 返回空Ticket，实际数据将通过信号返回
}

Ticket TicketService::getTicketDetail(const QString& ticketId, int userId, int userType)
{
    if (ticketId.isEmpty()) {
        setError("工单编号无效");
        return Ticket();
    }
    
    if (userId <= 0) {
        setError("用户ID无效");
        return Ticket();
    }
    
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取工单详情: %1, 用户ID: %2, 用户类型: %3").arg(ticketId).arg(userId).arg(userType));
    
    // 发送获取工单详情请求
    sendGetTicketDetailRequest(ticketId, userId, userType);
    
    return Ticket(); // 返回空Ticket，实际数据将通过信号返回
}

QList<Ticket> TicketService::getTicketsByStatus(const QString& status, int limit, int offset)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取工单列表，状态: %1").arg(status));
    
    // 发送获取工单列表请求
    sendGetTicketListRequest(status, limit, offset);
    
    return QList<Ticket>();
}

QList<Ticket> TicketService::getTicketsByCreator(int creatorId, int limit, int offset)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取用户创建的工单: %1").arg(creatorId));
    
    if (creatorId <= 0) {
        setError("创建者ID无效");
        return QList<Ticket>();
    }
    
    // 发送获取用户创建的工单请求
    sendGetTicketListRequest("created", limit, offset); // 使用状态过滤
    
    return QList<Ticket>(); // 返回空列表，实际数据将通过信号返回
}

QList<Ticket> TicketService::getTicketsByAssignee(int assigneeId, int limit, int offset)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取分配给用户的工单: %1").arg(assigneeId));
    
    if (assigneeId <= 0) {
        setError("分配用户ID无效");
        return QList<Ticket>();
    }
    
    // 发送获取分配给用户的工单请求
    sendGetTicketListRequest("assigned", limit, offset); // 使用状态过滤
    
    return QList<Ticket>(); // 返回空列表，实际数据将通过信号返回
}

QList<Ticket> TicketService::getAllTickets(int limit, int offset)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "获取所有工单");
    
    // 发送获取所有工单请求（将在网络层实现后调用）
    sendGetTicketListRequest(QString(), limit, offset);
    
    return QList<Ticket>();
}

bool TicketService::updateTicketStatus(const QString& ticketId, const QString& newStatus)
{
    if (ticketId.isEmpty()) {
        setError("工单编号无效");
        return false;
    }
    
    if (newStatus.isEmpty()) {
        setError("新状态不能为空");
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("更新工单状态: %1 -> %2").arg(ticketId).arg(newStatus));
    
    // 发送更新状态请求
    sendUpdateStatusRequest(ticketId, newStatus);
    
    return true;
}

bool TicketService::closeTicket(const QString& ticketId)
{
    return updateTicketStatus(ticketId, "closed");
}

bool TicketService::startProcessing(const QString& ticketId)
{
    return updateTicketStatus(ticketId, "in_progress");
}

bool TicketService::refuseTicket(const QString& ticketId, const QString& reason)
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("拒绝工单: %1, 原因: %2").arg(ticketId).arg(reason));
    
    return updateTicketStatus(ticketId, "refused");
}

// 新增：具体的状态更新方法实现
bool TicketService::acceptTicket(const QString& ticketId)
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("接受工单: %1").arg(ticketId));
    
    return updateTicketStatus(ticketId, "processing");
}

bool TicketService::finishTicket(const QString& ticketId)
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("完成工单: %1").arg(ticketId));
    
    return updateTicketStatus(ticketId, "closed");
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
                                         const QString& expertUsername, const QJsonObject& deviceInfo)
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
    bool success = networkClient_->sendCreateTicketRequest(title, description, priority, category, expertUsername, deviceInfo);
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
    
    // 构建删除工单消息
    QJsonObject messageData;
    messageData["id"] = ticketId;
    messageData["ticketId"] = QString::number(ticketId); // 同时发送字符串格式的ID
    messageData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // 通过网络客户端发送删除工单请求
    bool success = networkClient_->sendMessage(MSG_DELETE_WORKORDER, messageData);
    if (!success) {
        setError("发送删除工单请求失败");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "发送删除工单请求失败");
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("删除工单请求已发送: 工单ID=%1").arg(ticketId));
}

void TicketService::sendGetTicketRequest(int ticketId)
{
    // TODO: 构建获取工单消息并发送
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "发送获取工单请求");
}

void TicketService::sendGetTicketDetailRequest(const QString& ticketId, int userId, int userType)
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
    
    // 通过网络客户端发送获取工单详情请求
    bool success = networkClient_->sendGetTicketDetailRequest(ticketId, userId, userType);
    if (!success) {
        setError("发送获取工单详情请求失败");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "发送获取工单详情请求失败");
        return;
    }
    
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("获取工单详情请求已发送: 工单ID=%1, 用户ID=%2, 用户类型=%3").arg(ticketId).arg(userId).arg(userType));
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

void TicketService::sendUpdateStatusRequest(const QString& ticketId, const QString& newStatus)
{
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("准备发送状态更新请求: 工单%1 -> %2").arg(ticketId).arg(newStatus));
    
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
    
    // 通过网络客户端发送更新状态请求
    bool success = networkClient_->sendUpdateStatusRequest(ticketId, newStatus);
    if (!success) {
        setError("发送更新状态请求失败");
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", "发送更新状态请求失败");
        return;
    }
    
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", QString("状态更新请求已发送: 工单%1 -> %2").arg(ticketId).arg(newStatus));
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
    
    // 检查响应状态
    if (response.contains("code") && response["code"].toInt() != 0) {
        QString error = response.value("message").toString();
        if (error.isEmpty()) error = "删除工单失败";
        setError(error);
        emit ticketDeletedFailed(lastError_);
        return;
    }
    
    // 解析删除的工单ID
    int ticketId = -1;
    if (response.contains("workOrderId")) {
        ticketId = response["workOrderId"].toInt();
    } else if (response.contains("ticketId")) {
        ticketId = response["ticketId"].toInt();
    } else if (response.contains("id")) {
        ticketId = response["id"].toInt();
    }
    
    if (ticketId > 0) {
        LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                       "TicketService", QString("工单删除成功: %1").arg(ticketId));
        emit ticketDeleted(ticketId);
    } else {
        setError("删除工单响应中未找到有效的工单ID");
        emit ticketDeletedFailed(lastError_);
    }
}

void TicketService::onGetTicketResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "收到获取工单响应");
    
    // TODO: 解析获取工单响应
}

void TicketService::onGetTicketDetailResponse(const QJsonObject& response)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "收到获取工单详情响应");
    
    // 检查响应状态
    if (response.contains("code") && response["code"].toInt() != 0) {
        QString error = response.value("message").toString();
        if (error.isEmpty()) error = "获取工单详情失败";
        setError(error);
        emit ticketDetailFailed(lastError_);
        return;
    }
    
    // 解析工单详情
    Ticket ticket;
    if (parseTicketResponse(response, ticket)) {
        LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                       "TicketService", QString("工单详情解析成功: %1").arg(ticket.getTitle()));
        emit ticketDetailReceived(ticket);
    } else {
        emit ticketDetailFailed(lastError_);
    }
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
    
    // 解析更新状态响应
    int ticketId;
    QString newStatus;
    if (parseStatusUpdateResponse(response, ticketId, newStatus)) {
        LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                       "TicketService", QString("工单状态更新成功: %1 -> %2").arg(ticketId).arg(newStatus));
        
        // 发出状态变化信号
        emit ticketStatusChanged(ticketId, "", newStatus);
        
        // 根据新状态发出相应的成功信号
        if (newStatus == "processing") {
            emit ticketAccepted(ticketId);
        } else if (newStatus == "refused") {
            emit ticketRefused(ticketId);
        } else if (newStatus == "closed") {
            emit ticketFinished(ticketId);
        }
    } else {
        LogManager::getInstance()->error(LogModule::TICKET, LogLayer::BUSINESS, 
                                        "TicketService", QString("状态更新响应解析失败: %1").arg(lastError_));
        emit ticketStatusUpdateFailed(ticketId, lastError_);
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
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "解析工单响应");
    
    // 检查响应状态 - 服务器返回格式：{"code": 0, "message": "...", ...}
    if (response.contains("code") && response["code"].toInt() != 0) {
        QString error = response.value("message").toString();
        if (error.isEmpty()) error = "操作失败";
        setError(error);
        return false;
    }
    
    // 解析工单信息 - 支持多种字段名
    if (response.contains("ticketid")) {
        ticket.setTicketId(response["ticketid"].toString());
    } else if (response.contains("ticket_id")) {
        ticket.setTicketId(response["ticket_id"].toString());
    }
    
    if (response.contains("title")) {
        ticket.setTitle(response["title"].toString());
    }
    
    if (response.contains("description")) {
        ticket.setDescription(response["description"].toString());
    }
    
    if (response.contains("status")) {
        ticket.setStatus(response["status"].toString());
    }
    
    if (response.contains("priority")) {
        ticket.setPriority(response["priority"].toString());
    }
    
    if (response.contains("category")) {
        ticket.setCategory(response["category"].toString());
    }
    
    // 解析用户信息 - 支持多种字段名
    if (response.contains("factory_username")) {
        ticket.setCreatorName(response["factory_username"].toString());
    } else if (response.contains("creatorName")) {
        ticket.setCreatorName(response["creatorName"].toString());
    }
    
    if (response.contains("expert_username")) {
        ticket.setAssigneeName(response["expert_username"].toString());
    } else if (response.contains("assigneeName")) {
        ticket.setAssigneeName(response["assigneeName"].toString());
    }
    
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("工单响应解析成功: %1").arg(ticket.getTitle()));
    return true;
}

bool TicketService::parseTicketListResponse(const QJsonObject& response, QList<Ticket>& tickets)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "解析工单列表响应");
    
    // 检查响应状态 - 服务器返回格式：{"code": 0, "message": "...", "work_orders": [...]}
    if (response.contains("code") && response["code"].toInt() != 0) {
        QString error = response.value("message").toString();
        if (error.isEmpty()) error = "获取工单列表失败";
        setError(error);
        return false;
    }
    
    // 解析工单列表
    if (response.contains("work_orders")) {
        QJsonArray workOrdersArray = response["work_orders"].toArray();
        tickets.clear();
        
        for (const QJsonValue& value : workOrdersArray) {
            if (value.isObject()) {
                QJsonObject ticketObj = value.toObject();
                LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                                "TicketService", QString("解析工单对象: %1").arg(QString(QJsonDocument(ticketObj).toJson())));
                
                Ticket ticket(ticketObj);
                if (ticket.isValid()) {
                    tickets.append(ticket);
                    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                                    "TicketService", QString("工单验证通过: %1").arg(ticket.getTitle()));
                } else {
                    LogManager::getInstance()->warning(LogModule::TICKET, LogLayer::BUSINESS, 
                                                      "TicketService", QString("工单验证失败: %1").arg(ticket.getValidationError()));
                }
            }
        }
        
        int totalCount = response.value("total_count").toInt();
        LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                       "TicketService", QString("工单列表解析成功: 共 %1 个工单").arg(tickets.size()));
        return true;
    }
    
    setError("响应数据格式错误");
    return false;
}

bool TicketService::parseStatusUpdateResponse(const QJsonObject& response, int& ticketId, QString& newStatus)
{
    LogManager::getInstance()->debug(LogModule::TICKET, LogLayer::BUSINESS, 
                                    "TicketService", "解析状态更新响应");
    
    // 检查响应状态
    if (response.contains("code") && response["code"].toInt() != 0) {
        QString error = response.value("message").toString();
        if (error.isEmpty()) error = "状态更新失败";
        setError(error);
        return false;
    }
    
    // 解析工单ID和新状态
    if (response.contains("ticket_id")) {
        ticketId = response["ticket_id"].toString().toInt();
    } else if (response.contains("workorderId")) {
        ticketId = response["workorderId"].toString().toInt();
    } else {
        setError("响应中缺少工单ID");
        return false;
    }
    
    if (response.contains("status")) {
        newStatus = response["status"].toString();
    } else {
        setError("响应中缺少新状态");
        return false;
    }
    
    LogManager::getInstance()->info(LogModule::TICKET, LogLayer::BUSINESS, 
                                   "TicketService", QString("状态更新响应解析成功: 工单%1 -> %2").arg(ticketId).arg(newStatus));
    return true;
}

bool TicketService::parseAssignmentResponse(const QJsonObject& response, int& ticketId, int& assigneeId)
{
    // TODO: 解析分配响应
    return false;
}

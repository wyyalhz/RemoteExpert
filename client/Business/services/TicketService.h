#ifndef TICKET_SERVICE_H
#define TICKET_SERVICE_H

#include "../models/Ticket.h"
#include "../../Logger/log_manager.h"
#include "../../Network/client/network_client.h"
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>

// 工单服务 - 负责工单创建、查询、更新等业务
class TicketService : public QObject
{
    Q_OBJECT

public:
    explicit TicketService(QObject *parent = nullptr);
    ~TicketService();

    // 工单创建和管理
    bool createTicket(const QString& title, const QString& description, 
                     const QString& priority, const QString& category, 
                     const QString& expertUsername, const QJsonObject& deviceInfo = QJsonObject());
    bool updateTicket(const Ticket& ticket);
    bool deleteTicket(int ticketId);
    
    // 工单查询
    Ticket getTicketById(int ticketId);
    Ticket getTicketByTicketId(const QString& ticketId);
    Ticket getTicketDetail(const QString& ticketId, int userId, int userType);
    QList<Ticket> getTicketsByStatus(const QString& status, int limit = -1, int offset = 0);
    QList<Ticket> getTicketsByCreator(int creatorId, int limit = -1, int offset = 0);
    QList<Ticket> getTicketsByAssignee(int assigneeId, int limit = -1, int offset = 0);
    QList<Ticket> getAllTickets(int limit = -1, int offset = 0);
    
    // 工单状态管理
    bool updateTicketStatus(int ticketId, const QString& newStatus);
    bool closeTicket(int ticketId);
    bool startProcessing(int ticketId);
    bool refuseTicket(int ticketId, const QString& reason = QString());
    
    // 工单分配
    bool assignTicket(int ticketId, int assigneeId);
    bool unassignTicket(int ticketId);
    
    // 工单参与者管理
    bool joinTicket(const QString& ticketId, const QString& role = "participant");
    bool leaveTicket(const QString& ticketId);
    bool addParticipant(int ticketId, const QString& username, const QString& role);
    bool removeParticipant(int ticketId, const QString& username);
    QList<QString> getTicketParticipants(int ticketId);
    bool isParticipant(int ticketId, const QString& username);
    
    // 工单统计
    int getTicketCount(const QString& status = QString());
    int getTicketCountByCreator(int creatorId);
    int getTicketCountByAssignee(int assigneeId);
    
    // 权限检查
    bool canModifyTicket(int ticketId);
    bool canCloseTicket(int ticketId);
    bool canAssignTicket(int ticketId);
    
    // 状态查询
    QStringList getNextPossibleStatuses(int ticketId);
    bool canTransitionTo(int ticketId, const QString& targetStatus);
    
    // 错误处理
    QString getLastError() const { return lastError_; }
    void clearError() { lastError_.clear(); }

signals:
    // 工单状态变化信号
    void ticketCreated(const Ticket& ticket);
    void ticketCreatedFailed(const QString& error);
    void ticketUpdated(const Ticket& ticket);
    void ticketUpdatedFailed(const QString& error);
    void ticketDeleted(int ticketId);
    void ticketDeletedFailed(const QString& error);
    void ticketStatusChanged(int ticketId, const QString& oldStatus, const QString& newStatus);
    void ticketAssigned(int ticketId, int assigneeId);
    void ticketClosed(int ticketId);
    void ticketListReceived(const QList<Ticket>& tickets);
    void ticketListFailed(const QString& error);
    void ticketDetailReceived(const Ticket& ticket);
    void ticketDetailFailed(const QString& error);
    void participantJoined(int ticketId, const QString& username);
    void participantLeft(int ticketId, const QString& username);



private:
    // 私有辅助方法
    void setError(const QString& error);
    
    // 网络请求方法（将在网络层实现后调用）
    void sendCreateTicketRequest(const QString& title, const QString& description, 
                               const QString& priority, const QString& category, 
                               const QString& expertUsername, const QJsonObject& deviceInfo);
    void sendUpdateTicketRequest(const Ticket& ticket);
    void sendDeleteTicketRequest(int ticketId);
    void sendGetTicketRequest(int ticketId);
    void sendGetTicketDetailRequest(const QString& ticketId, int userId, int userType);
    void sendGetTicketListRequest(const QString& status = QString(), int limit = -1, int offset = 0);
    void sendUpdateStatusRequest(int ticketId, const QString& newStatus);
    void sendAssignTicketRequest(int ticketId, int assigneeId);
    void sendJoinTicketRequest(const QString& ticketId, const QString& role);
    void sendLeaveTicketRequest(const QString& ticketId);
    
    // 响应解析方法
    bool parseTicketResponse(const QJsonObject& response, Ticket& ticket);
    bool parseTicketListResponse(const QJsonObject& response, QList<Ticket>& tickets);
    bool parseStatusUpdateResponse(const QJsonObject& response, int& ticketId, QString& newStatus);
    bool parseAssignmentResponse(const QJsonObject& response, int& ticketId, int& assigneeId);

private:
    QString lastError_;
    
    // 网络客户端引用
    NetworkClient* networkClient_;
    
public:
    // 设置网络客户端
    void setNetworkClient(NetworkClient* client);

private:

public slots:
    // 网络响应处理
    void onCreateTicketResponse(const QJsonObject& response);
    void onUpdateTicketResponse(const QJsonObject& response);
    void onDeleteTicketResponse(const QJsonObject& response);
    void onGetTicketResponse(const QJsonObject& response);
    void onGetTicketDetailResponse(const QJsonObject& response);
    void onGetTicketListResponse(const QJsonObject& response);
    void onUpdateStatusResponse(const QJsonObject& response);
    void onAssignTicketResponse(const QJsonObject& response);
    void onJoinTicketResponse(const QJsonObject& response);
    void onLeaveTicketResponse(const QJsonObject& response);
};

#endif // TICKET_SERVICE_H

#ifndef TICKET_H
#define TICKET_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>

// 工单业务模型
class Ticket
{
public:
    Ticket();
    Ticket(const QString& title, const QString& description, int creatorId);
    Ticket(const QJsonObject& json);
    
    // 基本属性
    int getId() const { return id_; }
    void setId(int id) { id_ = id; }
    
    QString getTicketId() const { return ticketId_; }
    void setTicketId(const QString& ticketId) { ticketId_ = ticketId; }
    
    QString getTitle() const { return title_; }
    void setTitle(const QString& title) { title_ = title; }
    
    QString getDescription() const { return description_; }
    void setDescription(const QString& description) { description_ = description; }
    
    int getCreatorId() const { return creatorId_; }
    void setCreatorId(int creatorId) { creatorId_ = creatorId; }
    
    QString getCreatorName() const { return creatorName_; }
    void setCreatorName(const QString& name) { creatorName_ = name; }
    
    int getAssigneeId() const { return assigneeId_; }
    void setAssigneeId(int assigneeId) { assigneeId_ = assigneeId; }
    
    QString getAssigneeName() const { return assigneeName_; }
    void setAssigneeName(const QString& name) { assigneeName_ = name; }
    
    QString getStatus() const { return status_; }
    void setStatus(const QString& status) { status_ = status; }
    
    QString getPriority() const { return priority_; }
    void setPriority(const QString& priority) { priority_ = priority; }
    
    QString getCategory() const { return category_; }
    void setCategory(const QString& category) { category_ = category; }
    
    QDateTime getCreatedTime() const { return createdTime_; }
    void setCreatedTime(const QDateTime& time) { createdTime_ = time; }
    
    QDateTime getUpdatedTime() const { return updatedTime_; }
    void setUpdatedTime(const QDateTime& time) { updatedTime_ = time; }
    
    QDateTime getClosedTime() const { return closedTime_; }
    void setClosedTime(const QDateTime& time) { closedTime_ = time; }
    
    QString getRoomId() const { return roomId_; }
    void setRoomId(const QString& roomId) { roomId_ = roomId; }
    
    QJsonObject getDeviceInfo() const { return deviceInfo_; }
    void setDeviceInfo(const QJsonObject& info) { deviceInfo_ = info; }
    
    // 参与者列表
    QList<QString> getParticipants() const { return participants_; }
    void setParticipants(const QList<QString>& participants) { participants_ = participants; }
    void addParticipant(const QString& participant) { participants_.append(participant); }
    void removeParticipant(const QString& participant) { participants_.removeOne(participant); }
    
    // 序列化方法
    QJsonObject toJson() const;
    static Ticket fromJson(const QJsonObject& json);
    
    // 验证方法
    bool isValid() const;
    QString getValidationError() const;
    
    // 状态判断
    bool isCreated() const { return status_ == "created"; }
    bool isAssigned() const { return status_ == "assigned"; }
    bool isInProgress() const { return status_ == "in_progress"; }
    bool isResolved() const { return status_ == "resolved"; }
    bool isClosed() const { return status_ == "closed"; }
    
    // 优先级判断
    bool isHighPriority() const { return priority_ == "high"; }
    bool isMediumPriority() const { return priority_ == "medium"; }
    bool isLowPriority() const { return priority_ == "low"; }
    
    // 显示方法
    QString getStatusDisplayName() const;
    QString getPriorityDisplayName() const;
    QString getShortDescription() const;
    
    // 业务方法
    bool canTransitionTo(const QString& newStatus) const;
    QStringList getNextPossibleStatuses() const;
    bool isParticipant(const QString& username) const;
    
private:
    int id_;
    QString ticketId_;
    QString title_;
    QString description_;
    int creatorId_;
    QString creatorName_;
    int assigneeId_;
    QString assigneeName_;
    QString status_;      // created, assigned, in_progress, resolved, closed
    QString priority_;    // low, medium, high
    QString category_;
    QDateTime createdTime_;
    QDateTime updatedTime_;
    QDateTime closedTime_;
    QString roomId_;
    QJsonObject deviceInfo_;
    QList<QString> participants_;
};

#endif // TICKET_H

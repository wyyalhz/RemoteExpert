#include "Ticket.h"
#include <QJsonArray>

Ticket::Ticket()
    : id_(-1)
    , creatorId_(-1)
    , assigneeId_(-1)
{
}

Ticket::Ticket(const QString& title, const QString& description, int creatorId)
    : id_(-1)
    , title_(title)
    , description_(description)
    , creatorId_(creatorId)
    , assigneeId_(-1)
    , status_("created")
    , priority_("medium")
    , createdTime_(QDateTime::currentDateTime())
    , updatedTime_(QDateTime::currentDateTime())
{
}

Ticket::Ticket(const QJsonObject& json)
    : id_(-1)
    , creatorId_(-1)
    , assigneeId_(-1)
{
    fromJson(json);
}

QJsonObject Ticket::toJson() const
{
    QJsonObject json;
    json["id"] = id_;
    json["ticketId"] = ticketId_;
    json["title"] = title_;
    json["description"] = description_;
    json["creatorId"] = creatorId_;
    json["creatorName"] = creatorName_;
    json["assigneeId"] = assigneeId_;
    json["assigneeName"] = assigneeName_;
    json["status"] = status_;
    json["priority"] = priority_;
    json["category"] = category_;
    json["createdTime"] = createdTime_.toString(Qt::ISODate);
    json["updatedTime"] = updatedTime_.toString(Qt::ISODate);
    json["closedTime"] = closedTime_.toString(Qt::ISODate);
    json["roomId"] = roomId_;
    json["deviceInfo"] = deviceInfo_;
    
    QJsonArray participantsArray;
    for (const QString& participant : participants_) {
        participantsArray.append(participant);
    }
    json["participants"] = participantsArray;
    
    return json;
}

Ticket Ticket::fromJson(const QJsonObject& json)
{
    Ticket ticket;
    
    if (json.contains("id")) {
        ticket.id_ = json["id"].toInt();
    }
    
    if (json.contains("ticketId")) {
        ticket.ticketId_ = json["ticketId"].toString();
    }
    
    if (json.contains("title")) {
        ticket.title_ = json["title"].toString();
    }
    
    if (json.contains("description")) {
        ticket.description_ = json["description"].toString();
    }
    
    if (json.contains("creatorId")) {
        ticket.creatorId_ = json["creatorId"].toInt();
    }
    
    if (json.contains("creatorName")) {
        ticket.creatorName_ = json["creatorName"].toString();
    }
    
    if (json.contains("assigneeId")) {
        ticket.assigneeId_ = json["assigneeId"].toInt();
    }
    
    if (json.contains("assigneeName")) {
        ticket.assigneeName_ = json["assigneeName"].toString();
    }
    
    if (json.contains("status")) {
        ticket.status_ = json["status"].toString();
    }
    
    if (json.contains("priority")) {
        ticket.priority_ = json["priority"].toString();
    }
    
    if (json.contains("category")) {
        ticket.category_ = json["category"].toString();
    }
    
    if (json.contains("createdTime")) {
        ticket.createdTime_ = QDateTime::fromString(json["createdTime"].toString(), Qt::ISODate);
    }
    
    if (json.contains("updatedTime")) {
        ticket.updatedTime_ = QDateTime::fromString(json["updatedTime"].toString(), Qt::ISODate);
    }
    
    if (json.contains("closedTime")) {
        ticket.closedTime_ = QDateTime::fromString(json["closedTime"].toString(), Qt::ISODate);
    }
    
    if (json.contains("roomId")) {
        ticket.roomId_ = json["roomId"].toString();
    }
    
    if (json.contains("deviceInfo")) {
        ticket.deviceInfo_ = json["deviceInfo"].toObject();
    }
    
    if (json.contains("participants")) {
        QJsonArray participantsArray = json["participants"].toArray();
        for (const QJsonValue& value : participantsArray) {
            ticket.participants_.append(value.toString());
        }
    }
    
    return ticket;
}

bool Ticket::isValid() const
{
    return !title_.isEmpty() && !description_.isEmpty() && creatorId_ > 0;
}

QString Ticket::getValidationError() const
{
    if (title_.isEmpty()) {
        return "工单标题不能为空";
    }
    
    if (description_.isEmpty()) {
        return "工单描述不能为空";
    }
    
    if (creatorId_ <= 0) {
        return "创建者ID无效";
    }
    
    return QString();
}

QString Ticket::getStatusDisplayName() const
{
    if (status_ == "created") return "已创建";
    if (status_ == "assigned") return "已分配";
    if (status_ == "in_progress") return "进行中";
    if (status_ == "resolved") return "已解决";
    if (status_ == "closed") return "已关闭";
    return status_;
}

QString Ticket::getPriorityDisplayName() const
{
    if (priority_ == "low") return "低";
    if (priority_ == "medium") return "中";
    if (priority_ == "high") return "高";
    return priority_;
}

QString Ticket::getShortDescription() const
{
    if (description_.length() <= 50) {
        return description_;
    }
    return description_.left(50) + "...";
}

bool Ticket::canTransitionTo(const QString& newStatus) const
{
    // 状态转换规则
    if (status_ == "created") {
        return newStatus == "assigned" || newStatus == "closed";
    }
    if (status_ == "assigned") {
        return newStatus == "in_progress" || newStatus == "closed";
    }
    if (status_ == "in_progress") {
        return newStatus == "resolved" || newStatus == "closed";
    }
    if (status_ == "resolved") {
        return newStatus == "closed";
    }
    if (status_ == "closed") {
        return false; // 已关闭状态不能转换
    }
    return false;
}

QStringList Ticket::getNextPossibleStatuses() const
{
    QStringList nextStatuses;
    
    if (status_ == "created") {
        nextStatuses << "assigned" << "closed";
    } else if (status_ == "assigned") {
        nextStatuses << "in_progress" << "closed";
    } else if (status_ == "in_progress") {
        nextStatuses << "resolved" << "closed";
    } else if (status_ == "resolved") {
        nextStatuses << "closed";
    }
    
    return nextStatuses;
}

bool Ticket::isParticipant(const QString& username) const
{
    return participants_.contains(username);
}

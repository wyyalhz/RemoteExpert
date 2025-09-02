// ===============================================
// chat/chat_model.cpp
// 聊天消息模型实现
// ===============================================

#include "chatmodel.h"
#include <QDateTime>

ChatModel::ChatModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_currentUserId(0)
{
}

int ChatModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid())
        return 0;
    return m_messages.size();
}

QVariant ChatModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_messages.size())
        return QVariant();

    const TextMessage& message = m_messages.at(index.row());

    switch (role) {
    case RoomIdRole:
        return message.roomId;
    case UserIdRole:
        return message.userId;
    case UserNameRole:
        return message.userName;
    case ContentRole:
        return message.content;
    case TimestampRole:
        return message.timestamp.toString("hh:mm:ss");
    case MessageTypeRole:
        return static_cast<int>(message.type);
    case IsOwnMessageRole:
        return message.userId == m_currentUserId;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChatModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[RoomIdRole] = "roomId";
    roles[UserIdRole] = "userId";
    roles[UserNameRole] = "userName";
    roles[ContentRole] = "content";
    roles[TimestampRole] = "timestamp";
    roles[MessageTypeRole] = "messageType";
    roles[IsOwnMessageRole] = "isOwnMessage";
    return roles;
}

void ChatModel::addMessage(const TextMessage& message) {
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append(message);
    endInsertRows();

    emit newMessageReceived(message);
}

void ChatModel::addSystemMessage(uint32_t roomId, const QString& content) {
    TextMessage message;
    message.roomId = roomId;
    message.userId = 0;
    message.userName = "系统";
    message.content = content;
    message.type = TextMessageType::SYSTEM;
    message.timestamp = QDateTime::currentDateTime();

    addMessage(message);
}

void ChatModel::clearMessages() {
    beginResetModel();
    m_messages.clear();
    endResetModel();
}

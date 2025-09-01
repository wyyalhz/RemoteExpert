// ===============================================
// chat/chat_model.h
// 聊天消息模型
// ===============================================

#pragma once

#include <QAbstractListModel>
#include <QVector>
#include "protocol.h"

class ChatModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role {
        RoomIdRole = Qt::UserRole + 1,
        UserIdRole,
        UserNameRole,
        ContentRole,
        TimestampRole,
        MessageTypeRole,
        IsOwnMessageRole
    };

    explicit ChatModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addMessage(const TextMessage& message);
    void addSystemMessage(uint32_t roomId, const QString& content);
    void clearMessages();
    QVector<TextMessage> getMessages() const { return m_messages; }

    void setCurrentUserId(uint32_t userId) { m_currentUserId = userId; }
    uint32_t currentUserId() const { return m_currentUserId; }

signals:
    void newMessageReceived(const TextMessage& message);

private:
    QVector<TextMessage> m_messages;
    uint32_t m_currentUserId;
};

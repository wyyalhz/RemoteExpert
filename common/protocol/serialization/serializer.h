#pragma once
// ===============================================
// common/protocol/serialization/serializer.h
// 序列化工具定义
// ===============================================

#include <QtCore>
#include <QtNetwork>
#include "packet.h"
#include "../types/constants.h"

// 工具：JSON编解码（使用紧凑格式，节约带宽）
inline QByteArray toJsonBytes(const QJsonObject& j) {
    return QJsonDocument(j).toJson(QJsonDocument::Compact);
}

inline QJsonObject fromJsonBytes(const QByteArray& b) {
    auto doc = QJsonDocument::fromJson(b);
    return doc.isObject() ? doc.object() : QJsonObject{};
}

// 打包（发送前调用）
QByteArray buildPacket(quint16 type,
                       const QJsonObject& json,
                       const QByteArray& bin = QByteArray());

// 拆包（在QTcpSocket::readyRead里，把readAll追加到buffer，然后调用drainPackets）
// - 解决粘包/半包；只要buffer里有完整包就会解析出来放进out
// - 返回是否至少解析出1个完整包
bool drainPackets(QByteArray& buffer, QVector<Packet>& out);

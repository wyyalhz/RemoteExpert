#pragma once
// ===============================================
// common/protocol.h
// 统一协议（打包/拆包）最小实现
// 结构: [uint32 length][uint16 type][uint32 jsonSize][jsonBytes][bin...]
// - length: 从 type 开始的总字节数（大端序）
// - type  : 消息类型（见 protocol_types.h 中的 MsgType 枚举）
// - jsonSize: JSON字节长度（UTF-8, Compact）
// - jsonBytes: 固定存在的JSON（至少包含roomId/ts等字段）
// - bin   : 可选二进制负载（如JPEG/PCM）
// ===============================================

#include <QtCore>
#include <QtNetwork>
#include "protocol_types.h"  // 包含消息类型定义

// 一条完整消息
struct Packet {
    quint16 type = 0;
    QJsonObject json;
    QByteArray bin; // 可为空
};

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

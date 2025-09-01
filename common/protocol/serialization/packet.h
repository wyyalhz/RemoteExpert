#pragma once
// ===============================================
// common/protocol/serialization/packet.h
// 数据包结构定义
// ===============================================

#include <QtCore>
#include <QtNetwork>

// 一条完整消息
struct Packet {
    quint16 type = 0;
    QJsonObject json;
    QByteArray bin; // 可为空
};

#pragma once
// ===============================================
// common/protocol.h
// 统一协议（打包/拆包）最小实现
// 结构: [uint32 length][uint16 type][uint32 jsonSize][jsonBytes][bin...]
// - length: 从 type 开始的总字节数（大端序）
// - type  : 消息类型（见枚举 MsgType）
// - jsonSize: JSON字节长度（UTF-8, Compact）
// - jsonBytes: 固定存在的JSON（至少包含roomId/ts等字段）
// - bin   : 可选二进制负载（如JPEG/PCM）
// ===============================================

#include <QtCore>
#include <QtNetwork>

// 统一的消息类型定义 —— 以后扩展只“新增值”，不要修改已存在的值
enum MsgType : quint16 {
    MSG_REGISTER         = 1,   // 注册（示例骨架暂未实现）
    MSG_LOGIN            = 2,   // 登录（示例骨架暂未实现）
    MSG_CREATE_WORKORDER = 3,   // 创建工单（示例骨架暂未实现）
    MSG_JOIN_WORKORDER   = 4,   // 加入工单（设置roomId + username）

    MSG_TEXT             = 10,  // 文本聊天（先跑通端到端）
    // 设备/音视频后续添加：
    MSG_DEVICE_DATA      = 20,  // 设备数据（纯JSON即可）
    MSG_VIDEO_FRAME      = 30,  // bin: JPEG
    MSG_AUDIO_FRAME      = 40,  // bin: PCM S16LE
    MSG_CONTROL          = 50,  // 控制指令（可选加分）

    MSG_SERVER_EVENT     = 90   // 服务器提示/错误/房间事件等
};

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

#include "protocol.h"

// 头字段大小常量（以便统一维护）
static const int kLenFieldSize = 4; // uint32 length（大端）
static const int kTypeSize     = 2; // uint16
static const int kJsonSizeSize = 4; // uint32

QByteArray buildPacket(quint16 type,
                       const QJsonObject& json,
                       const QByteArray& bin)
{
    QByteArray jsonBytes = toJsonBytes(json);
    quint32 jsonSize = static_cast<quint32>(jsonBytes.size());
    quint32 length = static_cast<quint32>(kTypeSize + kJsonSizeSize + jsonSize + bin.size());

    QByteArray out;
    out.reserve(kLenFieldSize + length);

    QDataStream ds(&out, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);

    ds << length;   // 4B: 后续总长度（从type开始）
    ds << type;     // 2B: 消息类型
    ds << jsonSize; // 4B: JSON长度
    if (!jsonBytes.isEmpty())
        ds.writeRawData(jsonBytes.constData(), jsonBytes.size());
    if (!bin.isEmpty())
        ds.writeRawData(bin.constData(), bin.size());

    return out;
}

bool drainPackets(QByteArray& buffer, QVector<Packet>& out)
{
    bool produced = false;

    for (;;) {
        if (buffer.size() < kLenFieldSize) break; // 连长度都不够

        // 读取length（大端）但不消费
        quint32 length = 0;
        {
            QDataStream peek(buffer.left(kLenFieldSize));
            peek.setByteOrder(QDataStream::BigEndian);
            peek >> length;
        }
        if (length < (quint32)(kTypeSize + kJsonSizeSize)) {
            // 异常防御：声明的长度过小
            buffer.clear();
            break;
        }

        const int totalNeed = kLenFieldSize + int(length);
        if (buffer.size() < totalNeed) break; // 半包，等待更多数据

        // 取出一个完整包块并从buffer中移除
        QByteArray block = buffer.left(totalNeed);
        buffer.remove(0, totalNeed);

        // 解析 block
        QDataStream ds(block);
        ds.setByteOrder(QDataStream::BigEndian);

        quint32 lenField = 0; ds >> lenField; Q_UNUSED(lenField);
        quint16 type = 0;     ds >> type;
        quint32 jsonSize = 0; ds >> jsonSize;

        // 检查jsonSize合法性
        const int payloadBytes = totalNeed - kLenFieldSize - kTypeSize - kJsonSizeSize;
        if (jsonSize > (quint32)payloadBytes) {
            // 非法，丢弃
            continue;
        }

        QByteArray jsonBytes(jsonSize, Qt::Uninitialized);
        if (jsonSize > 0) {
            ds.readRawData(jsonBytes.data(), jsonBytes.size());
        }
        QByteArray bin;
        const int binSize = payloadBytes - int(jsonSize);
        if (binSize > 0) {
            bin = block.right(binSize);
        }

        Packet pkt;
        pkt.type = type;
        pkt.json = fromJsonBytes(jsonBytes);
        pkt.bin  = bin;
        out.push_back(std::move(pkt));
        produced = true;
    }

    return produced;
}

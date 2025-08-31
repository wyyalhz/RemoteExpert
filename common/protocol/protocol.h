#pragma once
// ===============================================
// common/protocol/protocol.h
// 协议模块主入口头文件
// 统一协议（打包/拆包）最小实现
// 结构: [uint32 length][uint16 type][uint32 jsonSize][jsonBytes][bin...]
// - length: 从 type 开始的总字节数（大端序）
// - type  : 消息类型（见 types/enums.h 中的 MsgType 枚举）
// - jsonSize: JSON字节长度（UTF-8, Compact）
// - jsonBytes: 固定存在的JSON（至少包含roomId/ts等字段）
// - bin   : 可选二进制负载（如JPEG/PCM）
// ===============================================

#include <QtCore>
#include <QtNetwork>

// 类型定义
#include "types/enums.h"
#include "types/constants.h"
#include "types/validation_rules.h"

// 序列化
#include "serialization/packet.h"
#include "serialization/serializer.h"

// 工具类
#include "builders/message_builder.h"
#include "validators/message_validator.h"
#include "parsers/message_parser.h"
#include "converters/enum_converter.h"

// 为了向后兼容，提供原有的接口
using Packet = ::Packet;
using MessageBuilder = ::MessageBuilder;
using MessageValidator = ::MessageValidator;
using MessageParser = ::MessageParser;
using EnumConverter = ::EnumConverter;

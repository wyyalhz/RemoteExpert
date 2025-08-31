#pragma once
// ===============================================
// common/protocol/types/validation_rules.h
// 消息验证规则定义
// ===============================================

#include <QString>
#include <QJsonObject>
#include "constants.h"

namespace ValidationRules {
    // 通用验证规则
    static const int MAX_USERNAME_LENGTH = ProtocolConstants::MAX_USERNAME_LENGTH;
    static const int MAX_PASSWORD_LENGTH = ProtocolConstants::MAX_PASSWORD_LENGTH;
    static const int MAX_TEXT_LENGTH = ProtocolConstants::MAX_TEXT_LENGTH;
    static const int MAX_TITLE_LENGTH = ProtocolConstants::MAX_TITLE_LENGTH;
    static const int MAX_DESCRIPTION_LENGTH = ProtocolConstants::MAX_DESCRIPTION_LENGTH;
    
    // 二进制数据限制
    static const int MAX_VIDEO_FRAME_SIZE = ProtocolConstants::MAX_VIDEO_FRAME_SIZE;
    static const int MAX_AUDIO_FRAME_SIZE = ProtocolConstants::MAX_AUDIO_FRAME_SIZE;
    static const int MAX_FILE_SIZE = ProtocolConstants::MAX_FILE_SIZE;
    
    // 时间限制
    static const int HEARTBEAT_INTERVAL = ProtocolConstants::HEARTBEAT_INTERVAL;
    static const int SESSION_TIMEOUT = ProtocolConstants::SESSION_TIMEOUT;
}

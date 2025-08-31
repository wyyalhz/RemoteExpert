#pragma once
// ===============================================
// common/protocol/types/constants.h
// 协议常量定义
// ===============================================

namespace ProtocolConstants {
    // 通用字段长度限制
    static const int MAX_USERNAME_LENGTH = 50;
    static const int MAX_PASSWORD_LENGTH = 100;
    static const int MAX_TEXT_LENGTH = 1000;
    static const int MAX_TITLE_LENGTH = 200;
    static const int MAX_DESCRIPTION_LENGTH = 2000;
    
    // 二进制数据限制
    static const int MAX_VIDEO_FRAME_SIZE = 1024 * 1024;  // 1MB
    static const int MAX_AUDIO_FRAME_SIZE = 64 * 1024;    // 64KB
    static const int MAX_FILE_SIZE = 10 * 1024 * 1024;    // 10MB
    
    // 时间限制
    static const int HEARTBEAT_INTERVAL = 30;  // 30秒
    static const int SESSION_TIMEOUT = 1800;   // 30分钟
    
    // 协议字段大小
    static const int LENGTH_FIELD_SIZE = 4;    // uint32 length
    static const int TYPE_FIELD_SIZE = 2;      // uint16 type
    static const int JSON_SIZE_FIELD_SIZE = 4; // uint32 jsonSize
}

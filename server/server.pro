QT += core sql widgets network
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TEMPLATE = app

TARGET = server

DESTDIR = $$PWD/bin

# 编译警告设置
DEFINES += QT_DEPRECATED_WARNINGS

# 协议模块
include(../common/protocol/protocol.pri)

# 源文件
SOURCES += \
    # 主程序入口
    src/main.cpp \
    # 日志模块
    src/logging/base/logger_base.cpp \
    src/logging/config/log_config.cpp \
    src/logging/managers/log_manager.cpp \
    # 数据库层
    src/data/databasemanager.cpp \
    src/data/base/db_base.cpp \
    src/data/models/user_model.cpp \
    src/data/models/workorder_model.cpp \
    src/data/models/session_model.cpp \
    src/data/repositories/user_repository.cpp \
    src/data/repositories/workorder_repository.cpp \
    src/data/repositories/session_repository.cpp \
    src/data/logging/db_logger.cpp \
    # 业务逻辑层
    src/business/exceptions/business_exception.cpp \
    src/business/logging/business_logger.cpp \
    src/business/managers/workorder_status_manager.cpp \
    src/business/services/user_service.cpp \
    src/business/services/workorder_service.cpp \
    src/business/services/session_service.cpp \
    src/business/validators/user_validator.cpp \
    src/business/validators/workorder_validator.cpp \
    # 网络层
    src/network/network_server.cpp \
    src/network/connection_manager.cpp \
    src/network/server/tcp_server.cpp \
    src/network/protocol/message_router.cpp \
    src/network/protocol/protocol_handler.cpp \
    src/network/protocol/protocol_handlers/user_handler.cpp \
    src/network/protocol/protocol_handlers/workorder_handler.cpp \
    src/network/protocol/protocol_handlers/chat_handler.cpp \
    src/network/logging/network_logger.cpp

# 头文件
HEADERS += \
    # 日志模块
    src/logging/base/logger_base.h \
    src/logging/config/log_config.h \
    src/logging/managers/log_manager.h \
    # 数据库层
    src/data/databasemanager.h \
    src/data/base/db_base.h \
    src/data/models/user_model.h \
    src/data/models/workorder_model.h \
    src/data/models/session_model.h \
    src/data/repositories/user_repository.h \
    src/data/repositories/workorder_repository.h \
    src/data/repositories/session_repository.h \
    src/data/logging/db_logger.h \
    # 业务逻辑层
    src/business/exceptions/business_exception.h \
    src/business/logging/business_logger.h \
    src/business/managers/workorder_status_manager.h \
    src/business/services/user_service.h \
    src/business/services/workorder_service.h \
    src/business/services/session_service.h \
    src/business/validators/user_validator.h \
    src/business/validators/workorder_validator.h \
    # 网络层
    src/network/network_server.h \
    src/network/connection_manager.h \
    src/network/server/tcp_server.h \
    src/network/protocol/message_router.h \
    src/network/protocol/protocol_handler.h \
    src/network/protocol/protocol_handlers/user_handler.h \
    src/network/protocol/protocol_handlers/workorder_handler.h \
    src/network/protocol/protocol_handlers/chat_handler.h \
    src/network/logging/network_logger.h

# 包含路径
INCLUDEPATH += \
    src \
    # 日志模块
    src/logging \
    src/logging/base \
    src/logging/config \
    src/logging/managers \
    # 数据库层
    src/data \
    src/data/base \
    src/data/models \
    src/data/repositories \
    src/data/logging \
    # 业务逻辑层
    src/business \
    src/business/exceptions \
    src/business/logging \
    src/business/managers \
    src/business/services \
    src/business/validators \
    # 网络层
    src/network \
    src/network/server \
    src/network/protocol \
    src/network/protocol/protocol_handlers \
    src/network/logging


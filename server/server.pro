QT += core sql widgets network
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TEMPLATE = app

# 源文件
SOURCES += \
    src/main.cpp \
    src/roomhub.cpp \
    src/databasemanager/databasemanager.cpp \
    src/databasemanager/base/db_base.cpp \
    src/databasemanager/user/user_manager.cpp \
    src/databasemanager/workorder/workorder_manager.cpp \
    src/databasemanager/logger/db_logger.cpp \
    ../common/protocol.cpp

# 头文件
HEADERS += \
    src/roomhub.h \
    src/databasemanager/databasemanager.h \
    src/databasemanager/base/db_base.h \
    src/databasemanager/user/user_manager.h \
    src/databasemanager/workorder/workorder_manager.h \
    src/databasemanager/logger/db_logger.h \
    ../common/protocol.h

# 包含路径
INCLUDEPATH += \
    src \
    src/databasemanager \
    src/databasemanager/base \
    src/databasemanager/user \
    src/databasemanager/workorder \
    src/databasemanager/logger \
    ../common

# 目标名称
TARGET = server

# 编译选项
DEFINES += QT_DEPRECATED_WARNINGS

# 输出目录
DESTDIR = $$PWD/bin

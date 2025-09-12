# 公共日志模块配置文件
# 用于客户端和服务端项目

# 包含路径
INCLUDEPATH += $$PWD

# 头文件
HEADERS += \
    $$PWD/base/logger_base.h \
    $$PWD/managers/log_manager.h \
    $$PWD/config/log_config.h

# 源文件
SOURCES += \
    $$PWD/base/logger_base.cpp \
    $$PWD/managers/log_manager.cpp \
    $$PWD/config/log_config.cpp

# 依赖的Qt模块
QT += core

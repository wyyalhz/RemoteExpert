# ===============================================
# common/protocol.pri
# 协议模块项目文件
# ===============================================

INCLUDEPATH += $$PWD

# 类型定义层
HEADERS += $$PWD/types/enums.h \
           $$PWD/types/constants.h \
           $$PWD/types/validation_rules.h

# 序列化层
SOURCES += $$PWD/serialization/serializer.cpp
HEADERS += $$PWD/serialization/packet.h \
           $$PWD/serialization/serializer.h


# 构建器层
SOURCES += $$PWD/builders/message_builder.cpp
HEADERS += $$PWD/builders/message_builder.h

# 验证器层
SOURCES += $$PWD/validators/message_validator.cpp
HEADERS += $$PWD/validators/message_validator.h

# 解析器层
SOURCES += $$PWD/parsers/message_parser.cpp
HEADERS += $$PWD/parsers/message_parser.h

# 主入口
HEADERS += $$PWD/protocol.h

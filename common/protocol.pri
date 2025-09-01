# ===============================================
# common/protocol.pri
# 重构后的协议模块项目文件
# ===============================================

INCLUDEPATH += $$PWD

# 类型定义层
HEADERS += $$PWD/protocol/types/enums.h \
           $$PWD/protocol/types/constants.h \
           $$PWD/protocol/types/validation_rules.h

# 序列化层
SOURCES += $$PWD/protocol/serialization/serializer.cpp
HEADERS += $$PWD/protocol/serialization/packet.h \
           $$PWD/protocol/serialization/serializer.h


# 构建器层
SOURCES += $$PWD/protocol/builders/message_builder.cpp
HEADERS += $$PWD/protocol/builders/message_builder.h

# 验证器层
SOURCES += $$PWD/protocol/validators/message_validator.cpp
HEADERS += $$PWD/protocol/validators/message_validator.h

# 解析器层
SOURCES += $$PWD/protocol/parsers/message_parser.cpp
HEADERS += $$PWD/protocol/parsers/message_parser.h

# 主入口
HEADERS += $$PWD/protocol/protocol.h

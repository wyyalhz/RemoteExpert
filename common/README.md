# 协议模块重构说明

## 概述

协议模块已按照协议处理步骤进行了重构，提高了代码的可维护性和模块化程度。

## 目录结构

```
common/
├── protocol/
│   ├── types/                  # 类型定义层
│   │   ├── enums.h            # 所有枚举定义
│   │   ├── constants.h        # 常量定义
│   │   └── validation_rules.h # 验证规则
│   │
│   ├── serialization/         # 序列化层
│   │   ├── packet.h          # 数据包定义
│   │   ├── packet.cpp        # 数据包实现
│   │   ├── serializer.h      # 序列化工具
│   │   └── serializer.cpp    # 序列化实现
│   │
│   ├── builders/             # 消息构建层
│   │   ├── message_builder.h # 消息构建器
│   │   └── message_builder.cpp
│   │
│   ├── validators/           # 消息验证层
│   │   ├── message_validator.h # 消息验证器
│   │   └── message_validator.cpp
│   │
│   ├── parsers/              # 消息解析层
│   │   ├── message_parser.h  # 消息解析器
│   │   └── message_parser.cpp
│   │
│   └── protocol.h            # 主入口头文件
│
├── protocol.pri              # 项目文件
└── README.md                 # 本文件
```

## 分层设计

### 1. 类型定义层 (`types/`)
- **enums.h**: 集中管理所有枚举（消息类型、用户类型、工单状态等）
- **constants.h**: 定义所有常量（长度限制、超时时间等）
- **validation_rules.h**: 定义验证规则

### 2. 序列化层 (`serialization/`)
- **packet.h/cpp**: 数据包结构定义
- **serializer.h/cpp**: 序列化/反序列化工具

### 3. 消息定义层
消息定义通过以下方式实现：
- **types/enums.h**: 定义所有消息类型枚举
- **builders/message_builder.h**: 定义消息JSON结构
- **validators/message_validator.h**: 定义消息验证规则
- **serialization/packet.h**: 定义数据包结构

### 4. 工具层 (`builders/`, `validators/`, `parsers/`)
- **builders**: 消息构建工具
- **validators**: 消息验证工具
- **parsers**: 消息解析工具

## 使用方法

### 在项目中使用

1. 在 `.pro` 文件中包含协议模块：
```qmake
include(../common/protocol.pri)
```

2. 在代码中包含主头文件：
```cpp
#include "../common/protocol/protocol.h"
```

### 主要接口

- **Packet**: 数据包结构
- **MessageBuilder**: 消息构建工具
- **MessageValidator**: 消息验证工具
- **MessageParser**: 消息解析工具

## 向后兼容性

重构后的协议模块保持了与原有代码的完全兼容性，所有原有的接口和功能都保持不变。

## 重构优势

1. **按处理步骤分层**: 类型定义 → 消息定义 → 序列化 → 工具层
2. **避免过度拆分**: 不按业务功能拆分，保持合理的文件粒度
3. **保持功能完整性**: 所有相关功能集中管理，便于查找和修改
4. **便于团队协作**: 清晰的职责分工，减少代码冲突

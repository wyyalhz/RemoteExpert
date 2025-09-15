TEMPLATE = subdirs

# 设置项目名称
TARGET = RemoteExpert

# 定义子项目
SUBDIRS = \
    client \
    server \
    videoplusplusplus

# 设置构建顺序（可选）
# client.depends = server
# videoplusplusplus.depends = client

# 项目配置 - 使用并行构建以提高效率
# CONFIG += ordered  # 注释掉顺序构建，启用并行构建

# 设置默认构建配置
CONFIG(debug, debug|release) {
    message("Building in debug mode")
} else {
    message("Building in release mode")
}

# 注意：DESTDIR在subdirs模板中不会传递给子项目
# 每个子项目都有自己的DESTDIR设置

# 添加一些通用的编译选项
QMAKE_CXXFLAGS += -Wall -Wextra

# RemoteExpert 远程专家支持系统

## 功能特性

* **智能工单管理** - 支持工单创建、分配、状态跟踪和生命周期管理，提供完整的工单处理流程
* **实时音视频通信** - 基于TCP协议的高质量音视频传输，支持屏幕共享和实时协作
* **多用户角色支持** - 区分普通用户和专家用户，提供差异化的功能权限
* **设备数据监控** - 实时采集和传输设备状态数据，支持远程设备诊断
* **文件传输功能** - 支持文件上传下载，便于技术文档和资料共享
* **聊天通信系统** - 实时文本消息、截图分享、系统通知等多样化沟通方式
* **跨平台兼容** - 支持Windows和Linux平台，提供统一的用户体验
* **模块化架构** - 采用分层架构设计，代码结构清晰，易于维护和扩展

## 目录

```
RemoteExpert/
├── client/                 # 客户端应用程序
│   ├── src/
│   │   ├── presentation/   # 表现层(UI界面)
│   │   ├── business/       # 业务逻辑层
│   │   ├── network/        # 网络通信层
│   │   └── resources/      # 资源文件
│   └── client.pro          # 客户端构建配置
├── server/                 # 服务端应用程序
│   ├── src/
│   │   ├── data/          # 数据访问层
│   │   ├── business/      # 业务逻辑层
│   │   └── network/       # 网络服务层
│   └── server.pro         # 服务端构建配置
├── common/                 # 公共模块
│   ├── protocol/          # 网络协议定义
│   └── logging/           # 日志系统
├── videoplusplusplus/     # 音视频处理模块
└── docs/                  # 项目文档
```

## 技术栈及开发环境

使用C++ 17 + Qt 5.12.8开发，使用qmake构建项目。开发过程中，项目组分别在Windows 11和VMware 16.0上运行的Ubuntu 18.04虚拟机上使用QtCreator和Cursor分别完成前端和后端开发。测试环节中，项目组在Windows 11和VMware 16.0上运行的Ubuntu 18.04虚拟机上进行了跑通测试，除了UI样式上的细微差别，基本功能均能正常实现。

### 核心技术组件
- **网络通信**: 基于TCP的自定义二进制协议，支持消息路由和实时数据传输
- **数据库**: SQLite数据库，提供用户管理、工单存储等数据持久化
- **音视频处理**: 集成摄像头捕获、音频录制、屏幕共享等多媒体功能
- **日志系统**: 统一的日志管理，支持多级别日志记录和文件输出

## 使用

### 通过发行版（Release）

**Windows请下载**
- [Windows客户端](https://github.com/your-repo/RemoteExpert/releases/latest/download/client-windows.exe)
- [Windows服务端](https://github.com/your-repo/RemoteExpert/releases/latest/download/server-windows.exe)

**Linux请下载**
- [Linux客户端](https://github.com/your-repo/RemoteExpert/releases/latest/download/client-linux)
- [Linux服务端](https://github.com/your-repo/RemoteExpert/releases/latest/download/server-linux)

### 启动步骤
1. 先启动服务端 `server.exe`（Windows）或 `./server`（Linux）
2. 再启动客户端 `client.exe`（Windows）或 `./client`（Linux）
3. 在客户端界面进行用户注册或登录
4. 创建工单或加入现有工单开始远程协作

## 贡献

假定你已经有了完整的C++ 17或更高版本的环境。

### 获取源码
```bash
# 拉取本项目源码仓库
git clone https://github.com/your-repo/RemoteExpert.git
cd RemoteExpert
```

### 开发规范
- 详细开发文档请参考 [GitHub Docs](https://github.com/your-repo/RemoteExpert/wiki)

### 许可证

所有代码使用[MIT](LICENSE)许可证。

## 鸣谢

感谢3组同学的相互陪伴、共同努力。

---

更多技术细节和API文档请访问：[项目Wiki](https://github.com/your-repo/RemoteExpert/wiki) | [开发博客](https://your-blog.com)

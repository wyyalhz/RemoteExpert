#include "roomhub.h"
#include "databasemanager/databasemanager.h"

RoomHub::RoomHub(QObject* parent) : QObject(parent), dbManager_(new DatabaseManager(this)) {}

RoomHub::~RoomHub(){}

// 实现监听功能
bool RoomHub::startListening(const QHostAddress &address, quint16 port)
{
    return server_.listen(address, port);
}

// 实现错误信息获取
QString RoomHub::lastError() const
{
    return server_.errorString();
}

QHostAddress RoomHub::serverAddress() const
{
    return server_.serverAddress();
}

// 启动服务器，开始监听指定端口port
bool RoomHub::start(quint16 port)
{
    if(!dbManager_->initialize())
    {
        qCritical()<<"Failed to initialize database!";
        return false;
    }

    // 当有新连接时，调用onNewConnection处理
    connect(&server_, &QTcpServer::newConnection, this, &RoomHub::onNewConnection);

    // QHostAddress::Any表示监听所有可用的网络接口
    if (!server_.listen(QHostAddress::Any, port))
    {
        qWarning() << "端口" << port << "监听失败:" << server_.errorString();
        return false;
    }
    qInfo() << "服务器正在监听" << server_.serverAddress().toString() << ":" << port;
    return true;
}

// 处理新的客户端连接
void RoomHub::onNewConnection()
{
    // 循环处理所有待处理的连接（可能有多个同时到来）
    while (server_.hasPendingConnections())
    {
        // 获取下一个待处理的连接
        QTcpSocket* sock = server_.nextPendingConnection();

        // 创建客户端上下文对象，存储客户端相关信息
        auto* ctx = new ClientCtx;
        ctx->sock = sock;  // 关联客户端套接字

        // 将客户端添加到客户端映射表中（套接字->上下文）
        clients_.insert(sock, ctx);

        // 输出新客户端连接信息（IP地址和端口）
        qInfo() << "新客户端连接来自" << sock->peerAddress().toString() << sock->peerPort();

        // 当客户端有数据可读时，调用onReadyRead处理
        connect(sock, &QTcpSocket::readyRead, this, &RoomHub::onReadyRead);
        // 当客户端断开连接时，调用onDisconnected处理
        connect(sock, &QTcpSocket::disconnected, this, &RoomHub::onDisconnected);
    }
}

// 处理客户端断开连接
void RoomHub::onDisconnected()
{
    // 获取发送信号的套接字（即断开连接的客户端）
    auto* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock) return;  // 如果不是QTcpSocket类型，直接返回

    // 在客户端映射表中查找该客户端
    auto it = clients_.find(sock);
    if (it == clients_.end()) return;  // 未找到客户端，直接返回

    ClientCtx* c = it.value();  // 获取客户端上下文

    // 如果客户端属于某个房间，从房间中移除
    if (!c->roomId.isEmpty()) {
        // 查找该房间的所有客户端
        auto range = rooms_.equal_range(c->roomId);
        // 遍历房间中的客户端，移除当前断开连接的客户端
        for (auto i = range.first; i != range.second; ) {
            if (i.value() == sock) {
                i = rooms_.erase(i);  // 从房间中删除
            } else {
                ++i;  // 继续下一个
            }
        }
    }

    // 输出客户端断开连接的信息
    if(!c->user.isEmpty())
    {
        qInfo()<<"Client disconnected - User:"<<c->user<<"Room:"<<c->roomId;
    }
    else
    {
        qInfo()<<"Client disconnected - Unauthenticated user from"<<sock->peerAddress().toString();
    }

    // 从客户端映射表中移除
    clients_.erase(it);

    buffers_.remove(sock);

    // 安排套接字在适当的时候删除
    sock->deleteLater();
    // 删除客户端上下文对象
    delete c;
}

// 处理客户端发送的数据
void RoomHub::onReadyRead()
{
    // 获取发送数据的客户端套接字
    auto* sock = qobject_cast<QTcpSocket*>(sender());
    if (!sock) return;  // 无效的套接字，直接返回

    // 查找客户端上下文
    auto it = clients_.find(sock);
    if (it == clients_.end()) return;  // 未找到客户端，直接返回

    ClientCtx* c = it.value();  // 获取客户端上下文

    //为每个套接字维护一个接收缓冲区
    QByteArray& buf = buffers_[sock];  // 获取当前客户端的缓冲区
    QByteArray newData = sock->readAll();  // 读取新收到的数据
        if (!newData.isEmpty()) {  // 如果有新数据
            // 打印：客户端IP、收到的字节数、前20字节（十六进制，方便核对）
            qInfo() << "[TCP接收] 客户端" << sock->peerAddress().toString()
                     << "收到" << newData.size() << "字节，前20字节：" << newData.left(20).toHex();
            // 打印当前缓冲区总长度（判断是否数据不完整）
            buf.append(newData);  // 将新数据追加到缓冲区
            qInfo() << "[TCP接收] 当前缓冲区总长度：" << buf.size() << "字节";
        }

    // 解析缓冲区中的数据包
    QVector<Packet> pkts;
    // 从缓冲区中提取完整的数据包
    if (drainPackets(buf, pkts)) {
        // 处理每个提取到的数据包
        for (const Packet& p : pkts) {
            handlePacket(c, p);
        }
    }
    else {
           // 新增日志：未解析出完整数据包时的提示
           if (buf.size() > 0) {
               qInfo() << "[TCP解析] 未解析出完整数据包，当前缓冲区长度：" << buf.size() << "字节";
           }
    }
}

// 处理解析后的数据包
// c: 客户端上下文
// p: 要处理的数据包
void RoomHub::handlePacket(ClientCtx* c, const Packet& p)
{
    if(p.type == MSG_REGISTER)
    {
        QString username = p.json.value("username").toString();
        QString password = p.json.value("password").toString();
        QString email = p.json.value("email").toString();
        QString phone = p.json.value("phone").toString();
        int userType = p.json.value("user_type").toInt();

        if(username.isEmpty()||password.isEmpty()||userType<=0)
        {
            QJsonObject resp
            {
                {"code",400},
                {"message","Invalid parameters: username/password/user_type cannot be empty"}
            };
            c->sock->write(buildPacket(MSG_SERVER_EVENT,resp));
            return;
        }

        bool registerSuccess = dbManager_->userManager()->registerUser(username,password,email,phone,userType);
        if(registerSuccess)
        {
            QJsonObject resp
            {
                {"code",0},
                {"message","User registered successfully"},
                {"username",username}
            };
            c->sock->write(buildPacket(MSG_SERVER_EVENT,resp));
        }
        else
        {
            QJsonObject resp
            {
                {"code",409},
                {"message","Username already exists"}
            };
            c->sock->write(buildPacket(MSG_SERVER_EVENT,resp));
        }
        return;
    }

    if(p.type == MSG_LOGIN)
    {
        //如果已经登录，则拒绝重复登录
        if(c->isAuthenticated)
        {
            QJsonObject response
            {
                {"code",400},
                {"message","Already logged in."}
            };
            c->sock->write(buildPacket(MSG_SERVER_EVENT,response));
            return;
        }

    //解析登录数据
    QString username=p.json.value("username").toString();
    QString password=p.json.value("password").toString();
    int userType = p.json.value("user_type").toInt();

    if(//username == "factory" && password =="123456"
            dbManager_->userManager()->validateUser(username,password,userType))
    {
        c->user =username;
        c->isAuthenticated =true;
        QJsonObject response
        {
            {"code", 0},
            {"message", "Login successful."},
            {"username", username} // 可以返回角色信息供客户端使用
        };
        c->sock->write(buildPacket(MSG_SERVER_EVENT,response));
        qInfo()<<"User logged in:"<<username<<"from"<<c->sock->peerAddress();
    }
//    else if(username == "expert"&&password == "123456")
//    {
//        c->user =username;
//        c->isAuthenticated = true;
//        QJsonObject response
//        {
//            {"code", 0},
//            {"message", "Login successful."},
//            {"role", "expert"}
//        };
//        c->sock->write(buildPacket(MSG_SERVER_EVENT, response));
//        qInfo() << "User logged in:" << username << "from" << c->sock->peerAddress();
//    }
    else
    {
        // 失败
        QJsonObject response
        {
            {"code", 401},
            {"message", "Invalid username or password."}
        };
        c->sock->write(buildPacket(MSG_SERVER_EVENT, response));
        qInfo() << "Login failed for user:" << username << "from" << c->sock->peerAddress();
    }
    return;
    }

    //安全检查 - 后续所有请求都必须先登录
    if (!c->isAuthenticated)
    {
        QJsonObject response
        {
            {"code", 403},
            {"message", "Authentication required. Please login first."}
        };
            c->sock->write(buildPacket(MSG_SERVER_EVENT, response));
            return; // 未登录用户无法进行任何其他操作
    }
    // 处理其他已认证的请求
    // 处理加入房间的请求
    if (p.type == MSG_JOIN_WORKORDER)
    {
        // 从数据包中解析房间ID和用户名
        const QString roomId = p.json.value("roomId").toString();

        // 检查房间ID是否为空
        if (roomId.isEmpty()) {
            // 构造错误响应：缺少roomId
            QJsonObject j{{"code",400},{"message","需要roomId"}};
            // 发送响应给客户端
            c->sock->write(buildPacket(MSG_SERVER_EVENT, j));
            return;
        }

        // 保存用户名
        QString user = c->user;
        if(user.isEmpty())
        {
            user = "anonymous";
        }

        // 加入指定房间
        joinRoom(c, roomId);

        // 构造成功响应
        QJsonObject j{{"code",0},{"message","已加入"},{"roomId",roomId}};
        c->sock->write(buildPacket(MSG_SERVER_EVENT, j));
        return;
    }

    // 检查客户端是否已加入房间，未加入则拒绝后续操作
    if (c->roomId.isEmpty()) {
        QJsonObject j{{"code",403},{"message","请先加入一个房间"}};
        c->sock->write(buildPacket(MSG_SERVER_EVENT, j));
        return;
    }

    // 处理各种类型的消息，转发到同一房间的其他客户端
    if (p.type == MSG_TEXT || p.type == MSG_DEVICE_DATA ||
        p.type == MSG_VIDEO_FRAME || p.type == MSG_AUDIO_FRAME ||
        p.type == MSG_CONTROL) {
        // 构建原始数据包（保持原样，服务端不修改内容）
        QByteArray raw = buildPacket(p.type, p.json, p.bin);
        // 广播到房间内其他客户端（排除发送者自己）
        broadcastToRoom(c->roomId, raw, c->sock);
        return;
    }

    // 处理未识别的消息类型
    QJsonObject j{{"code",404},{"message",QString("未知消息类型 %1").arg(p.type)}};
    c->sock->write(buildPacket(MSG_SERVER_EVENT, j));
}

// 让客户端加入指定房间
// c: 客户端上下文
// roomId: 要加入的房间ID
void RoomHub::joinRoom(ClientCtx* c, const QString& roomId) {
    qInfo() << "[joinRoom] 进入函数，原始c->roomId=" << c->roomId << "，目标roomId=" << roomId;
    // 如果客户端已在其他房间，先从原房间移除
    if (!c->roomId.isEmpty()) {
        auto range = rooms_.equal_range(c->roomId);
        for (auto i = range.first; i != range.second; ) {
            if (i.value() == c->sock) {
                i = rooms_.erase(i);  // 从原房间删除
            } else {
                ++i;
            }
        }
    }

    // 更新客户端的房间ID
    c->roomId = roomId;
    qInfo() << "[joinRoom] 已设置c->roomId=" << c->roomId << "（赋值后检查）";
    // 将客户端添加到新房间
    rooms_.insert(roomId, c->sock);
    qInfo() << "客户端已添加到新房间" << roomId << "，房间当前客户端数：" << rooms_.count(roomId);
}

// 向房间内其他客户端广播数据包
// roomId: 房间ID
// packet: 要广播的数据包
// except: 不需要接收广播的客户端（通常是发送者自己）
void RoomHub::broadcastToRoom(const QString& roomId, const QByteArray& packet, QTcpSocket* except) {
    // 查找该房间的所有客户端
    auto range = rooms_.equal_range(roomId);
    // 遍历所有客户端并发送数据包
    for (auto i = range.first; i != range.second; ++i) {
        QTcpSocket* s = i.value();
        if (s == except) continue;  // 跳过不需要接收的客户端
        s->write(packet);  // 发送数据包
    }
}

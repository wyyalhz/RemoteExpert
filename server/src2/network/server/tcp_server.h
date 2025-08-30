#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QHostAddress>
#include <QString>

class ConnectionManager;

// TCP服务器类 - 负责监听端口和接受新连接
class TCPServer : public QObject
{
    Q_OBJECT
public:
    explicit TCPServer(QObject *parent = nullptr);
    ~TCPServer();

    // 启动服务器
    bool start(quint16 port);
    bool start(const QHostAddress &address, quint16 port);
    
    // 停止服务器
    void stop();
    
    // 获取服务器状态
    bool isListening() const;
    QString lastError() const;
    QHostAddress serverAddress() const;
    quint16 serverPort() const;
    
    // 设置连接管理器
    void setConnectionManager(ConnectionManager* manager);

private slots:
    void onNewConnection();

private:
    QTcpServer server_;
    ConnectionManager* connectionManager_;
    
    void setupConnections();
};

#endif // TCP_SERVER_H

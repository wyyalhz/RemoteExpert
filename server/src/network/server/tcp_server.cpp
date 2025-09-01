#include "tcp_server.h"
#include "../connection_manager.h"
#include "../logging/network_logger.h"

TCPServer::TCPServer(QObject *parent)
    : QObject(parent)
    , connectionManager_(nullptr)
{
    setupConnections();
}

TCPServer::~TCPServer()
{
    stop();
}

bool TCPServer::start(quint16 port)
{
    return start(QHostAddress::Any, port);
}

bool TCPServer::start(const QHostAddress &address, quint16 port)
{
    if (server_.isListening()) {
        NetworkLogger::warning("TCP Server", "Server is already listening");
        return true;
    }

    if (!server_.listen(address, port)) {
        NetworkLogger::serverError(QString("Failed to start server on %1:%2 - %3")
                                  .arg(address.toString()).arg(port).arg(server_.errorString()));
        return false;
    }

    NetworkLogger::serverStarted(server_.serverAddress(), server_.serverPort());
    
    return true;
}

void TCPServer::stop()
{
    if (server_.isListening()) {
        server_.close();
        NetworkLogger::serverStopped();
    }
}

bool TCPServer::isListening() const
{
    return server_.isListening();
}

QString TCPServer::lastError() const
{
    return server_.errorString();
}

QHostAddress TCPServer::serverAddress() const
{
    return server_.serverAddress();
}

quint16 TCPServer::serverPort() const
{
    return server_.serverPort();
}

void TCPServer::setConnectionManager(ConnectionManager* manager)
{
    connectionManager_ = manager;
}

void TCPServer::setupConnections()
{
    connect(&server_, &QTcpServer::newConnection, 
            this, &TCPServer::onNewConnection);
}

void TCPServer::onNewConnection()
{
    while (server_.hasPendingConnections()) {
        QTcpSocket* socket = server_.nextPendingConnection();
        
        if (connectionManager_) {
            connectionManager_->addConnection(socket);
        } else {
            NetworkLogger::error("TCP Server", "Connection manager not set, cannot handle new connection");
            socket->close();
            socket->deleteLater();
        }
    }
}

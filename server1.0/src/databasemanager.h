#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonArray>

class DatabaseManager:public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool initialize();
    bool validateUser(const QString &username,const QString &password);

    // 工单管理相关方法
    bool createWorkOrder(const QString &title, const QString &description,
                        int creatorId, const QString &priority, const QString &category, QString &generatedTicketId);
    bool joinWorkOrder(int workOrderId, int userId, const QString &role);
    bool leaveWorkOrder(int workOrderId, int userId);
    QJsonArray getWorkOrderParticipants(int workOrderId);
    bool updateWorkOrderStatus(int workOrderId, const QString &status);
    bool assignWorkOrder(int workOrderId, int expertId);
    QJsonObject getWorkOrderInfo(int workOrderId);
    bool closeWorkOrder(int workOrderId, int userId);  // 关闭工单（仅创建者可关闭）

private:
    QSqlDatabase db_;
};

#endif // DATABASEMANAGER_H

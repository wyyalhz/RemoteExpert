#include <QtSql>
#include "ticketschema.h"
#include "homemainwindow.h"
#include "logindialog.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Ensure SQLite DB and ticket schema
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid()) db = QSqlDatabase::addDatabase("QSQLITE");
    if (!db.isOpen()) { db.setDatabaseName("app.db"); db.open(); }
    ensureTicketSchema();

    LoginDialog lg;
    if (lg.exec() == QDialog::Accepted){
        HomeMainWindow *home = new HomeMainWindow(lg.getCurrentUser(), lg.getCurrentUserType());
        // 如果需要把登录信息带给主页：
        home->show();
        return a.exec();
    } else {
        // 用户取消/失败
        return 0;
    }
}

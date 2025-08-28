#include "logindialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 初始化数据库
    DatabaseManager::instance().initDatabase();

    LoginDialog login;
    if(login.exec() == QDialog::Accepted){
        // 登录成功，进入主界面
        LoginDialog w;
        w.show();
        return a.exec();
    }

    return 0;
}

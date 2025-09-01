#include <QtSql>
#include "Presentation/utils/ticket_schema.h"
#include "Presentation/main_window/home_main_window.h"
#include "Presentation/dialogs/LoginDialog/login_dialog.h"
#include "Presentation/utils/theme.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QApplication::setStyle("fusion"); // 建议，基底更统一


    // Ensure SQLite DB and ticket schema
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid()) db = QSqlDatabase::addDatabase("QSQLITE");
    if (!db.isOpen()) { db.setDatabaseName("app.db"); db.open(); }
    ensureTicketSchema();

    LoginDialog lg;

    if (lg.exec() == QDialog::Accepted){
        HomeMainWindow *home = new HomeMainWindow(lg.getCurrentUser(), lg.getCurrentUserType());
        // 如果需要把登录信息带给主页：

        QSettings st("YourOrg","YourApp");
        const QString t = st.value("theme","light").toString();
        applyTheme(t == "dark" ? Theme::Dark : Theme::Light);


        home->show();
        return a.exec();
    } else {
        // 用户取消/失败
        return 0;
    }
}

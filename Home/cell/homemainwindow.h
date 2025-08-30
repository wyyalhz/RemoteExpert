#ifndef HOMEMAINWINDOW_H
#define HOMEMAINWINDOW_H

#include <QMainWindow>

#include "thankswidget.h"
#include "tickets/ticketpage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class HomeMainWindow; }
QT_END_NAMESPACE

class TicketPage;
class HomeMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    HomeMainWindow(const QString& currentUser, int currentUserType, QWidget *parent = nullptr);
    ~HomeMainWindow();

    void dealMenu();//

private:
    TicketPage *ticketPage = nullptr;

    //加载那几个切换页面
    void initPage();

private:
    Ui::HomeMainWindow *ui;

    ThanksWidget *m_thanksPage;
    TicketPage *m_ticketPage;

    QString currentUser;
    int currentUserType;
};
#endif // HOMEMAINWINDOW_H

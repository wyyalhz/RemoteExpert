#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class SettingPage; }
QT_END_NAMESPACE

class SettingPage : public QWidget
{
    Q_OBJECT
public:
    explicit SettingPage(QWidget *parent = nullptr);
    ~SettingPage();

private:
    Ui::SettingPage *ui;
    void initThemeState();    // 从当前主题初始化复选框
    void connectSignals();    // 连接信号槽
};

#endif // SETTINGPAGE_H

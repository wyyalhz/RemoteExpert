#include "setting_page.h"
#include "ui_setting_page.h"
#include "../../utils/theme.h"
#include <QSettings>

SettingPage::SettingPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPage)
{
    ui->setupUi(this);
    // 利用全局QSS里对 QLabel#titleLabel 的定义
    ui->titleLabel->setObjectName("titleLabel");

    initThemeState();
    connectSignals();
}

SettingPage::~SettingPage()
{
    delete ui;
}

void SettingPage::initThemeState()
{
    ui->chkDarkMode->setChecked(currentThemeName() == "dark");
}

void SettingPage::connectSignals()
{
    connect(ui->chkDarkMode, &QCheckBox::toggled, this, [](bool on){
        applyTheme(on ? Theme::Dark : Theme::Light);
        // applyTheme 内部已写入 QSettings 并刷新图标资源
    });
}

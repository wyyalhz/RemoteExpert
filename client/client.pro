QT += core gui sql widgets svg network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 包含协议模块
include(../common/protocol.pri)

# 定义输出目录
DESTDIR = $$PWD/../bin

# 源文件
SOURCES += \
    # 主程序入口
    main.cpp \
    # Presentation层 - 对话框
    Presentation/dialogs/LoginDialog/login_dialog.cpp \
    Presentation/dialogs/RegisterDialog/register_dialog.cpp \
    Presentation/dialogs/TicketDialog/ticket_dialog.cpp \
    Presentation/dialogs/TicketDialogDetail/ticket_dialog_detail.cpp \
    Presentation/dialogs/AddTicket/add_ticket.cpp \
    # Presentation层 - 页面
    Presentation/pages/TicketPage/ticket_page.cpp \
    Presentation/pages/ThanksPage/thanks_page.cpp \
    Presentation/pages/SettingPage/setting_page.cpp \
    # Presentation层 - 主窗口
    Presentation/main_window/home_main_window.cpp \
    # Presentation层 - 工具类
    Presentation/utils/theme.cpp \
    Presentation/utils/ticket_schema.cpp \
    # Business层 - 管理器
    Business/managers/database_manager.cpp

# 头文件
HEADERS += \
    # Presentation层 - 对话框
    Presentation/dialogs/LoginDialog/login_dialog.h \
    Presentation/dialogs/RegisterDialog/register_dialog.h \
    Presentation/dialogs/TicketDialog/ticket_dialog.h \
    Presentation/dialogs/TicketDialogDetail/ticket_dialog_detail.h \
    Presentation/dialogs/AddTicket/add_ticket.h \
    # Presentation层 - 页面
    Presentation/pages/TicketPage/ticket_page.h \
    Presentation/pages/ThanksPage/thanks_page.h \
    Presentation/pages/SettingPage/setting_page.h \
    # Presentation层 - 主窗口
    Presentation/main_window/home_main_window.h \
    # Presentation层 - 工具类
    Presentation/utils/theme.h \
    Presentation/utils/ticket_schema.h \
    # Business层 - 管理器
    Business/managers/database_manager.h

# UI文件
FORMS += \
    Presentation/dialogs/LoginDialog/login_dialog.ui \
    Presentation/dialogs/RegisterDialog/register_dialog.ui \
    Presentation/dialogs/TicketDialog/ticket_dialog.ui \
    Presentation/dialogs/TicketDialogDetail/ticket_dialog_detail.ui \
    Presentation/dialogs/AddTicket/add_ticket.ui \
    Presentation/pages/TicketPage/ticket_page.ui \
    Presentation/pages/ThanksPage/thanks_page.ui \
    Presentation/pages/SettingPage/setting_page.ui \
    Presentation/main_window/home_main_window.ui

# 资源文件
RESOURCES += \
    Resources/images/UiTool.qrc \
    Resources/images/icons_dark.qrc \
    Resources/images/icons_light.qrc \
    Resources/styles/themes/themes.qrc

# 包含路径
INCLUDEPATH += \
    . \
    Presentation \
    Presentation/dialogs \
    Presentation/pages \
    Presentation/main_window \
    Presentation/utils \
    Business \
    Business/managers \
    Business/models \
    Business/services \
    Network \
    Network/client \
    Network/connection \
    Network/protocol \
    Network/protocol/handlers \
    Network/status \
    Logger \
    Resources

# 编译警告设置
DEFINES += QT_DEPRECATED_WARNINGS

# 开发/运行时：把外部 themes 目录复制到可执行文件旁，方便覆盖qss
win32 {
    QMAKE_POST_LINK += xcopy /E /I /Y \"$$PWD\\Resources\\styles\\themes\" \"$$OUT_PWD\\themes\" & echo.
}
unix:!macx {
    QMAKE_POST_LINK += mkdir -p \"$$OUT_PWD/themes\"; cp -R \"$$PWD/Resources/styles/themes/.\" \"$$OUT_PWD/themes/\"
}
macx {
    QMAKE_POST_LINK += mkdir -p \"$$OUT_PWD/themes\"; cp -R \"$$PWD/Resources/styles/themes/.\" \"$$OUT_PWD/themes/\"
}

# make install 时把themes安装到可执行旁
themes.files = $$files($$PWD/Resources/styles/themes/*)
themes.path  = $$DESTDIR/themes
INSTALLS    += themes

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

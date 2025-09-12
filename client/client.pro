QT += core gui sql widgets svg network multimedia multimediawidgets serialport charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TEMPLATE = app

TARGET = client

DESTDIR = $$PWD/bin

# 编译警告设置
DEFINES += QT_DEPRECATED_WARNINGS

# 协议模块
include(../common/protocol/protocol.pri)

# 公共日志模块
include(../common/logging/logging.pri)

# 源文件
SOURCES += \
    # 主程序入口
    src/main.cpp \
    # 表示层
    src/presentation/dialogs/equipment_dialog/data_model.cpp \
    src/presentation/dialogs/equipment_dialog/equipment_show.cpp \
    src/presentation/dialogs/login_dialog/login_dialog.cpp \
    src/presentation/dialogs/register_dialog/register_dialog.cpp \
    src/presentation/dialogs/ticket_dialog/ticket_dialog.cpp \
    src/presentation/dialogs/ticket_dialog_detail/ticket_dialog_detail.cpp \
    src/presentation/dialogs/add_ticket/add_ticket.cpp \
    src/presentation/pages/ticket_page/ticket_page.cpp \
    src/presentation/pages/thanks_page/thanks_page.cpp \
    src/presentation/pages/setting_page/setting_page.cpp \
    src/presentation/pages/log_page/audio_pipe.cpp \
    src/presentation/pages/log_page/log_main_window.cpp \
    src/presentation/pages/log_page/mtr_reader.cpp \
    src/presentation/main_window/home_main_window.cpp \
    src/presentation/utils/theme.cpp \
    src/presentation/utils/ticket_schema.cpp \
    # 业务逻辑层
    src/business/models/user.cpp \
    src/business/models/ticket.cpp \
    src/business/services/auth_service.cpp \
    src/business/services/ticket_service.cpp \
    src/business/managers/database_manager.cpp \
    src/business/managers/session_manager.cpp \
    # 网络层
    src/network/client/network_client.cpp \
    src/network/connection/connection_manager.cpp \
    src/network/protocol/handlers/message_handler.cpp \
    src/network/protocol/handlers/user_message_handler.cpp \
    src/network/protocol/handlers/workorder_message_handler.cpp \
    src/network/protocol/handlers/other_message_handler.cpp \
    src/network/status/network_status.cpp

# 头文件
HEADERS += \
    # 表示层
    src/presentation/dialogs/equipment_dialog/data_model.h \
    src/presentation/dialogs/equipment_dialog/equipment_show.h \
    src/presentation/dialogs/login_dialog/login_dialog.h \
    src/presentation/dialogs/register_dialog/register_dialog.h \
    src/presentation/dialogs/ticket_dialog/ticket_dialog.h \
    src/presentation/dialogs/ticket_dialog_detail/ticket_dialog_detail.h \
    src/presentation/dialogs/add_ticket/add_ticket.h \
    src/presentation/pages/ticket_page/ticket_page.h \
    src/presentation/pages/thanks_page/thanks_page.h \
    src/presentation/pages/setting_page/setting_page.h \
    src/presentation/pages/log_page/audio_pipe.h \
    src/presentation/pages/log_page/log_main_window.h \
    src/presentation/pages/log_page/mtr_reader.h \
    src/presentation/main_window/home_main_window.h \
    src/presentation/utils/theme.h \
    src/presentation/utils/ticket_schema.h \
    # 业务逻辑层
    src/business/models/user.h \
    src/business/models/ticket.h \
    src/business/services/auth_service.h \
    src/business/services/ticket_service.h \
    src/business/managers/database_manager.h \
    src/business/managers/session_manager.h \
    # 网络层
    src/network/client/network_client.h \
    src/network/connection/connection_manager.h \
    src/network/protocol/handlers/message_handler.h \
    src/network/protocol/handlers/user_message_handler.h \
    src/network/protocol/handlers/workorder_message_handler.h \
    src/network/protocol/handlers/other_message_handler.h \
    src/network/status/network_status.h

# UI文件
FORMS += \
    src/presentation/dialogs/equipment_dialog/equipment_show.ui \
    src/presentation/dialogs/login_dialog/login_dialog.ui \
    src/presentation/dialogs/register_dialog/register_dialog.ui \
    src/presentation/dialogs/ticket_dialog/ticket_dialog.ui \
    src/presentation/dialogs/ticket_dialog_detail/ticket_dialog_detail.ui \
    src/presentation/dialogs/add_ticket/add_ticket.ui \
    src/presentation/pages/ticket_page/ticket_page.ui \
    src/presentation/pages/thanks_page/thanks_page.ui \
    src/presentation/pages/setting_page/setting_page.ui \
    src/presentation/main_window/home_main_window.ui \
    src/presentation/pages/log_page/log_main_window.ui

# 资源文件
RESOURCES += \
    src/resources/images/UiTool.qrc \
    src/resources/images/icons_dark.qrc \
    src/resources/images/icons_light.qrc \
    src/resources/styles/themes/themes.qrc

# 包含路径
INCLUDEPATH += \
    . \
    src \
    src/presentation \
    src/presentation/dialogs \
    src/presentation/pages \
    src/presentation/main_window \
    src/presentation/utils \
    src/business \
    src/business/managers \
    src/business/models \
    src/business/services \
    src/network \
    src/network/client \
    src/network/connection \
    src/network/protocol \
    src/network/protocol/handlers \
    src/network/status \
    src/resources

# 开发/运行时：把外部 themes 目录复制到可执行文件旁，方便覆盖qss
win32 {
    QMAKE_POST_LINK += xcopy /E /I /Y \"$$PWD\\src\\resources\\styles\\themes\" \"$$OUT_PWD\\themes\" & echo.
}
unix:!macx {
    QMAKE_POST_LINK += mkdir -p \"$$OUT_PWD/themes\"; cp -R \"$$PWD/src/resources/styles/themes/.\" \"$$OUT_PWD/themes/\"
}
macx {
    QMAKE_POST_LINK += mkdir -p \"$$OUT_PWD/themes\"; cp -R \"$$PWD/src/resources/styles/themes/.\" \"$$OUT_PWD/themes/\"
}

# make install 时把themes安装到可执行旁
themes.files = $$files($$PWD/src/resources/styles/themes/*)
themes.path  = $$DESTDIR/themes
INSTALLS    += themes

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


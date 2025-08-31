QT       += core gui sql widgets svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = $$PWD/../bin

include($$PWD/cell/cell.pri)
include($$PWD/lib/lib.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image/UiTool.qrc \
    image/icons_dark.qrc \
    image/icons_light.qrc \
    themes/themes.qrc

# 开发/运行时：把外部 themes 目录复制到可执行文件旁，方便覆盖qss
# （三端分别处理）
win32 {
    QMAKE_POST_LINK += xcopy /E /I /Y \"$$PWD\\themes\" \"$$OUT_PWD\\themes\" & echo.
}
unix:!macx {
    QMAKE_POST_LINK += mkdir -p \"$$OUT_PWD/themes\"; cp -R \"$$PWD/themes/.\" \"$$OUT_PWD/themes/\"
}
macx {
    QMAKE_POST_LINK += mkdir -p \"$$OUT_PWD/themes\"; cp -R \"$$PWD/themes/.\" \"$$OUT_PWD/themes/\"
}

# make install 时把themes安装到可执行旁
themes.files = $$files($$PWD/themes/*)
themes.path  = $$DESTDIR/themes
INSTALLS    += themes

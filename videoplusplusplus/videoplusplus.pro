QT       += core gui  multimedia multimediawidgets

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

# 包含重构后的协议模块
include(../common/protocol.pri)

SOURCES += \
    audiocapture.cpp \
    audioplayer.cpp \
    avreceiver.cpp \
    avsender.cpp \
    chatmodel.cpp \
    main.cpp \
    mainwindow.cpp \
    screencapture.cpp \
    videorecorder.cpp \
    networkmanager.cpp \
#    ../common/protocol/serialization/serializer.cpp\
#    ../common/protocol/builders/message_builder.cpp\
#    ../common/protocol/packager/protocol_packager.cpp


HEADERS += \
    audiocapture.h \
    audioplayer.h \
    avreceiver.h \
    avsender.h \
    chatmodel.h \
    mainwindow.h \
    screencapture.h \
    videorecorder.h \
    networkmanager.h \
#    ../common/protocol/serialization/serializer.h\
#    ../common/protocol/serialization/packet.h\
#    ../common/protocol/types/constants.h\
#    ../common/protocol/builders/message_builder.h\
#    ../common/protocol/types/enums.h\
#    ../common/protocol/types/text.h\
#    ../common/protocol/types/screen.h\
#    ../common/protocol/types/audio.h\
#    ../common/protocol/packager/protocol_packager.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

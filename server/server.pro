TEMPLATE = app
TARGET = server
QT += core network gui widgets
CONFIG += c++11
CONFIG -= app_bundle
SOURCES += src/mainwindow.cpp \
            src/main.cpp \
           src/roomhub.cpp
HEADERS += src/mainwindow.h \
            src/roomhub.h
include(../common/common.pri)

FORMS   += src/mainwindow.ui

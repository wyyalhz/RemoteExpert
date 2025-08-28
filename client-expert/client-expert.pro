TEMPLATE = app
TARGET = client-expert
QT += core gui widgets network
CONFIG += c++11
SOURCES += src/main.cpp \
           src/mainwindow.cpp \
           src/clientconn.cpp
HEADERS += src/mainwindow.h \
           src/clientconn.h
FORMS   +=
include(../common/common.pri)

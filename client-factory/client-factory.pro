TEMPLATE = app
TARGET = client-factory
QT += core gui widgets network
CONFIG += c++11
SOURCES += src/main.cpp \
           src/mainwindow.cpp \
           src/clientconn.cpp
HEADERS += src/mainwindow.h \
           src/clientconn.h
FORMS   += src/mainwindow.ui
include(../common/common.pri)

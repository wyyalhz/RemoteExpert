TEMPLATE = app
TARGET = server
QT += core network sql
CONFIG += c++11 console
CONFIG -= app_bundle
SOURCES += src/main.cpp \
           src/databasemanager.cpp \
           src/roomhub.cpp
HEADERS += src/roomhub.h \
    src/databasemanager.h
include(../common/common.pri)

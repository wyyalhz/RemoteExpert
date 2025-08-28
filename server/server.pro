TEMPLATE = app
TARGET = server
QT += core network
CONFIG += c++11 console
CONFIG -= app_bundle
SOURCES += src/main.cpp \
           src/roomhub.cpp
HEADERS += src/roomhub.h
include(../common/common.pri)

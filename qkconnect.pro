#-------------------------------------------------
#
# Project created by QtCreator 2014-11-22T15:46:35
#
#-------------------------------------------------

QT       += core network serialport
QT       -= gui

TARGET = qkconnect
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += \
include \
src \
src/include

SOURCES += \
src/main.cpp \
    src/clhandler.cpp \
    src/qkconnectserver.cpp \
    src/qkconnectsocket.cpp \
    src/qkconnectclientthread.cpp \
    src/qkconnserial.cpp \
    src/qkconn.cpp \
    src/qkconnthread.cpp \
    src/qkconnloopback.cpp

HEADERS += \
    include/qkconnect_global.h \
    src/clhandler.h \
    src/qkconnectserver.h \
    src/qkconnectsocket.h \
    src/qkconnectclientthread.h \
    src/qkconnserial.h \
    src/qkconn.h \
    src/qkconnthread.h \
    src/qkconnloopback.h

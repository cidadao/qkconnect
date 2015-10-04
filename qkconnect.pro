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
src \
 ../utils

SOURCES += \
src/main.cpp \
    src/clhandler.cpp \
    src/qkconnectserver.cpp \
    src/qkconnserial.cpp \
    src/qkconn.cpp \
    src/qkconnthread.cpp \
    src/qkconnloopback.cpp \
    src/qkclientthread.cpp \
    src/qksocket.cpp \
    src/qkserver.cpp \
    src/qkspyserver.cpp \
    ../utils/qkutils.cpp \
    ../utils/qkcore.cpp \
    src/qkconnect.cpp

HEADERS += \
    src/qkconnect_global.h \
    src/clhandler.h \
    src/qkconnectserver.h \
    src/qkconnserial.h \
    src/qkconn.h \
    src/qkconnthread.h \
    src/qkconnloopback.h \
    src/qkclientthread.h \
    src/qksocket.h \
    src/qkserver.h \
    src/qkspyserver.h \
    ../utils/qkutils.h \
    ../utils/qkcore.h \
    src/qkconnect.h

CONFIG(debug, debug|release){
    DESTDIR = debug
} else {
    DESTDIR = release
}

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui

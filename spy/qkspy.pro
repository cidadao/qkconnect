#-------------------------------------------------
#
# Project created by QtCreator 2014-12-13T02:10:21
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QkSpy
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

CONFIG(debug, debug|release){
    DESTDIR = debug
} else {
    DESTDIR = release
}

OBJECTS_DIR = obj
MOC_DIR = moc
RCC_DIR = rcc
UI_DIR = ui

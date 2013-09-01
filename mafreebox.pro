#-------------------------------------------------
#
# Project created by QtCreator 2013-07-02T22:40:20
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = freebox-desktop
TEMPLATE = app



include(api/api.pri)
include(gui/common/common.pri)
include(gui/filesystem/filesystem.pri)
include(gui/download/download.pri)

SOURCES += main.cpp\
    mainapplication.cpp

HEADERS  += \
    mainapplication.h

RESOURCES += \
    assets/assets.qrc \
    assets/mime.qrc

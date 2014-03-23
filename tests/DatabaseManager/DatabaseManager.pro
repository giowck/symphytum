#-------------------------------------------------
#
# Project created by QtCreator 2012-07-15T11:41:39
#
#-------------------------------------------------

QT       += sql testlib

QT       -= gui

TARGET = tst_databasemanagertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_databasemanagertest.cpp \
    ../../components/databasemanager.cpp \
    ../../utils/definitionholder.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../components/databasemanager.h \
    ../../utils/definitionholder.h

#-------------------------------------------------
#
# Project created by QtCreator 2012-07-03T11:28:20
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_metadatapropertiesparsertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_metadatapropertiesparsertest.cpp \
    ../../utils/metadatapropertiesparser.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../utils/metadatapropertiesparser.h

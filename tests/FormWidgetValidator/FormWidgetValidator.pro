#-------------------------------------------------
#
# Project created by QtCreator 2012-07-04T11:38:55
#
#-------------------------------------------------

QT       += testlib sql

QT       -= gui

TARGET = tst_formwidgetvalidatortest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_formwidgetvalidatortest.cpp \
    ../../components/metadataengine.cpp \
    ../../components/databasemanager.cpp \
    ../../models/standardmodel.cpp \
    ../../utils/formwidgetvalidator.cpp \
    ../../utils/metadatapropertiesparser.cpp \
    ../../utils/definitionholder.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../components/metadataengine.h \
    ../../components/databasemanager.h \
    ../../models/standardmodel.h \
    ../../utils/formwidgetvalidator.h \
    ../../utils/metadatapropertiesparser.h \
    ../../utils/definitionholder.h

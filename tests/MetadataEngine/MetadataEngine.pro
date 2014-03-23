#-------------------------------------------------
#
# Project created by QtCreator 2012-06-22T13:55:34
#
#-------------------------------------------------

QT       += gui sql testlib

TARGET = tst_metadataenginetest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_metadataenginetest.cpp \
    ../../components/databasemanager.cpp \
    ../../components/metadataengine.cpp \
    ../../utils/definitionholder.cpp \
    ../../models/standardmodel.cpp \
    ../../components/filemanager.cpp \
    ../../utils/metadatapropertiesparser.cpp \
    ../../components/alarmmanager.cpp \
    ../../components/settingsmanager.cpp \
    ../../components/sync_framework/syncsession.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../components/databasemanager.h \
    ../../components/metadataengine.h \
    ../../utils/definitionholder.h \
    ../../models/standardmodel.h \
    ../../components/filemanager.h \
    ../../utils/metadatapropertiesparser.h \
    ../../components/alarmmanager.h \
    ../../components/settingsmanager.h \
    ../../components/sync_framework/syncsession.h

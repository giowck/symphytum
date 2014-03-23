#-------------------------------------------------
#
# Project created by QtCreator 2012-04-16T13:24:49
#
#-------------------------------------------------

QT       += testlib core gui

TARGET = tst_formlayoutmatrixtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_formlayoutmatrixtest.cpp \
    ../../components/formlayoutmatrix.cpp \
    ../../widgets/form_widgets/abstractformwidget.cpp \
    ../../widgets/form_widgets/testformwidget.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../components/formlayoutmatrix.h \
    ../../widgets/form_widgets/abstractformwidget.h \
    ../../widgets/form_widgets/testformwidget.h

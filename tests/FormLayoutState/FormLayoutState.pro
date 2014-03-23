#-------------------------------------------------
#
# Project created by QtCreator 2012-08-04T09:36:44
#
#-------------------------------------------------

QT       += testlib

TARGET = tst_formlayoutstatetest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_formlayoutstatetest.cpp \
    ../../utils/formviewlayoutstate.cpp \
    ../../components/formlayoutmatrix.cpp \
    ../../widgets/form_widgets/testformwidget.cpp \
    ../../widgets/form_widgets/abstractformwidget.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../utils/formviewlayoutstate.h \
    ../../components/formlayoutmatrix.h \
    ../../widgets/form_widgets/abstractformwidget.h \
    ../../widgets/form_widgets/testformwidget.h

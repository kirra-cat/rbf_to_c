#-------------------------------------------------
#
# Project created by QtCreator 2017-09-18T17:38:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = rbf_to_c
TEMPLATE = app


SOURCES += main.cpp\
        rbf_to_c.cpp

HEADERS  += rbf_to_c.h

FORMS    += rbf_to_c.ui

win32:RC_FILE = app.rc

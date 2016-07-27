QT += core
QT -= gui

TARGET = 02-dispatch
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

HEADERS += tp.h

LIBS += -ltbb -llttng-ust -ldl

include(../common.pri)

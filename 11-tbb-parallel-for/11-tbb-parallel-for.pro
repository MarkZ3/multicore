#-------------------------------------------------
#
# Project created by QtCreator 2015-09-15T22:21:21
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 11-tbb-parallel-for
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += c++14

TEMPLATE = app
LIBS += -ltbb

SOURCES += main.cpp

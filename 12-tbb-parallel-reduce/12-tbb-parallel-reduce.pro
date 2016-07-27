#-------------------------------------------------
#
# Project created by QtCreator 2015-09-16T14:27:38
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 11-tbb-parallel-reduce
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++14

TEMPLATE = app
LIBS += -ltbb


SOURCES += main.cpp

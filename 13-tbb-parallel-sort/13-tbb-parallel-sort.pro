#-------------------------------------------------
#
# Project created by QtCreator 2015-09-16T15:58:54
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 13-tbb-parallel-sort
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++11

TEMPLATE = app
LIBS += -ltbb


SOURCES += main.cpp

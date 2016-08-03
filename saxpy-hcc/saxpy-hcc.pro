#-------------------------------------------------
#
# Project created by QtCreator 2015-10-25T15:37:06
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = saxpy-hcc
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += c++14

TEMPLATE = app

QMAKE_CXX = hcc
QMAKE_LINK = hcc
QMAKE_CXXFLAGS += $$system("hcc-config --cxxflags")
QMAKE_CXXFLAGS += -Wno-unused-variable
QMAKE_LFLAGS += $$system("hcc-config --ldflags")

SOURCES += main.cpp

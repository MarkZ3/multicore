#-------------------------------------------------
#
# Project created by QtCreator 2015-10-25T15:37:06
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = saxpy
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += c++11

TEMPLATE = app

# replace with your own toolkit
LIBS += -lOpenCL -L/opt/AMDAPPSDK-3.0/lib/x86_64/
QMAKE_CXXFLAGS += -I/opt/AMDAPPSDK-3.0/include

SOURCES += main.cpp

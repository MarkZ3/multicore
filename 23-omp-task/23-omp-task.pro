#-------------------------------------------------
#
# Project created by QtCreator 2015-10-02T14:03:16
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 23-omp-task
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++11

TEMPLATE = app

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

SOURCES += main.cpp

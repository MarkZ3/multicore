#-------------------------------------------------
#
# Project created by QtCreator 2015-09-30T20:51:44
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 20-omp-speedup
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++11

TEMPLATE = app

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

SOURCES += main.cpp

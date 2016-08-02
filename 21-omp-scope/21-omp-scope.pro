#-------------------------------------------------
#
# Project created by QtCreator 2015-10-01T10:11:00
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 21-omp-scope
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++11

TEMPLATE = app

QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

SOURCES += main.cpp

#-------------------------------------------------
#
# Project created by QtCreator 2015-10-07T23:53:51
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 26-simd-autovec
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
QMAKE_CXXFLAGS += -O2 -fopenmp-simd

SOURCES += main.cpp

#-------------------------------------------------
#
# Project created by QtCreator 2015-11-11T22:50:36
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 43-bitfields
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

QMAKE_CXX = clang++
QMAKE_LINK = clang++
QMAKE_CXXFLAGS = -fsanitize=thread
QMAKE_LIBS = -fsanitize=thread

SOURCES += main.cpp

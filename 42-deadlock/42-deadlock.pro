#-------------------------------------------------
#
# Project created by QtCreator 2015-11-11T22:28:01
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 42-deadlock
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# utilisation de ThreadSanitizer
#QMAKE_CXX = clang++
#QMAKE_LINK = clang++
#QMAKE_CXXFLAGS = -fsanitize=thread
#QMAKE_LIBS = -fsanitize=thread

SOURCES += main.cpp

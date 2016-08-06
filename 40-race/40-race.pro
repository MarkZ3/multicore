QT += core
QT -= gui

TARGET = 40-race
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

# utilisation de ThreadSanitizer
QMAKE_CXX = clang++
QMAKE_LINK = clang++
QMAKE_CXXFLAGS = -fsanitize=thread
QMAKE_LIBS = -fsanitize=thread

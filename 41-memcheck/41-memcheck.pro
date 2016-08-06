QT += core
QT -= gui

TARGET = 41-memcheck
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

# utilisation de AddressSanitizer
QMAKE_CXX = clang++
QMAKE_LINK = clang++
QMAKE_CXXFLAGS = -fsanitize=address
QMAKE_LIBS = -fsanitize=address

SOURCES += main.cpp


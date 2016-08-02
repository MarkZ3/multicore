QT += core
QT -= gui

TARGET = 30-tsx
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++14

# force to use RTM instructions
QMAKE_CXXFLAGS += -mrtm

TEMPLATE = app

SOURCES += main.cpp

HEADERS += tsx-cpuid.h

QT += core
QT -= gui

TARGET = 14-tbb-parallel-scan
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app
LIBS += -ltbb

TEMPLATE = app

SOURCES += main.cpp


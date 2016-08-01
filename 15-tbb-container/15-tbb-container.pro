QT += core
QT -= gui

TARGET = 15-tbb-container
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++14

TEMPLATE = app
LIBS += -ltbb

SOURCES += main.cpp


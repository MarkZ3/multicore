QT += core
QT -= gui

TARGET = dyninst-demo
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

LIBS += -ldyninstAPI

TEMPLATE = app

SOURCES += main.cpp


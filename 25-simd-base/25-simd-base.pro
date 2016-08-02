#-------------------------------------------------
#
# Project created by QtCreator 2015-10-02T15:23:32
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = 25-simd-base
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    ops.S \
    vec.S

OTHER_FILES += \
    ops.S \
    vec.S

HEADERS += \
    ops.h \
    vec.h

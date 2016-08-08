QT       -= core
QT       -= gui

TARGET = dragonizer
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++11

TEMPLATE = app
LIBS += -ltbb -lpthread

QMAKE_CFLAGS += -fopenmp
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp

SOURCES += dragonizer.c \
    dragon.c \
    color.c \
    dragon_pthread.c \
    dragon_tbb.cpp \
    utils.c

HEADERS += color.h \
    dragon.h \
    dragon_pthread.h \
    dragon_tbb.h \
    utils.h

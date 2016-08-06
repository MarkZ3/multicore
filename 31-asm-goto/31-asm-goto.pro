QT += core
QT -= gui

TARGET = 31-asm-goto
CONFIG += console
CONFIG -= app_bundle

# the compiler must know that the static key address is constant
# with debug enabled, the compiler complains:
# warning: asm operand 0 probably doesn't match constraints
# error: impossible constraint in 'asm'
CONFIG += release

TEMPLATE = app

SOURCES += \
    main.cpp


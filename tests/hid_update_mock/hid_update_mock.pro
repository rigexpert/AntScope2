QT += core widgets network serialport bluetooth concurrent printsupport xml opengl
CONFIG += console c++17
CONFIG -= app_bundle
TEMPLATE = app

SRCDIR = ../..

INCLUDEPATH += . $$SRCDIR $$SRCDIR/analyzer $$SRCDIR/analyzer/updater $$SRCDIR/analyzer/usbhid/hidapi

DEFINES += ANTSCOPE2_UNIT_TEST
DEFINES += ANTSCOPE2VER='\\"2.0.3\\"'
DEFINES += OLD_TDR
DEFINES += NEW_CONNECTION
DEFINES += NEW_ANALYZER

SOURCES += \
    test_main.cpp \
    mock_hid.cpp \
    test_stubs.cpp \
    $$SRCDIR/analyzer/hid_analyzer.cpp \
    $$SRCDIR/analyzer/baseanalyzer.cpp \
    $$SRCDIR/AA55BTPacket.cpp \
    $$SRCDIR/crc32.cpp

HEADERS += \
    $$SRCDIR/analyzer/hid_analyzer.h \
    $$SRCDIR/analyzer/baseanalyzer.h \
    analyzerpro.h

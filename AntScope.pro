#-------------------------------------------------
#
# Project created by QtCreator 2016-12-30T15:00:50
#
#-------------------------------------------------

QT       += core gui
QT       += printsupport
QT       += serialport
QT       += network
QT       += xml
QT       += quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += ANTSCOPE2VER='\\"1.0.10\\"'

TARGET = AntScope2

TRANSLATIONS += QtLanguage.ts
TRANSLATIONS += QtLanguage_uk.ts
TRANSLATIONS += QtLanguage_ru.ts
TRANSLATIONS += QtLanguage_ja.ts
CODECFORSRC     = UTF-8

CONFIG += debug
CONFIG -= release

CONFIG(release) {
    DESTDIR = $${PWD}/build/release
}
else {
    DESTDIR = $${PWD}/build/debug
}


OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
UI_DIR = $$DESTDIR/.ui
RCC_DIR = $$DESTDIR/.rcc

message("$${TARGET}: ")
message("       ["$${DESTDIR}"]")
message("       ["$${OBJECTS_DIR}"]")
message("       ["$${MOC_DIR}"]")
message("       ["$${UI_DIR}"]")
message("       ["$${RCC_DIR}"]")

SOURCES += main.cpp\
        mainwindow.cpp \
        qcustomplot.cpp \
        analyzer/analyzer.cpp \
        analyzer/hidanalyzer.cpp \
        analyzer/comanalyzer.cpp \
        presets.cpp \
        measurements.cpp \
        analyzer/analyzerdata.cpp \
        screenshot.cpp \
        popup.cpp \
        analyzer/updater/downloader.cpp \
        settings.cpp \
        fqsettings.cpp \
        crc32.cpp \
        updatedialog.cpp \
        devinfo/redeviceinfo.cpp \
        ftdi/ftdiinfo.cpp \
        markers.cpp \
        calibration.cpp \
        print.cpp \
        export.cpp \
        markerspopup.cpp \
        updater.cpp \
        antscopeupdatedialog.cpp \
    popupindicator.cpp

HEADERS  += mainwindow.h \
        qcustomplot.h \
        analyzer/analyzer.h \
        ui_mainwindow.h \
        analyzer/hidanalyzer.h \
        analyzer/comanalyzer.h \
        analyzer/analyzerparameters.h \
        analyzer/usbhid/hidapi/hidapi.h \
        presets.h \
        measurements.h \
        analyzer/analyzerdata.h \
        screenshot.h \
        popup.h \
        analyzer/updater/downloader.h \
        settings.h \
        fqsettings.h \
        crc32.h \
        updatedialog.h \
        ui_analyzerdata.h \
        ui_fqsettings.h \
        ui_screenshot.h \
        ui_settings.h \
        ui_updatedialog.h \
        devinfo/redeviceinfo.h \
        ftdi/ftd2xx.h \
        ftdi/ftdiinfo.h \
        markers.h \
        calibration.h \
        print.h \
        export.h \
        markerspopup.h \
        updater.h \
        antscopeupdatedialog.h \
    popupindicator.h

FORMS    += mainwindow.ui \
        analyzer/analyzerdata.ui \
        screenshot.ui \
        settings.ui \
        fqsettings.ui \
        updatedialog.ui \
        print.ui \
        export.ui \
        antscopeupdatedialog.ui

win32{
    SOURCES += analyzer/usbhid/hidapi/windows/hid.c
    LIBS += -lsetupapi
    RC_ICONS += AntScope2.ico

    INCLUDEPATH +=  $$PWD/ftdi/windows
    DEPENDPATH += $$PWD/ftdi/windows
    LIBS += -L$$PWD/ftdi/windows/win32/ -lftd2xx
}

win64{
    SOURCES += analyzer/usbhid/hidapi/windows/hid.c
    LIBS += -lsetupapi
    RC_ICONS += AntScope2.ico

    INCLUDEPATH +=  $$PWD/ftdi/windows
    DEPENDPATH += $$PWD/ftdi/windows
    LIBS += -L$$PWD/ftdi/windows/win64/ -lftd2xx
    QMAKE_POST_LINK = COPY .\ftdi\windows\win64\ftd2xx.dll $$APPDIR\ftd2xx.dll &&
}
unix {
    SOURCES += analyzer/usbhid/hidapi/mac/hid.c
    LIBS += -framework CoreFoundation
    DEFINES += _NO_WINDOWS_
}

macx {
    RUNTIME_FILES.path = Contents/Resources
    RUNTIME_FILES.files = \
        $$PWD/cables.txt \
        $$PWD/itu-regions.txt \
        $$PWD/AntScope2.png
    QMAKE_BUNDLE_DATA += RUNTIME_FILES
    QMAKE_INFO_PLIST= $${PWD}/Info.plist
}



DISTFILES += \
    rig_logo.png

RESOURCES += \
    res.qrc

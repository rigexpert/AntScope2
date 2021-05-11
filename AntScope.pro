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
QT       += concurrent
QT       += opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += ANTSCOPE2VER='\\"1.2.0\\"'
DEFINES += OLD_TDR

#-------------------------------------------------
# under construction

# add connection dialog
#DEFINES += NEW_CONNECTION

# improve analyzer selection
DEFINES += NEW_ANALYZER

#-------------------------------------------------


TARGET = AntScope2


TRANSLATIONS += QtLanguage.ts
TRANSLATIONS += QtLanguage_uk.ts
TRANSLATIONS += QtLanguage_ru.ts
TRANSLATIONS += QtLanguage_ja.ts
CODECFORSRC   = UTF-8

CONFIG(release, debug|release) {
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
    popupindicator.cpp \
    analyzer/customanalyzer.cpp \
    analyzer/updater/aa30zerofirmwareupdater.cpp \
    analyzer/updater/aa230firmwareupdater.cpp \
    analyzer/updater/firmwareupdater.cpp \
    analyzer/updater/hidfirmwareupdater.cpp \
    ProgressDlg.cpp \
    iprof.cpp \
    onefqwidget.cpp \
    Notification.cpp \
    licensesdialog.cpp \
    glwidget.cpp \
    CustomPlot.cpp \
    customgraph.cpp \
    analyzer/nanovna_analyzer.cpp \
    tdrprogressdialog.cpp \
    editbandsdialog.cpp \
    AA55BTPacket.cpp \
    marqueelabel.cpp \
    selectdevicedialog.cpp \
    analyzer/baseanalyzer.cpp

HEADERS  += mainwindow.h \
        qcustomplot.h \
        analyzer/analyzer.h \
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
    popupindicator.h \
    analyzer/customanalyzer.h \
    fqinputvalidator.h \
    analyzer/updater/aa30zerofirmwareupdater.h \
    analyzer/updater/aa230firmwareupdater.h \
    analyzer/updater/firmwareupdater.h \
    analyzer/updater/hidfirmwareupdater.h \
    ProgressDlg.h \
    iprof.h \
    htime.h \
    onefqwidget.h \
    Notification.h \
    licensesdialog.h \
    glwidget.h \
    CustomPlot.h \
    customgraph.h \
    analyzer/nanovna_analyzer.h \
    tdrprogressdialog.h \
    editbandsdialog.h \
    AA55BTPacket.h \
    marqueelabel.h \
    selectdevicedialog.h \
    analyzer/baseanalyzer.h

# TODO these files dont exist and are not generated
#        ui_mainwindow.h \
#        ui_analyzerdata.h \
#        ui_fqsettings.h \
#        ui_screenshot.h \
#        ui_settings.h \
#        ui_updatedialog.h \

FORMS    += mainwindow.ui \
        analyzer/analyzerdata.ui \
        screenshot.ui \
        settings.ui \
        fqsettings.ui \
        updatedialog.ui \
        print.ui \
        export.ui \
        antscopeupdatedialog.ui \
    ProgressDlg.ui \
    licensesdialog.ui \
    tdrprogressdialog.ui \
    editbandsdialog.ui \
    selectdevicedialog.ui

INCLUDEPATH +=  $$PWD/analyzer \
            $$PWD/analyzer/updater

contains(DEFINES, NEW_CONNECTION) {
SOURCES -= analyzer/hidanalyzer.cpp
HEADERS -= analyzer/hidanalyzer.h
SOURCES += analyzer/hid_analyzer.cpp
HEADERS += analyzer/hid_analyzer.h

SOURCES -= analyzer/comanalyzer.cpp
HEADERS -= analyzer/comanalyzer.h
SOURCES += analyzer/com_analyzer.cpp
HEADERS += analyzer/com_analyzer.h
}


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

# Linux
unix:!macx {
    SOURCES += analyzer/usbhid/hidapi/linux/hid.c
    LIBS += -lusb-1.0
    DEFINES += _NO_WINDOWS_
}

macx {
    SOURCES += analyzer/usbhid/hidapi/mac/hid.c
    LIBS += -framework CoreFoundation
    DEFINES += _NO_WINDOWS_
    RUNTIME_FILES.path = Contents/MacOS/Resources
    RUNTIME_FILES.files = \
        $$PWD/cables.txt \
        $$PWD/itu-regions-defaults.txt

    TRANSLATION_FILES.path = Contents/MacOS
    TRANSLATION_FILES.files = \
        $$PWD/QtLanguage_ja.qm \
        $$PWD/QtLanguage_uk.qm \
        $$PWD/QtLanguage_ru.qm

    ICON_FILES.path = Contents
    ICON_FILES.files = \
        $$PWD/AntScope2.icns

    QMAKE_BUNDLE_DATA += RUNTIME_FILES TRANSLATION_FILES ICON_FILES
    QMAKE_INFO_PLIST= $${PWD}/Info.plist
}


#win32:QMAKE_CXXFLAGS_WARN_ON += -W4

DISTFILES += \
    rig_logo.png

RESOURCES += \
    res.qrc


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
QT       += bluetooth

#Kit: Desktop Qt 6.2.4 MSVC2015 32 bit
#message ("!!!  set path 6.2.4/MSVC19")

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += ANTSCOPE2VER='\\"2.0.4\\"'
DEFINES += OLD_TDR

#{ debug
#DEFINES += NO_MULTITAB
#} debug

QT_QPA_PLATFORM_PLUGIN_PATH=/Users/reu-admin/Qt/6.8.3/macos/plugins/platforms

#-------------------------------------------------
# under construction

# add connection dialog
DEFINES += NEW_CONNECTION

# improve analyzer selection
DEFINES += NEW_ANALYZER

#-------------------------------------------------
#DEFINES += Q_OS_WIN_FTDI

TARGET = AntScope2


TRANSLATIONS += QtLanguage.ts
TRANSLATIONS += QtLanguage_uk.ts
TRANSLATIONS += QtLanguage_ja.ts
CODECFORSRC   = UTF-8

CONFIG(release, debug|release) {
    DESTDIR = $${PWD}/build/release
}
else {
    DESTDIR = $${PWD}/build/debug
    DEFINES += _DEBUG
}

CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
    DEFINES += QT_NO_INFO_OUTPUT
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
    analyzer/analyzerpro.cpp \
    appregistrationdialog.cpp \
    encodinghelpers.cpp \
    filedialog.cpp \
    inforequestdialog.cpp \
    licenseagent.cpp \
    mainwindow.cpp \
    modelesspopup.cpp \
    printmulti.cpp \
    qcustomplot.cpp \
    #analyzer/analyzer.cpp \
    analyzer/hidanalyzer.cpp \
    analyzer/comanalyzer.cpp \
    analyzer/ble_analyzer.cpp \
    presets.cpp \
    measurements.cpp \
    analyzer/analyzerdata.cpp \
    screenshot.cpp \
    popup.cpp \
    analyzer/updater/downloader.cpp \
    settings.cpp \
    fqsettings.cpp \
    crc32.cpp \
    unitrequestdialog.cpp \
    updatedialog.cpp \
    devinfo/redeviceinfo.cpp \
    ftdi/ftdiinfo.cpp \
    markers.cpp \
    calibration.cpp \
    print.cpp \
    printmarkers.cpp \
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
    #iprof.cpp \
    onefqwidget.cpp \
    Notification.cpp \
    #licensesdialog.cpp \
    glwidget.cpp \
    CustomPlot.cpp \
    customgraph.cpp \
    analyzer/nanovna_analyzer.cpp \
    tdrprogressdialog.cpp \
    editbandsdialog.cpp \
    AA55BTPacket.cpp \
    marqueelabel.cpp \
    selectdevicedialog.cpp \
    style.cpp\
    centralwidget.cpp \
    analyzer/baseanalyzer.cpp

HEADERS  += mainwindow.h \
    analyzer/analyzerpro.h \
    appregistrationdialog.h \
    encodinghelpers.h \
    filedialog.h \
    inforequestdialog.h \
    licenseagent.h \
    modelesspopup.h \
    printmulti.h \
    qcustomplot.h \
    #analyzer/analyzer.h \
    analyzer/hidanalyzer.h \
    analyzer/comanalyzer.h \
    analyzer/analyzerparameters.h \
    analyzer/usbhid/hidapi/hidapi.h \
    analyzer/ble_analyzer.h \
    presets.h \
    measurements.h \
    analyzer/analyzerdata.h \
    screenshot.h \
    popup.h \
    analyzer/updater/downloader.h \
    settings.h \
    fqsettings.h \
    crc32.h \
    unitrequestdialog.h \
    updatedialog.h \
    devinfo/redeviceinfo.h \
    ftdi/ftd2xx.h \
    ftdi/ftdiinfo.h \
    markers.h \
    calibration.h \
    print.h \
    printmarkers.h \
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
    #iprof.h \
    htime.h \
    onefqwidget.h \
    Notification.h \
    #licensesdialog.h \
    glwidget.h \
    CustomPlot.h \
    customgraph.h \
    analyzer/nanovna_analyzer.h \
    tdrprogressdialog.h \
    editbandsdialog.h \
    AA55BTPacket.h \
    marqueelabel.h \
    selectdevicedialog.h \
    style.h \
    centralwidget.h \
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
    appregistrationdialog.ui \
    inforequestdialog.ui \
    modelesspopup.ui \
    screenshot.ui \
    settings.ui \
    fqsettings.ui \
    unitrequestdialog.ui \
    updatedialog.ui \
    print.ui \
    export.ui \
    antscopeupdatedialog.ui \
    ProgressDlg.ui \
    #licensesdialog.ui \
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

    INCLUDEPATH += $$PWD/ftdi
    DEPENDPATH += $$PWD/ftdi
    FTDI_DLL = $$PWD/ftdi/amd64/ftd2xx.dll
    win32:FTDI_DLL ~= s,/,\\,g
    win32:DESTDIR ~= s,/,\\,g
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_quote($$FTDI_DLL) $$shell_quote($$DESTDIR) $$escape_expand(\\n\\t)

    #{ 20260928_vn : OpenSSL 3.x
    # Qt 6 НЕ лінкується з OpenSSL — бекенд TLS (plugins/tls/qopensslbackend.dll)
    # завантажує libcrypto/libssl динамічно під час першого TLS-з'єднання.
    # Тому import-бібліотеки не потрібні; потрібні лише DLL поруч із .exe.
    # Шлях перевизначається: qmake OPENSSL_DIR=<...> або змінною оточення.
    isEmpty(OPENSSL_DIR): OPENSSL_DIR = $$(OPENSSL_DIR)
    isEmpty(OPENSSL_DIR): OPENSSL_DIR = C:/Qt/Tools/OpenSSLv3/Win_x64/bin

    contains(QT_ARCH, x86_64) {
        OPENSSL_DLLS = libcrypto-3-x64.dll libssl-3-x64.dll
    } else {
        OPENSSL_DLLS = libcrypto-3.dll libssl-3.dll
    }

    for(dll, OPENSSL_DLLS) {
        OPENSSL_SRC = $$OPENSSL_DIR/$$dll
        !exists($$OPENSSL_SRC): error("OpenSSL 3 runtime not found: $$OPENSSL_SRC — install the OpenSSL 3.x Toolkit via the Qt Maintenance Tool or pass OPENSSL_DIR=<path to OpenSSL 3 bin>")
        OPENSSL_SRC ~= s,/,\\,g
        QMAKE_POST_LINK += $$QMAKE_COPY $$shell_quote($$OPENSSL_SRC) $$shell_quote($$DESTDIR) $$escape_expand(\\n\\t)
    }
    #}

    sbom.target = sbom
    sbom.commands = powershell -ExecutionPolicy Bypass -File "$$PWD/scripts/make-sbom.ps1"
    QMAKE_EXTRA_TARGETS += sbom
}

# 20260928_vn : блок win64{} видалено — такого scope у qmake не існує,
# на 64-бітних збірках Windows істинним є win32. Увесь вміст блоку
# (LIBS для OpenSSL, копіювання ftd2xx.dll, ціль sbom) ніколи не виконувався
# і перенесений до win32{} вище.

# Linux
unix:!macx {
    SOURCES += analyzer/usbhid/hidapi/linux/hid.c
    LIBS += -lusb-1.0
    DEFINES += _NO_WINDOWS_
}

macx {
    DEFINES += NO_MULTITAB
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
        $$PWD/QtLanguage_uk.qm

    ICON = $$PWD/AntScope2.icns
 #   ICON_FILES.path = Contents/Resources
 #   ICON_FILES.files = \
 #       $$PWD/AntScope2.icns

    QMAKE_BUNDLE_DATA += RUNTIME_FILES TRANSLATION_FILES ICON_FILES
    QMAKE_INFO_PLIST= $${PWD}/Info.plist
}


#win32:QMAKE_CXXFLAGS_WARN_ON += -W4

DISTFILES += \
    rig_logo.png

RESOURCES += \
    res.qrc


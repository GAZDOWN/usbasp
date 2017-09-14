#-------------------------------------------------
#
# Project created by QtCreator 2012-03-04T12:29:58
#
#-------------------------------------------------

QT       += core widgets

TARGET = DebugConsole
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    usbasp.cpp \
    console.cpp \
    settingsdialog.cpp \
    settings.cpp \
    deviceselect.cpp

HEADERS  += MainWindow.h \
    usbasp.h \
    console.h \
    settingsdialog.h \
    settings.h \
    deviceselect.h

FORMS    += MainWindow.ui \
    settingsdialog.ui \
    deviceselect.ui

INCLUDEPATH += /usr/include/libusb-1.0

LIBS += -L/usr/lib -lusb-1.0

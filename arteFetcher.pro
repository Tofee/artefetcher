#-------------------------------------------------
#
# Project created by QtCreator 2012-07-25T19:49:25
#
#-------------------------------------------------

QT       += core gui network xmlpatterns script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

isEmpty(PREFIX) {
 PREFIX = /usr/bin
}

VERSION = 0.4.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

TARGET = arteFetcher

target.path = $$PREFIX/
INSTALLS += target

TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    filmdelegate.cpp \
    preferencedialog.cpp \
    preferences.cpp \
    downloadManager.cpp \
    queuedownloader.cpp \
    aboutdialog.cpp

HEADERS  += mainwindow.h \
    FilmDetails.h \
    filmdelegate.h \
    preferencedialog.h \
    preferences.h \
    downloadManager.h \
    queuedownloader.h \
    aboutdialog.h
TRANSLATIONS    = arteFetcher_de.ts \
                  arteFetcher_fr.ts

FORMS    += mainwindow.ui \
    preferencedialog.ui \
    aboutdialog.ui

INCLUDEPATH += /usr/lib64
DEPENDPATH += /usr/lib64

RESOURCES += \
    resources.qrc

RC_FILE += arteFetcher.rc

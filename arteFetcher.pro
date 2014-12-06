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

VERSION = 0.6.3

QMAKE_CXXFLAGS_DEBUG += -pg
QMAKE_LFLAGS_DEBUG += -pg
CONFIG += warn_on

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

TARGET = arteFetcher

target.path = $$PREFIX/
INSTALLS += target

TEMPLATE = app


SOURCES += main.cpp\
    filmdelegate.cpp \
    preferences.cpp \
    downloadManager.cpp \
    queuedownloader.cpp \
    catalogs/artemaincatalog.cpp \
    catalogs/icatalog.cpp \
    catalogs/artedatecatalog.cpp \
    catalogs/artelivecatalog.cpp \
    film/streamtype.cpp \
    view/aboutdialog.cpp \
    view/mainwindow.cpp \
    view/preferencedialog.cpp \
    catalogs/artedefinitions.cpp \
    catalogs/incompletedownloads.cpp \
    test/testpreferences.cpp \
    tools.cpp

HEADERS  += \
    filmdelegate.h \
    preferences.h \
    downloadManager.h \
    queuedownloader.h \
    catalogs/artemaincatalog.h \
    catalogs/icatalog.h \
    catalogs/artedatecatalog.h \
    catalogs/artelivecatalog.h \
    film/streamtype.h \
    film/filmdetails.h \
    view/aboutdialog.h \
    view/mainwindow.h \
    view/preferencedialog.h \
    catalogs/artedefinitions.h \
    context.h \
    catalogs/incompletedownloads.h \
    test/testpreferences.h \
    tools.h
TRANSLATIONS    = arteFetcher_de.ts \
                  arteFetcher_fr.ts

FORMS    += \
    view/aboutdialog.ui \
    view/mainwindow.ui \
    view/preferencedialog.ui

INCLUDEPATH += /usr/lib64
DEPENDPATH += /usr/lib64

RESOURCES += \
    resources.qrc

RC_FILE += arteFetcher.rc

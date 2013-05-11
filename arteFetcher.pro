#-------------------------------------------------
#
# Project created by QtCreator 2012-07-25T19:49:25
#
#-------------------------------------------------

QT       += core gui sql network xmlpatterns webkit script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = arteFetcher
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    filmdelegate.cpp \
    preferencedialog.cpp \
    preferences.cpp \
    downloadManager.cpp \
    queuedownloader.cpp

HEADERS  += mainwindow.h \
    FilmDetails.h \
    filmdelegate.h \
    preferencedialog.h \
    preferences.h \
    downloadManager.h \
    queuedownloader.h
TRANSLATIONS    = arteFetcher_de.ts \
                  arteFetcher_fr.ts

FORMS    += mainwindow.ui \
    preferencedialog.ui

INCLUDEPATH += /usr/lib64
DEPENDPATH += /usr/lib64

RESOURCES += \
    resources.qrc

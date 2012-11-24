#-------------------------------------------------
#
# Project created by QtCreator 2012-07-25T19:49:25
#
#-------------------------------------------------

QT       += core gui sql network xmlpatterns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = arteFetcher
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    filmdelegate.cpp \
    rtmpthread.cpp \
    preferencedialog.cpp \
    preferences.cpp

HEADERS  += mainwindow.h \
    FilmDetails.h \
    filmdelegate.h \
    rtmpthread.h \
    preferencedialog.h \
    preferences.h

FORMS    += mainwindow.ui \
    preferencedialog.ui

unix:!macx:!symbian: LIBS += -L$$PWD/../../../../usr/lib64/ -lrtmp

INCLUDEPATH += $$PWD/../../../../usr/lib64
DEPENDPATH += $$PWD/../../../../usr/lib64

RESOURCES += \
    resources.qrc

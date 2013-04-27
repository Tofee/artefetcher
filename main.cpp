#include <QApplication>
#include "mainwindow.h"
#include <FilmDetails.h>
#include <QTextCodec>
#include <QTranslator>
int main(int argc, char *argv[])
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load(":/translation/arteFetcher_de.qm");
    translator.load(":/translation/arteFetcher_fr.qm");
    a.installTranslator(&translator);

    QCoreApplication::setOrganizationName("ArteFetcher");
    QCoreApplication::setOrganizationDomain("ArteFetcher");
    QCoreApplication::setApplicationName("ArteFetcher");

    qRegisterMetaType<StreamType>("StreamType");
    MainWindow w;
    w.show();
    
    return a.exec();
}

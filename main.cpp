#include <QApplication>
#include "mainwindow.h"
#include <FilmDetails.h>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("ArteFetcher");
    QCoreApplication::setOrganizationDomain("ArteFetcher");
    QCoreApplication::setApplicationName("ArteFetcher");

    qRegisterMetaType<StreamType>("StreamType");
    MainWindow w;
    w.show();
    
    return a.exec();
}

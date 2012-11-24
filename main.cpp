#include <QApplication>
#include "mainwindow.h"
#include <FilmDetails.h>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QCoreApplication::setOrganizationName("ArteFetcher");
    QCoreApplication::setOrganizationDomain("ArteFetcher");
    QCoreApplication::setApplicationName("ArteFetcher");

    qRegisterMetaType<StreamType>("StreamType");
    MainWindow w;
    w.show();
    
    return a.exec();
}

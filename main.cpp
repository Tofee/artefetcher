/****************************************************************************

    This file is part of ArteFetcher.

    ArteFetcher is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ArteFetcher is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ArteFetcher.  If not, see <http://www.gnu.org/licenses/>.
    
****************************************************************************/

#include <QApplication>
#include "mainwindow.h"
#include <FilmDetails.h>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationVersion(APP_VERSION);


    QCoreApplication::setOrganizationName("ArteFetcher");
    QCoreApplication::setOrganizationDomain("ArteFetcher");
    QCoreApplication::setApplicationName("ArteFetcher");

    qRegisterMetaType<StreamType>("StreamType");

    // TODO give the language choice in the settings
    // (for now "export LANG=de_DE.utf8" works fine)
    QTranslator translator;
    translator.load(QLocale::system(), ":/translation/arteFetcher", "_");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    
    return a.exec();
}

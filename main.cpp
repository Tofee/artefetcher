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
#include "view/mainwindow.h"
#include <QTranslator>

#ifdef RUN_TESTS
#include "test/testpreferences.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef RUN_TESTS
    qDebug() << "Running tests...";
    TestPreferences pref;
    pref.testCompareVersions();
    qDebug() << "Test ran.";
#endif

    a.setApplicationVersion(APP_VERSION);


    QCoreApplication::setOrganizationName("ArteFetcher");
    QCoreApplication::setOrganizationDomain("ArteFetcher");
    QCoreApplication::setApplicationName("ArteFetcher");

    qRegisterMetaType<StreamType>("StreamType");

    QTranslator translator;
#if QT_VERSION >= 0x040800
    translator.load(QLocale::system(), ":/translation/arteFetcher", "_");
#else
    translator.load(QString(":/translation/arteFetcher_").append(QLocale::system().name()));
#endif
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    
    return a.exec();
}

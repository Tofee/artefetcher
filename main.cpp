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


// TODO pas coder de chiffres en dur, utiliser au moins des macros
// TODO l'icône de download n'est pas de la même taille que clock => c'est moche
// TODO voir si y'a pas moyen de récupérer d'autres infos
// TODO traductions
// TODO dans la popup quand le fichier existe déjà, donner trois choix: annuler, continuer, recommencer
// TODO bouton pour mettre en pause, arrêter les téléchargements, ou annuler les demandes
// TODO refactor pour avoir le thread dans le delegate
// TODO se débarrasser des MyPair ou au moins s'assurer que le nombre de new/delete coïncide

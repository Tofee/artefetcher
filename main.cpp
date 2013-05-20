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

//       Importance     Temps       Description
// TODO  Grande         Moyen       Bug : quand on télécharge deux épisodes d'une même série (ex téléchat), l'un écrase l'autre...
// TODO  Grande         Moyen       Bug : quand on change de langue ou de qualité, il faut effacer le cache ou recharger toutes les vidéos....
// TODO                             recherche par champ texte
// TODO  Standard       Long        voir s'il est possible de récupérer d'autres infos

// TODO  Grande         Moyen       boutons pour mettre en pause, arrêter les téléchargements, ou annuler les demandes
// TODO  Mineur         Moyen       Notifications systeme quand un film est téléchargé
// TODO  Standard       Rapide      ne pas coder de chiffres ou de string en dur, utiliser des macros ou mieux
// TODO  Standard       Moyen       refactor pour avoir le thread dans le delegate

// TODO  Mineur         Moyen       se débarrasser des MyPair ou au moins s'assurer que le nombre de new/delete coïncide
// TODO  Mineur         Moyen       avoir un changement de page plus simple


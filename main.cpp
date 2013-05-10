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
// DONE  Grande         Moyen       quand on quitte, enregistrer la liste des URL en cours de téléchargement
// DONE  Standard       Moyen       Un fichier incomplet est un .mp4.part, l'image un .png et l'info un .txt,
//                                  Quand un .part existe déjà indiquer qu'on va le continuer (Y/N)
//                                  Quand le fichier définitif existe déjà, indiquer qu'on va l'écraser (Y/N)
// DONE  Grande         Moyen       pouvoir minimiser l'application dans le tray
// TODO                             recherche par calendrier et par champ texte
// DONE  Mineur         Rapide      l'icône de download n'est pas de la même taille que clock => c'est moche
// TODO  Standard       Long        voir s'il est possible de récupérer d'autres infos
// TODO  Mineur         Rapide      Une icône moins large pour le tray

// TODO  Grande         Moyen       boutons pour mettre en pause, arrêter les téléchargements, ou annuler les demandes
// TODO  Standard       Rapide      traductions
// TODO  Mineur         Moyen       Notifications systeme quand un film est téléchargé
// TODO  Standard       Rapide      ne pas coder de chiffres ou de string en dur, utiliser des macros ou mieux
// TODO  Standard       Moyen       refactor pour avoir le thread dans le delegate

// TODO  Mineur         Moyen       se débarrasser des MyPair ou au moins s'assurer que le nombre de new/delete coïncide
// TODO  Mineur         Moyen       avoir un changement de page plus simple


#include "artelivecatalog.h"
#include <QDebug> // TODO
#include <film/filmdetails.h>
#include <preferences.h>
#include "artedefinitions.h"

ArteLiveCatalog::ArteLiveCatalog(QObject *parent) : QObject(parent)
{
    m_urlByCatalogName.insert(tr("Live - Selection"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/selection");
    m_urlByCatalogName.insert(tr("Live - Classical"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/classic");
    m_urlByCatalogName.insert(tr("Live - Rock"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/rock");
    m_urlByCatalogName.insert(tr("Live - Jazz"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/jazz");
    m_urlByCatalogName.insert(tr("Live - World"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/world");
    m_urlByCatalogName.insert(tr("Live - Dance"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/dance");
}

QList<FilmDetails*> ArteLiveCatalog::listFilmsFromCatalogAnswer(QString catalogName, const QString& catalogAnswer, int fromIndex, int toIndex, int& lastIndex){
    QList<FilmDetails*> result;

    int i = -1;
    QList<QVariant> list = extractJsonMapFromAnswer(catalogAnswer).value("videos").toList();

    foreach(QVariant catalogItem, list)
    {
        ++i;

        if (i >= fromIndex && i < toIndex) {
            QString url = catalogItem.toMap().value("VTR").toString(); // or VUP
            QString title = catalogItem.toMap().value("VTI").toString();
            QString arteId = catalogItem.toMap().value("VPI").toString();

            FilmDetails* newFilm = new FilmDetails(catalogName, title, url, arteId);

            addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), "VDE", Description);
            addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), "VCG", Genre);
            //addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIEWS, Views);
            //addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIDEO_CHANNEL, Channels);

            extractArteVideoStreamsFromMap(catalogItem.toMap(), newFilm, false);

            result << newFilm;

            QString imageUrl = catalogItem.toMap().value("VTU").toMap().value("IUR").toString();
            if (!imageUrl.isEmpty()){
                emit requestImageDownload(newFilm, imageUrl);
            }
        }
    }

    lastIndex = i+1;// TODO c'est moche!!!
    return result;
}

QString ArteLiveCatalog::getFilmDetailsUrl(FilmDetails*){
    // Nothing to do here. Everything is fetched in listFilmsFromCatalogAnswer
    return QString();
}

void ArteLiveCatalog::processFilmDetails(FilmDetails*, QString){
    // Nothing to do here. Everything is fetched in listFilmsFromCatalogAnswer
}

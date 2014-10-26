#include "artelivecatalog.h"
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
    QList<QVariant> list = extractJsonMapFromAnswer(catalogAnswer).value("videos").toList();
    QList<FilmDetails*> result;
    lastIndex = list.size();

    for (int i = fromIndex; i < toIndex && i < list.size(); ++i){
        QMap<QString, QVariant> catalogItem = list.at(i).toMap();
        QString url = catalogItem.value("VTR").toString(); // or VUP
        QString title = catalogItem.value("VTI").toString();
        QString arteId = catalogItem.value("VPI").toString();

        FilmDetails* newFilm = new FilmDetails(catalogName, title, url, arteId);

        QDateTime shootingDate = QDateTime::fromString(catalogItem.value(JSON_FILMPAGE_SHOOTING_DATE).toString(), "dd/MM/yyyy HH:mm:ss +0200");
        if (shootingDate.date().year() > 1900){
            newFilm->m_metadata[Shooting_date] = shootingDate.toString(Qt::DefaultLocaleLongDate);
            newFilm->m_metadata[Production_year] = QString::number(shootingDate.date().year());
        }

        extractMainArteMetaFromJsonMap(newFilm, catalogItem);
        updateArteEpisodeNumber(newFilm);

        extractArteVideoStreamsFromMap(catalogItem, newFilm, false);

        result << newFilm;

        QString imageUrl = catalogItem.value("VTU").toMap().value("IUR").toString();
        if (!imageUrl.isEmpty()){
            emit requestImageDownload(newFilm, imageUrl);
        }
    }
    return result;
}

QString ArteLiveCatalog::getFilmDetailsUrl(FilmDetails*){
    // Nothing to do here. Everything is fetched in listFilmsFromCatalogAnswer
    return QString();
}

void ArteLiveCatalog::processFilmDetails(FilmDetails*, QString){
    // Nothing to do here. Everything is fetched in listFilmsFromCatalogAnswer
}

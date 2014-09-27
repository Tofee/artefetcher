#include "artemaincatalog.h"
#include <preferences.h>
#include <film/filmdetails.h>
#include "artedefinitions.h"

ArteMainCatalog::ArteMainCatalog(QObject *parent)
    :QObject(parent)
{
    m_urlByCatalogName.insert(tr("All"), "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7.json");
    m_urlByCatalogName.insert(tr("Arte selection"), "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7/selection.json");
    m_urlByCatalogName.insert(tr("Most recent"), "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7/plus_recentes.json");
    m_urlByCatalogName.insert(tr("Most seen"), "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7/plus_vues.json");
    m_urlByCatalogName.insert(tr("Last chance"), "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7/derniere_chance.json");
}


QList<FilmDetails*> ArteMainCatalog::listFilmsFromCatalogAnswer(QString catalogName, const QString& catalogAnswer, int fromIndex, int toIndex, int& lastIndex){
    QList<QVariant> list = extractJsonMapFromAnswer(catalogAnswer).value("videos").toList();
    lastIndex = list.size();
    QList<FilmDetails*> result;

    for (int i = fromIndex; i < toIndex && i < list.size(); ++i){
        QMap<QString, QVariant> catalogItem = list.at(i).toMap();
        QString url = catalogItem.value("url").toString();
        url.prepend("http://www.arte.tv");
        QString title = catalogItem.value("title").toString();
        QString arteId = catalogItem.value("em").toString();

        FilmDetails* newFilm = new FilmDetails(catalogName, title, url, arteId);
        updateArteEpisodeNumber(newFilm);
        addMetadataIfNotEmpty(newFilm, catalogItem, JSON_DESC, Description);
        addMetadataIfNotEmpty(newFilm, catalogItem, JSON_VIEWS, Views);
        addMetadataIfNotEmpty(newFilm, catalogItem, JSON_VIDEO_CHANNEL, Channels);

        result << newFilm;

        QString imageUrl = catalogItem.value("image_url").toString();
        if (!imageUrl.isEmpty()){
            emit requestImageDownload(newFilm, imageUrl);
        }
    }
    return result;
}

QString ArteMainCatalog::getFilmDetailsUrl(FilmDetails* film){

    QString languageCharacter = Preferences::getInstance()->applicationLanguage().left(1).toUpper();

    QString jsonUrl = film->m_arteId.isEmpty() ? "" : QString("http://org-www.arte.tv/papi/tvguide/videos/stream/player/%0/%1_PLUS7-%2/ALL/ALL.json")
                                         .arg(languageCharacter)
                                         .arg(film->m_arteId)
                                         .arg(languageCharacter);
    return jsonUrl;
}

void ArteMainCatalog::processFilmDetails(FilmDetails* film, QString htmlPage){
    defaultArteProcessFilmDetails(film, htmlPage);
}

#include "artemaincatalog.h"
#include <preferences.h>
#include <film/filmdetails.h>
#include "artedefinitions.h"
#include <QDebug> // TODO

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
    QList<FilmDetails*> result;
    int i = -1;

    foreach(QVariant catalogItem, list)
    {
        ++i;

        if (i >= fromIndex && i < toIndex) {
            QString url = catalogItem.toMap().value("url").toString();
            url.prepend("http://www.arte.tv");
            QString title = catalogItem.toMap().value("title").toString();
            QString arteId = catalogItem.toMap().value("em").toString();

            FilmDetails* newFilm = new FilmDetails(catalogName, title, url, arteId);

            addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_DESC, Description);
            addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIEWS, Views);
            addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIDEO_CHANNEL, Channels);

            result << newFilm;

            QString imageUrl = catalogItem.toMap().value("image_url").toString();
            if (!imageUrl.isEmpty()){
                emit requestImageDownload(newFilm, imageUrl);
            }
        }
    }

    lastIndex = i+1;// TODO c'est moche!!!
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

void ArteMainCatalog::processFilmDetails(FilmDetails* film, QString httpAnswer){
    QMap<QString, QVariant> mymap = extractJsonMapFromAnswer(httpAnswer).value("videoJsonPlayer").toMap();
    if (mymap.isEmpty())
    {
        // TODO
        return;
        //qDebug() << "[ERROR] Cannot find 'videoJsonPlayer' for" << film->m_infoUrl << " in" << reply->request().url() << page;
        //emit(errorOccured(film->m_infoUrl,tr("Cannot load stream details")));
    }
    else {


        if (mymap.value(JSON_FILMPAGE_DURATION_SECONDS).toString() != "" && film->m_title == "")
        {
            film->m_title = mymap.value(JSON_FILMPAGE_DURATION_SECONDS).toString();
        }
        film->m_durationInMinutes = mymap.value("videoDurationSeconds").toInt() /60;
        film->m_summary = mymap.value(JSON_FILMPAGE_SUMMARY).toString();

        addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_TYPE,              Type);
        addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_FIRST_BROADCAST,   RAW_First_Broadcast, true); // 25/04/2013 20:50:30 +0200
        addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_AVAILABILITY,      RAW_Available_until, true); // 02/05/2013 20:20:30 +0200
        addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_VIDEO_TYPE,        Preview_Or_ArteP7); // EXTRAIT (AUSSCHNITT in german) or ARTE+7
        addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_VSU,               Episode_name); // if not null, it belongs to a serie
        addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_VIEWS,             Views); // different from the one in the catalog: this is just a number
        addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_DESCRIPTION,       Description);

        QStringList labels;
        foreach (QVariant channelItem, mymap.value(JSON_FILMPAGE_CHANNELS).toList())
        {
            labels << channelItem.toMap().value(JSON_FILMPAGE_CHANNELS_LABEL).toString();
        }
        if (!labels.isEmpty())
        {
            film->m_metadata.insert(Channels, labels.join(", "));
        }

        extractArteVideoStreamsFromMap(mymap, film, true);
    }
}

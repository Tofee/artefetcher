#include "artedefinitions.h"
#include <QDebug>
#include "../film/filmdetails.h"
#include "../preferences.h"
#include "icatalog.h"
void extractArteVideoStreamsFromMap(QMap<QString, QVariant> mapWithStream, FilmDetails* film, bool onlyHbbtv)
{
    foreach (QVariant streamJson, mapWithStream.value("VSR").toMap().values()){
        QMap<QString, QVariant> map = streamJson.toMap();
        if (onlyHbbtv && map.value("videoFormat").toString() != "HBBTV"){
            continue;
        }
        if (map.value("VQU").toString().toLower() == Preferences::getInstance()->selectedQuality())
        {
            QString streamName = map.value("versionLibelle").toString();
            if (!Preferences::getInstance()->favoriteStreamTypes().contains(streamName)){
                qDebug() << "Add missing stream type in preferences:" << streamName;
                Preferences::getInstance()->addStreamName(streamName);
            }
            film->m_allStreams[streamName] = map.value("url").toString();
        }
    }
}

void updateArteEpisodeNumber(FilmDetails* film){
    QStringList splittenCode = film->m_arteId.split("-");
    film->m_episodeNumber = (splittenCode.size() == 2 ? splittenCode.value(1).toInt() : 0);
}

void defaultArteProcessFilmDetails(FilmDetails *film, QString htmlPage){
    QMap<QString, QVariant> mymap = extractJsonMapFromAnswer(htmlPage).value("videoJsonPlayer").toMap();
    if (mymap.value(JSON_FILMPAGE_TITLE).toString() != "" && film->m_title == "")
    {
        film->m_title = mymap.value(JSON_FILMPAGE_TITLE).toString();
    }
    extractMainArteMetaFromJsonMap(film, mymap);

    extractArteVideoStreamsFromMap(mymap, film, true);
}



void extractMainArteMetaFromJsonMap(FilmDetails* film, QMap<QString, QVariant> map) {
    bool ok;
    int minutes = map.value("videoDurationSeconds").toInt(&ok) /60;
    if (ok)
        film->m_durationInMinutes = minutes;
    film->m_summary = map.value(JSON_FILMPAGE_SUMMARY).toString();

    addMetadataIfNotEmpty(film, map, JSON_FILMPAGE_TYPE,              Type);
    addMetadataIfNotEmpty(film, map, JSON_FILMPAGE_FIRST_BROADCAST,   RAW_First_Broadcast, true); // 25/04/2013 20:50:30 +0200
    addMetadataIfNotEmpty(film, map, JSON_FILMPAGE_AVAILABILITY,      RAW_Available_until, true); // 02/05/2013 20:20:30 +0200
    addMetadataIfNotEmpty(film, map, JSON_FILMPAGE_VIDEO_TYPE,        Preview_Or_ArteP7); // EXTRAIT (AUSSCHNITT in german) or ARTE+7
    addMetadataIfNotEmpty(film, map, JSON_FILMPAGE_VSU,               Episode_name); // if not null, it belongs to a serie
    addMetadataIfNotEmpty(film, map, JSON_FILMPAGE_VIEWS,             Views); // different from the one in the catalog: this is just a number
    addMetadataIfNotEmpty(film, map, JSON_FILMPAGE_DESCRIPTION,       Description);
    addMetadataIfNotEmpty(film, map, JSON_FILMPAGE_PARTNERSHIP,       Partnership);
    addMetadataIfNotEmpty(film, map, JSON_FILMPAGE_PARTNERSHIP_WEB,   Partnership_web);
    addMetadataIfNotEmpty(film, map, JSON_FILMPAGE_PRODUCTION_YEAR,   Production_year);

    QStringList labels;
    foreach (QVariant channelItem, map.value(JSON_FILMPAGE_CHANNELS).toList())
    {
        labels << channelItem.toMap().value(JSON_FILMPAGE_CHANNELS_LABEL).toString();
    }
    if (!labels.isEmpty())
    {
        film->m_metadata.insert(Channels, labels.join(", "));
    }
}

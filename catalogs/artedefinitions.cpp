#include "artedefinitions.h"
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
            film->m_allStreams[map.value("versionLibelle").toString()] = map.value("url").toString();
        }
    }
}

void updateArteEpisodeNumber(FilmDetails* film){
    QStringList splittenCode = film->m_arteId.split("-");
    film->m_episodeNumber = (splittenCode.size() == 2 ? splittenCode.value(1).toInt() : 0);
}

void defaultArteProcessFilmDetails(FilmDetails *film, QString httpAnswer){
    QMap<QString, QVariant> mymap = extractJsonMapFromAnswer(httpAnswer).value("videoJsonPlayer").toMap();
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

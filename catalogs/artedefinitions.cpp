#include "artedefinitions.h"
#include "../film/filmdetails.h"
#include "../preferences.h"

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

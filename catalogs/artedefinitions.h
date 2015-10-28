#ifndef ARTEDEFINITIONS_H
#define ARTEDEFINITIONS_H
#include <QMap>
#include <QVariant>


#define JSON_AIRDATE        "airdate"
#define JSON_AIRDATE_LONG   "airdate_long"
#define JSON_AIRTIME        "airtime"
#define JSON_DESC           "DLO"
#define JSON_RIGHTS_UNTIL   "video_rights_until"
#define JSON_VIEWS          "video_views"
#define JSON_VIDEO_CHANNEL  "video_channels"
#define JSON_RANK           "video_rank"

#define JSON_FILMPAGE_TYPE              "VCG"
#define JSON_FILMPAGE_FIRST_BROADCAST   "VDA"
#define JSON_FILMPAGE_AVAILABILITY      "VRU"
#define JSON_FILMPAGE_VIDEO_TYPE        "VTX"
#define JSON_FILMPAGE_VSU               "VSU"
#define JSON_FILMPAGE_VIEWS             "VVI"
// #define JSON_FILMPAGE_RANK           "videoRank"// useless, it just gives the position in the carousel...
#define JSON_FILMPAGE_DESCRIPTION       "V7T"
#define JSON_FILMPAGE_TITLE             "VTI"
#define JSON_FILMPAGE_SUMMARY           "VDE"
#define JSON_FILMPAGE_CHANNELS          "VCH"
#define JSON_FILMPAGE_CHANNELS_LABEL    "label"
#define JSON_FILMPAGE_PREVIEW           "VTU"
#define JSON_FILMPAGE_PREVIEW_URL       "IUR"
#define JSON_FILMPAGE_PARTNERSHIP       "VPT"
#define JSON_FILMPAGE_PARTNERSHIP_WEB   "VPA"
#define JSON_FILMPAGE_PRODUCTION_YEAR   "productionYear"
#define JSON_FILMPAGE_SHOOTING_DATE     "shootingDate"
#define JSON_FILMPAGE_DIRECTOR          "director"
#define JSON_FILMPAGE_INFOPROG          "infoProg" /*Donne quelque chose comme : "(ROYAUME UNI , 2008, 52mn)
    ARTE F"*/

class FilmDetails;

void extractArteVideoStreamsFromMap(QMap<QString, QVariant> mapWithStream, FilmDetails* film, bool onlyHbbtv);

/**
 * @brief updateArteEpisodeNumber Extracts the arte episode number from the arte id
 * @param film film to update
 */
void updateArteEpisodeNumber(FilmDetails* film);

void defaultArteProcessFilmDetails(FilmDetails* film, QString htmlPage);

void extractMainArteMetaFromJsonMap(FilmDetails* film, QMap<QString, QVariant> map);

#endif // ARTEDEFINITIONS_H

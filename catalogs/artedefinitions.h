#ifndef ARTEDEFINITIONS_H
#define ARTEDEFINITIONS_H
#include <QMap>
#include <QVariant>


#define JSON_AIRDATE        "airdate"
#define JSON_AIRDATE_LONG   "airdate_long"
#define JSON_AIRTIME        "airtime"
#define JSON_DESC           "desc"
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
// #define JSON_FILMPAGE_RANK              "videoRank"// useless, it just gives the position in the carousel...
#define JSON_FILMPAGE_DESCRIPTION       "V7T"
#define JSON_FILMPAGE_DURATION_SECONDS  "VTI"
#define JSON_FILMPAGE_SUMMARY           "VDE"
#define JSON_FILMPAGE_CHANNELS          "VCH"
#define JSON_FILMPAGE_CHANNELS_LABEL    "label"
#define JSON_FILMPAGE_PREVIEW           "VTU"
#define JSON_FILMPAGE_PREVIEW_URL       "IUR"

class FilmDetails;

void extractArteVideoStreamsFromMap(QMap<QString, QVariant> mapWithStream, FilmDetails* film, bool onlyHbbtv);

/**
 * @brief updateArteEpisodeNumber Extracts the arte episode number from the arte id
 * @param film film to update
 */
void updateArteEpisodeNumber(FilmDetails* film);

void defaultArteProcessFilmDetails(FilmDetails* film, QString httpAnswer);

#endif // ARTEDEFINITIONS_H

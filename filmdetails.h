/****************************************************************************

    This file is part of ArteFetcher.

    ArteFetcher is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ArteFetcher is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ArteFetcher.  If not, see <http://www.gnu.org/licenses/>.
    
****************************************************************************/

#ifndef FILMDETAILS_H
#define FILMDETAILS_H
#include <QString>
#include <QImage>
#include <QMap>
#include <QStringList>
#include <QApplication>
struct StreamType {

public:
    StreamType(){}
    StreamType(StreamType& other)
        :humanCode(other.humanCode), languageCode(other.languageCode), qualityCode(other.qualityCode)
    {}
    StreamType(const StreamType& other)
        :humanCode(other.humanCode), languageCode(other.languageCode), qualityCode(other.qualityCode)
    {}
    virtual ~StreamType(){}

    StreamType(const QString& in_humanCode, const QString& in_languageCode, const QString& in_qualityCode)
        :humanCode(in_humanCode), languageCode(in_languageCode), qualityCode(in_qualityCode)
    {}

    bool operator==(const StreamType& other) const
    {
        return other.humanCode == humanCode &&
                other.languageCode == languageCode &&
                other.qualityCode == qualityCode;
    }
    int operator<(const StreamType& other) const
    {
        if (other.humanCode == humanCode &&
                other.languageCode == languageCode &&
                other.qualityCode == qualityCode)
            return 0;
        return -1;
    }

    QString humanCode;
    QString languageCode;
    QString qualityCode;
};

enum MetaType {
//    First_broadcast, // useless, see RAW_First_Broadcast
//    First_broadcast_long, // useless, see RAW_First_Broadcast
//    First_broadcast_time, // useless, see RAW_First_Broadcast
    Description,
//    Available_until, // useless, see RAW_Available_until
    Views,
    Channels,
    Type,
    RAW_First_Broadcast,
    RAW_Available_until,
    Episode_name /* This is only available for series */,
    Preview_Or_ArteP7,
    Genre
};

enum EDownloadStatus {
    DL_NONE, DL_REQUESTED, DL_DOWNLOADING, DL_DOWNLOADED, DL_CANCELLED, DL_ERROR
};

class FilmDetails {

    //TODO mettre tout en privé sauf les accesseurs
public:
    FilmDetails(QString originCatalog, QString title, QString filmUrl, QString arteId)
        :m_catalogName(originCatalog), m_title(title), m_infoUrl(filmUrl), m_arteId(arteId), m_durationInMinutes(-1), m_downloadStatus(DL_NONE), m_downloadProgress(0), m_replayAvailable(true)
    {}
    QString m_catalogName;
    QString m_title;
    QString m_infoUrl;
    QString m_arteId;

    int episodeNumber;

    QMap<QString, QImage> m_preview; // preview indexed by image url
    QString m_summary;

    double m_rating;
    uint m_numberOfViews;


    int m_durationInMinutes;
    int m_year;

    QMap<MetaType, QString> m_metadata;

    EDownloadStatus m_downloadStatus;
    int m_downloadProgress;

    // Map of stream urls indexed by stream type name
    QMap<QString, QString> m_allStreams;
    // Full path of the future downloaded film.
    QString m_targetFileName;
    QString m_choosenStreamType;

    /**
     * @brief m_replayAvailable true if replay is available for the film
     */
    bool m_replayAvailable;

    QStringList m_errors;

    QString title() const { return m_title; }
    static const QString enum2Str(MetaType t){
        static const char* enum2Str[] = {
//             QT_TRANSLATE_NOOP("FilmDetails","First broadcast"),
//             QT_TRANSLATE_NOOP("FilmDetails","First broadcast"),
//             QT_TRANSLATE_NOOP("FilmDetails","Time of the first broadcast"),
             QT_TRANSLATE_NOOP("FilmDetails","Description"),
//             QT_TRANSLATE_NOOP("FilmDetails","Available until"),
             QT_TRANSLATE_NOOP("FilmDetails","Views"),
             QT_TRANSLATE_NOOP("FilmDetails","Channels"),
//             QT_TRANSLATE_NOOP("FilmDetails","Rank"),
             QT_TRANSLATE_NOOP("FilmDetails","Type"),
             QT_TRANSLATE_NOOP("FilmDetails","First Broadcast"),
             QT_TRANSLATE_NOOP("FilmDetails","Available until"),
             QT_TRANSLATE_NOOP("FilmDetails","Episode"),
             QT_TRANSLATE_NOOP("FilmDetails","Broadcast type"),
             QT_TRANSLATE_NOOP("FilmDetails", "Genre")
        };


        return QApplication::translate("FilmDetails",
                                       enum2Str[t]);
    }
};



#endif // FILMDETAILS_H

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
    First_broadcast,
    First_broadcast_long,
    First_broadcast_time,
    Description,
    Available_until,
    Views,
    Channels,
    Rank,
    Type,
    RAW_First_Broadcast,
    RAW_Available_until,
    Episode_name /* This is only available for series */
};


class FilmDetails {

    //TODO mettre tout en privé sauf les accesseurs
public:
    QString m_title;
    QImage m_preview;
    QString m_summary;

    double m_rating;
    uint m_numberOfViews;
    QString m_infoUrl;

    int m_durationInMinutes;
    int m_year;

    QMap<MetaType, QString> m_metadata;

    bool m_hasBeenRequested;
    bool m_isDownloaded;
    bool m_isDownloading;
    QString m_streamUrl;
    // Full path of the future downloaded film.
    QString m_targetFileName;


    QString title() const { return m_title; }
    static const QString enum2Str(MetaType t){
        static const char* enum2Str[] = {
             QT_TRANSLATE_NOOP("FilmDetails","First broadcast"),
             QT_TRANSLATE_NOOP("FilmDetails","First broadcast"),
             QT_TRANSLATE_NOOP("FilmDetails","Time of the first broadcast"),
             QT_TRANSLATE_NOOP("FilmDetails","Description"),
             QT_TRANSLATE_NOOP("FilmDetails","Available until"),
             QT_TRANSLATE_NOOP("FilmDetails","Views"),
             QT_TRANSLATE_NOOP("FilmDetails","Channels"),
             QT_TRANSLATE_NOOP("FilmDetails","Rank"),
             QT_TRANSLATE_NOOP("FilmDetails","Type"),
             QT_TRANSLATE_NOOP("FilmDetails","RAW First Broadcast"),
             QT_TRANSLATE_NOOP("FilmDetails","RAW Available until"),
             QT_TRANSLATE_NOOP("FilmDetails","Episode")
        };


        return QApplication::translate("FilmDetails",
                                       enum2Str[t]);
    }
};



#endif // FILMDETAILS_H

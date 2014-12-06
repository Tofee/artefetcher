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
    Partnership,
    Partnership_web,
    Production_year,
    Shooting_date,
    Director,
    Infoprog
};

enum EDownloadStatus {
    DL_NONE, DL_REQUESTED, DL_DOWNLOADING, DL_DOWNLOADED, DL_CANCELLED, DL_ERROR
};

class FilmDetails {
    //TODO mettre tout en privé sauf les accesseurs
public:
    FilmDetails(QString originCatalog, QString title, QString filmUrl, QString arteId)
        :m_catalogName(originCatalog), m_title(title), m_arteId(arteId), m_episodeNumber(0), m_durationInMinutes(-1), m_downloadStatus(DL_NONE), m_downloadProgress(0), m_replayAvailable(true), m_infoUrl(filmUrl)
    {}
    QString m_catalogName;
    QString m_title;

    QString m_arteId;

    int m_episodeNumber;

    QMap<QString, QImage> m_preview; // preview indexed by image url
    QString m_summary;

    uint m_numberOfViews;

    int m_durationInMinutes;

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

    QString relatedWebPage(){ return m_infoUrl; }

    QStringList m_errors;
    QStringList m_producingCountries;// Liste des pays de réalisation du film

    QString title() const { return m_title; }
    static const QString enum2Str(MetaType t){
        static const char* enum2Str[] = {
             QT_TRANSLATE_NOOP("FilmDetails","Description"),
             QT_TRANSLATE_NOOP("FilmDetails","Views"),
             QT_TRANSLATE_NOOP("FilmDetails","Channels"),
             QT_TRANSLATE_NOOP("FilmDetails","Type"),
             QT_TRANSLATE_NOOP("FilmDetails","First Broadcast"),
             QT_TRANSLATE_NOOP("FilmDetails","Available until"),
             QT_TRANSLATE_NOOP("FilmDetails","Episode"),
             QT_TRANSLATE_NOOP("FilmDetails","Broadcast type"),
             QT_TRANSLATE_NOOP("FilmDetails","Partnership"),
             QT_TRANSLATE_NOOP("FilmDetails","Partnership website"),
             QT_TRANSLATE_NOOP("FilmDetails","Year"),
             QT_TRANSLATE_NOOP("FilmDetails","Shooting date"),
             QT_TRANSLATE_NOOP("FilmDetails","Director"),
             QT_TRANSLATE_NOOP("FilmDetails","Origine") // Pays d'origine
        };


        return QApplication::translate("FilmDetails",
                                       enum2Str[t]);
    }
protected:
    QString m_infoUrl;
};



#endif // FILMDETAILS_H

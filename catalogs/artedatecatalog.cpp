#include "artedatecatalog.h"
#include <QDate>
#include <preferences.h>
#include "artedefinitions.h"

ArteDateCatalog::ArteDateCatalog(QObject *parent)
    :QObject(parent)
{
    m_urlByCatalogName.insert(tr("By date"),"::arteDate::");
    // No need to provide a real url, there is only one url for dates and the final url
    // will be built according to the given date
}


QString ArteDateCatalog::getUrlForCatalogNames(QString, QDate catalogDate) const {
    // Goal build an URL with the date in this example format:
    // "http://www.arte.tv/papi/tvguide/epg/schedule/F/L3/2014-09-05/2014-9-5.json"
    // 2015-10-28 new goal : "http://www.arte.tv/papi/tvguide/videos/plus7/program/F/L3/ALL/ALL/-1/AIRDATE_DESC/0/0/DE_FR/2015-10-28.json;
    QString baseUrl("http://www.arte.tv/papi/tvguide/videos/plus7/program/F/L3/ALL/ALL/-1/AIRDATE_DESC/0/0/DE_FR/%1-%2-%3.json");
    return baseUrl.arg(catalogDate.year())
                .arg(catalogDate.month(), 2, 10, QChar('0'))
                .arg(catalogDate.day(), 2, 10, QChar('0'));
}

QList<FilmDetails*> ArteDateCatalog::listFilmsFromCatalogAnswer(QString catalogName, const QString &catalogAnswer, int fromIndex, int toIndex, int &lastIndex)
{
    QList<QVariant> list = extractJsonMapFromAnswer(catalogAnswer).value("programFRList").toList();
    lastIndex = list.size();
    QList<FilmDetails*> result;

    for (int i = fromIndex; i < toIndex && i < list.size(); ++i){
        QMap<QString, QVariant> catalogItem = list.at(i).toMap();
        QString url = catalogItem.value("videoStreamUrl").toString();
        QString title = catalogItem.value("TIT").toString();
        QString arteId = catalogItem.value("PID").toString();

        FilmDetails* newFilm = new FilmDetails(catalogName, title, url, arteId);
        // updateArteEpisodeNumber(newFilm);
        addMetadataIfNotEmpty(newFilm, catalogItem, JSON_DESC, Description);
        //addMetadataIfNotEmpty(newFilm, catalogItem, JSON_VIEWS, Views);
        //addMetadataIfNotEmpty(newFilm, catalogItem, JSON_VIDEO_CHANNEL, Channels);

        result << newFilm;

        QString imageUrl = catalogItem.value("IMG").toMap().value("IUR").toString();
        if (!imageUrl.isEmpty()){
            emit requestImageDownload(newFilm, imageUrl);
        }
    }
    return result;
}

QString ArteDateCatalog::getFilmDetailsUrl(FilmDetails *film){
    QString languageCharacter = Preferences::getInstance()->applicationLanguage().left(1).toUpper();

    QString jsonUrl = film->m_arteId.isEmpty() ? "" : QString("http://org-www.arte.tv/papi/tvguide/videos/stream/player/%0/%1_PLUS7-%2/ALL/ALL.json")
                                         .arg(languageCharacter)
                                         .arg(film->m_arteId)
                                         .arg(languageCharacter);
    return jsonUrl;
}

void ArteDateCatalog::processFilmDetails(FilmDetails *film, QString htmlPage){
    defaultArteProcessFilmDetails(film, htmlPage);
}

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
    QString baseUrl("http://www.arte.tv/papi/tvguide/videos/plus7/program/%1/L3/ALL/ALL/-1/AIRDATE_DESC/0/0/DE_FR/%2-%3-%4.json");
    return baseUrl.arg( Preferences::getInstance()->applicationLanguage().left(1).toUpper()) // On transforme "fr" en "F", et "de" en "D"
                .arg(catalogDate.year())
                .arg(catalogDate.month(), 2, 10, QChar('0'))
                .arg(catalogDate.day(), 2, 10, QChar('0'));
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

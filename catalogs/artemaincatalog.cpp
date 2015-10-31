#include "artemaincatalog.h"
#include <preferences.h>
#include <film/filmdetails.h>
#include "artedefinitions.h"

ArteMainCatalog::ArteMainCatalog(QObject *parent)
    :QObject(parent)
{
    // On extrait un language code sur un caractère (F, D)
    QString languageCode = Preferences::getInstance()->applicationLanguage().left(1).toUpper();

    //  http://www.arte.tv/hbbtvv2/index.html?lang=fr_FR&tv=false
    //m_urlByCatalogName.insert(tr("All"),            QString("http://www.arte.tv/papi/tvguide/videos/ARTE_PLUS_SEVEN/%1.json?limit=1").arg(languageCode));
    // Url parameters :
    //   First number after AIRDATE_DESC is the maxResult
    //   Second number after AIRDATE_DESC is the offset
    m_urlByCatalogName.insert(tr("Arte selection"), QString("http://www.arte.tv/papi/tvguide/videos/plus7/program/%1/L3/ALL/ALL/1/AIRDATE_DESC/6/0/DE_FR.json").arg(languageCode));

    //m_urlByCatalogName.insert(tr("Most recent"), "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7/plus_recentes.json");
    m_urlByCatalogName.insert(tr("Most seen"),      QString("http://www.arte.tv/papi/tvguide/videos/plus7/program/%1/L3/ALL/ALL/-1/VIEWS/10/0/DE_FR.json").arg(languageCode));
    //m_urlByCatalogName.insert(tr("Last chance"), "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7/derniere_chance.json");
}



QString ArteMainCatalog::getFilmDetailsUrl(FilmDetails* film){

    QString languageCharacter = Preferences::getInstance()->applicationLanguage().left(1).toUpper();

    QString jsonUrl = film->m_arteId.isEmpty() ? "" : QString("http://org-www.arte.tv/papi/tvguide/videos/stream/player/%0/%1_PLUS7-%2/ALL/ALL.json")
                                         .arg(languageCharacter)
                                         .arg(film->m_arteId)
                                         .arg(languageCharacter);
    return jsonUrl;
}

void ArteMainCatalog::processFilmDetails(FilmDetails* film, QString htmlPage){
    defaultArteProcessFilmDetails(film, htmlPage);
}

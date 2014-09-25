#include "incompletedownloads.h"
#include "preferences.h"
#include <QDebug>
IncompleteDownloads::IncompleteDownloads(QObject* parent)
    :QObject(parent)
{
    QString catalogName = tr("Incomplete downloads");
    m_urlByCatalogName.insert(catalogName, INCOMPLETE_DOWNLOADS_URL);
}


QList<FilmDetails*> IncompleteDownloads::listFilmsFromCatalogAnswer(QString catalogName, const QString&, int fromIndex, int toIndex, int& lastIndex){
    QList<FilmDetails*> m_incompleteDownloads;
    int i = -1;

    foreach(QVariant v, Preferences::getInstance()->pendingDownloads()){
        ++i;

        if (i >= fromIndex && i < toIndex) {
            QMap<QString, QVariant> filmMap = v.toMap();

            QString arteId = filmMap.value(PREFERENCES_FILMMAP_ARTEID).toString();
            QString title = filmMap.value(PREFERENCES_FILMMAP_TITLE).toString();
            QString filmUrl = filmMap.value(PREFERENCES_FILMMAP_FILMURL).toString();

            FilmDetails* film = new FilmDetails(catalogName, title, filmUrl, arteId);
            film->m_summary = filmMap.value(PREFERENCES_FILMMAP_DESC).toString();
            film->m_durationInMinutes = filmMap.value(PREFERENCES_FILMMAP_DURATION).toInt();

            film->m_metadata.insert(Episode_name, filmMap.value(PREFERENCES_FILMMAP_EPISODE_NAME).toString());
            film->m_episodeNumber = filmMap.value(PREFERENCES_FILMMAP_EPISODE_NUMBER).toInt();

            film->m_allStreams.insert(filmMap.value(PREFERENCES_FILMMAP_VIDEOQUALITY).toString()
                                      ,filmMap.value(PREFERENCES_FILMMAP_VIDEOURL).toString());

            QString imageUrl = filmMap.value(PREFERENCES_FILMMAP_IMAGE).toString();
            if (!imageUrl.isEmpty()) {
                requestImageDownload(film, imageUrl);
            }
            m_incompleteDownloads << film;

        }
    }
    lastIndex = i+1;// TODO c'est moche!!!
    return m_incompleteDownloads;
}

QString IncompleteDownloads::getFilmDetailsUrl(FilmDetails*){
    // NOTHING TO DO
    return QString();
}

void IncompleteDownloads::processFilmDetails(FilmDetails*, QString){
    // NOTHING TO DO
}

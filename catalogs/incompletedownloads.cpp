#include "incompletedownloads.h"
#include "preferences.h"

IncompleteDownloads::IncompleteDownloads(QObject* parent)
    :QObject(parent)
{
    QString catalogName = tr("Downloads to resume");
    m_urlByCatalogName.insert(catalogName, INCOMPLETE_DOWNLOADS_URL);
}


QList<FilmDetails*> IncompleteDownloads::listFilmsFromCatalogAnswer(QString catalogName, const QString&, int fromIndex, int toIndex, int& lastIndex){
    QList<QVariant> list = Preferences::getInstance()->pendingDownloads();
    QList<FilmDetails*> result;
    lastIndex = list.size();

    for (int i = fromIndex; i < toIndex && i < list.size(); ++i){
        QMap<QString, QVariant> filmMap = list.at(i).toMap();

        QString arteId = filmMap.value(PREFERENCES_FILMMAP_ARTEID).toString();
        QString title = filmMap.value(PREFERENCES_FILMMAP_TITLE).toString();
        QString filmUrl = filmMap.value(PREFERENCES_FILMMAP_FILMURL).toString();

        FilmDetails* film = new FilmDetails(catalogName, title, filmUrl, arteId);
        film->m_summary = filmMap.value(PREFERENCES_FILMMAP_DESC).toString();
        film->m_durationInMinutes = filmMap.value(PREFERENCES_FILMMAP_DURATION).toInt();

        film->m_episodeNumber = filmMap.value(PREFERENCES_FILMMAP_EPISODE_NUMBER).toInt();
        film->m_producingCountries = filmMap.value(PREFERENCES_FILMMAP_COUNTRIES).toStringList();

        // All metadata
        QMap<QString, QVariant> metadatas = filmMap.value(PREFERENCES_FILMMAP_METADATAS).toMap();
        foreach(QString skey, metadatas.keys()){
            int key = skey.toInt();
            film->m_metadata[static_cast<MetaType>(key)] = metadatas[skey].toString();
        }

        film->m_allStreams.insert(filmMap.value(PREFERENCES_FILMMAP_VIDEOQUALITY).toString()
                                  ,filmMap.value(PREFERENCES_FILMMAP_VIDEOURL).toString());

        QString imageUrl = filmMap.value(PREFERENCES_FILMMAP_IMAGE).toString();
        if (!imageUrl.isEmpty()) {
            requestImageDownload(film, imageUrl);
        }
        result << film;
    }
    return result;
}

QString IncompleteDownloads::getFilmDetailsUrl(FilmDetails*){
    // NOTHING TO DO
    return QString();
}

void IncompleteDownloads::processFilmDetails(FilmDetails*, QString){
    // NOTHING TO DO
}

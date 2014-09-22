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

#include "filmdelegate.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtGui>
#include <filmdetails.h>
#include <QXmlQuery>
#include <QXmlFormatter>
#include <QXmlResultItems>
#include <QAbstractNetworkCache>
#include <QDebug>
#include <QScriptEngine>
#include <QScriptValue>
#include <catalogs/icatalog.h>

#define ARTE_PLAYLIST_URL "http://videos.arte.tv/fr/videos/playlistplaylist/index--3259492.html"
#define ARTE_SEARCH_URL_FR "http://www.arte.tv/guide/fr/resultats-de-recherche?keyword=%1"
#define ARTE_SEARCH_URL_DE "http://www.arte.tv/guide/de/suchergebnisse?keyword=%1"

#define VIDEO_LINE_HTML_BEGIN "<div id=\"myPlaylistCont\">" //aaa<h2><a href=\"/fr/videos/"

#define VIDEO_URL_PREFIX "http://videos.arte.tv/fr/videos/"

#define MAPPER_STEP_DATE "DATE"
#define MAPPER_STEP_SEARCH "SEARCH"
#define MAPPER_STEP_CATALOG "CATALOG"
#define MAPPER_STEP_CODE_1_HTML "HTML"
#define MAPPER_STEP_CODE_2_XML "XML"
#define MAPPER_STEP_CODE_4_PREVIEW "PREVIEW"

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


QList<QString> FilmDelegate::listLanguages()
{
    return QList<QString>() << "fr" << "de";
}
QList<QString> FilmDelegate::listQualities()
{
    return QList<QString>() << "sq" << "hq" << "eq";
}

QList<StreamType>& FilmDelegate::listStreamTypes()
{
    static QList<StreamType> streamTypes;
    static bool first(true);
    if (first)
    {
        QString language;
        QString quality;
        foreach (language, listLanguages())
        {
            foreach (quality, listQualities())
            {
                QString humanLanguage(language);
                humanLanguage.replace(0, 1, language.left(1).toUpper());

                streamTypes << StreamType(QString("%1 %2").arg(humanLanguage, quality.toUpper()),
                                          language,
                                          quality);
            }
        }
        first = false;
    }
    return streamTypes;
}

FilmDelegate::FilmDelegate(QNetworkAccessManager * in_manager)
    :m_manager(in_manager), m_signalMapper(new QSignalMapper(this)), m_currentPageCount(0), m_lastRequestPageId(0)
{
    connect(m_signalMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(requestReadyToRead(QObject*)));
    m_currentDownloads = Preferences::getInstance()->pendingDownloads();
}

FilmDelegate::~FilmDelegate()
{
    QList<QString> pendingDownloads;
    foreach (QString dlUrl, m_currentDownloads)
    {
        FilmDetails* film = findFilmByUrl(dlUrl);
        if (film && (film->m_downloadStatus == DL_REQUESTED
                    || film->m_downloadStatus == DL_DOWNLOADING))
        {
            pendingDownloads << dlUrl;
        }
    }
    Preferences::getInstance()->setPendingDownloads(pendingDownloads);

    FilmDetails* film;
    foreach(film, m_films)
    {
        delete film;
    }

    while(!m_catalogs.empty()){
        delete m_catalogs.takeFirst();
    }
}

void FilmDelegate::loadPlayList(QString catalogName, QDate date)
{
    QString url;
    foreach(ICatalog* catalog, m_catalogs){
        url = catalog->getUrlForCatalogNames(catalogName, QDate(date)); // ui->dateEdit->date().toString("yyyyMMdd")
        if (!url.isEmpty()){
            break;
        }
    }
    if (url.isEmpty()){
        qDebug() << "No catalog found for name:" << catalogName;
        return;
    }

    abortDownloadItemsInProgress();
    QString type  = MAPPER_STEP_CATALOG;
    if (url == DOWNLOAD_STREAM)
    {
        m_visibleFilms.clear();

        foreach(QString filmUrl, m_currentDownloads) {
            if (!m_films.contains(filmUrl)) {
                addMovieFromUrl("FAKE"/* TODO */, filmUrl);
            } else {
                m_visibleFilms << filmUrl;
            }
        }
        m_currentPageCount = 1;
        emit(streamIndexLoaded(m_visibleFilms.size(), 1, m_currentPageCount));
        emit(playListHasBeenUpdated());
        return;
    } else if (url.startsWith(DATE_STREAM_PREFIX))
    {
        m_visibleFilms.clear();
        QStringList urlParts = url.split(":");
        QString language = urlParts.at(2);
        QString dateString = urlParts.at(3);
        url = QString("http://www.arte.tv/guide/%1/%2.json")
                .arg(language)
                .arg(dateString);
        type = MAPPER_STEP_DATE;

    } else if (url.startsWith(SEARCH_PREFIX))
    {
        QStringList urlParts = url.split(":");
        QString language = urlParts.at(2);
        QString search = urlParts.at(3);
        type = MAPPER_STEP_SEARCH;
        if (search.isEmpty())
        {
            m_visibleFilms.clear();
            m_currentPageCount = 1;
            emit(streamIndexLoaded(m_visibleFilms.size(), 1, m_currentPageCount));
            emit(playListHasBeenUpdated());
            return;
        }
        else if (language.toLower() == "fr"){
            url = QString(ARTE_SEARCH_URL_FR)
                .arg(search);
        }
        else {
            url = QString(ARTE_SEARCH_URL_DE)
                .arg(search);
        }
    }
    m_currentPage = 1;
    m_lastPlaylistUrl = url;
    commonLoadPlaylist(catalogName, type);
}

bool FilmDelegate::isDateCatalog(QString catalogName) const{
    QString url;
    foreach(ICatalog* catalog, m_catalogs){
        url = catalog->getUrlForCatalogNames(catalogName, QDate());
        if (!url.isEmpty()){
            return catalog->isDateCatalog();
        }
    }
    return false;
}

void FilmDelegate::loadNextPage(QString catalogName){
    if (m_currentPage >= m_currentPageCount)
        return;
    ++m_currentPage;
    abortDownloadItemsInProgress();
    commonLoadPlaylist(catalogName, m_initialyCatalog ? MAPPER_STEP_CATALOG : MAPPER_STEP_DATE);
}

void FilmDelegate::loadPreviousPage(QString catalogName){
    if (m_currentPage <= 1)
        return;
    --m_currentPage;
    abortDownloadItemsInProgress();
    commonLoadPlaylist(catalogName, m_initialyCatalog ? MAPPER_STEP_CATALOG : MAPPER_STEP_DATE);
}

void FilmDelegate::abortDownloadItemsInProgress() {
    // Aborting all deprecated download request
    QList<QNetworkReply*> oldRequests(m_manager->findChildren<QNetworkReply*>());
    foreach (QNetworkReply* old, oldRequests){
        old->abort();
        old->deleteLater();
    }
}

void FilmDelegate::commonLoadPlaylist(QString catalogName, QString type/*TODO type ne sert plus à rien*/) {
    ++m_lastRequestPageId;

    m_visibleFilms.clear();
    m_initialyCatalog = (type == MAPPER_STEP_CATALOG);
    downloadUrl(catalogName, m_lastPlaylistUrl, m_lastRequestPageId, QString(), type);
}



QString extractUniqueResult(const QString& document, const QString& xpath)
{
    QXmlQuery query;
    query.setFocus(document);
    query.setQuery(xpath);

    QXmlResultItems items;
    if (!query.isValid())
    {
        return QString();
    }

    query.evaluateTo(&items);

    QXmlItem item(items.next());
    if (item.isNull())
    {
        return QString();
    }
    QString result = item.toAtomicValue().toString();
    Q_ASSERT(items.next().isNull());
    return result;
}

void FilmDelegate::downloadUrl(const QString& catalogName, const QString& url, int requestPageId, const QString& destinationKey, const QString& step)
{
    if (requestPageId < m_lastRequestPageId)
    {
        // No need to continue, this download request is out dated
        return;
    }
    QNetworkReply* xmlReply = m_manager->get(QNetworkRequest(QUrl(url)));
    m_signalMapper->setMapping(xmlReply, new MyPair(catalogName, requestPageId, destinationKey, step));
    connect(xmlReply, SIGNAL(finished()), m_signalMapper, SLOT(map()));
}

int FilmDelegate::getFilmId(FilmDetails * film) const
{
    return m_films.values().indexOf(film);
}

void addMetadataIfNotEmpty(FilmDetails* film, QVariantMap inputMap, QString fieldName, MetaType internalFieldName, bool isDate = false)
{
    if (!inputMap.value(fieldName).isValid())
        return;
    QString value = inputMap.value(fieldName).toString();
    if (!isDate) {
        film->m_metadata.insert(internalFieldName, value);
    }
    else {
        // convert "25/10/2013 08:30:09 +0200" into "25/10/2013 08:30:09"
        film->m_metadata.insert(internalFieldName, value.left(19));
    }
}

void FilmDelegate::fetchImagesFromUrlsInPage(const QString catalogName, const QString& htmlPage,
                                             const FilmDetails * const film,
                                             const int pageRequestId)
{
    // Search for the "Photo" section in the page to skip images of related videos
    QRegExp photoSectionRegExp("#content-photo");
    int pos = photoSectionRegExp.indexIn(htmlPage, 0);

    // Search for any image file matching this URL:
    QRegExp imageRegExp("(http://www\\.arte\\.tv/papi/tvguide/images[^\"]+.jpg)");
    imageRegExp.setMinimal(true);

    QSet<QString> fetchedImage;
    while ((pos = imageRegExp.indexIn(htmlPage, pos)) != -1) {
         pos += imageRegExp.matchedLength();

        QString imageUrl = imageRegExp.capturedTexts().at(0);
        if (film->m_preview.contains(imageUrl) || fetchedImage.contains(imageUrl))
            continue;
        downloadUrl(catalogName, imageUrl, pageRequestId, film->m_infoUrl, MAPPER_STEP_CODE_4_PREVIEW);
        fetchedImage << imageUrl;
    }
}

void FilmDelegate::requestReadyToRead(QObject* object)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(m_signalMapper->mapping(object));

    if (reply == NULL || object == NULL)
        return;

    MyPair* pair = qobject_cast<MyPair* >(object) ;
    if (pair == NULL)
        return;
    QString itemName = pair->first;
    QString itemStep = pair->second;
    int pageRequestId = pair->pageRequestId;

    pair->deleteLater();
    if (pageRequestId < m_lastRequestPageId)
    {
        // This download request is out dated. No need to continue
        return;
    }

    if (itemName.isEmpty())
    {
        if (itemStep ==  MAPPER_STEP_SEARCH)
        {
            // result count: <div class="span4 mini">      <strong>
            // Current page:
            // each href of the video-placeholder
            const QString page(QString::fromUtf8(reply->readAll()));
            QRegExp regexp("<a class=\"video-placeholder\" href=\"([^\"]+)\"");
            regexp.setMinimal(true);

            int pos = 0;
            int count = 0;
            while ((pos = regexp.indexIn(page, pos)) != -1) {
                 ++count;
                 pos += regexp.matchedLength();
                 QString newUrl = regexp.cap(1);
                 if (newUrl.isEmpty()) {
                     continue;
                 }
                 if (!m_films.contains(newUrl))
                 {
                     addMovieFromUrl(pair->catalogName, newUrl);
                 }
                 else {
                     m_visibleFilms << newUrl;
                 }
             }
             emit(playListHasBeenUpdated());

        }
        else if (itemStep == MAPPER_STEP_CATALOG || itemStep == MAPPER_STEP_DATE) {
            const QString page(QString::fromUtf8(reply->readAll()));


            QScriptEngine engine;
            QScriptValue json = engine.evaluate("JSON.parse").call(QScriptValue(),
                                                                   QScriptValueList() << QString(page));
            int i = 0;
            QList<QVariant> list;
            if (itemStep == MAPPER_STEP_CATALOG)
                list = json.toVariant().toMap().value("videos").toList();
            else // MAPPER_STEP_DATE
                list = json.toVariant().toList();

            const int resultCountPerPage(Preferences::getInstance()->resultCountPerPage());
            foreach(QVariant catalogItem, list)
            {
                ++i;

                if (i > resultCountPerPage * (m_currentPage - 1) && i <= resultCountPerPage * m_currentPage) {

                    QString url = catalogItem.toMap().value(itemStep == MAPPER_STEP_CATALOG ? "url" : "details_url").toString();
                    url.prepend("http://www.arte.tv");
                    QString title = catalogItem.toMap().value("title").toString();

                    if (m_films.contains(url))
                    {
                        m_visibleFilms << url;
                        if (m_films.value(url)->m_preview.isEmpty() || m_films.value(url)->m_allStreams.isEmpty())
                        {
                            // refresh incomplete cache
                            reloadFilm(m_films.value(url));
                        }
                        // Continue with cache
                        continue;
                    }

                    FilmDetails* newFilm = new FilmDetails(pair->catalogName, title, url, /*TODO*/"Arte id ");

                    // addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_AIRDATE_LONG, First_broadcast_long);
                    // addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_RIGHTS_UNTIL, Available_until);
                    // addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_AIRDATE, First_broadcast);
                    // addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_AIRTIME, First_broadcast_time);

                    addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_DESC, Description);
                    addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIEWS, Views);
                    addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIDEO_CHANNEL, Channels);

                    m_films.insert(newFilm->m_infoUrl, newFilm);
                    m_visibleFilms << newFilm->m_infoUrl;
                    reloadFilm(newFilm);
                }
            }

            if (i % resultCountPerPage == 0) {
                m_currentPageCount = i/ resultCountPerPage;
                emit(streamIndexLoaded(i, m_currentPage, m_currentPageCount));
            }
            else
            {
                m_currentPageCount = (i / resultCountPerPage) + 1;
                emit(streamIndexLoaded(i, m_currentPage, m_currentPageCount));
            }
            emit (playListHasBeenUpdated());

        }
    }
    else
    {
        if (!m_films.contains(itemName))
            return;
        FilmDetails* film = m_films[itemName];

        if (itemStep == MAPPER_STEP_CODE_1_HTML)
        {
            const QString page(QString::fromUtf8(reply->readAll()));
            // this page is further information for the film


            // reply.url est égal à http://www.arte.tv/guide/fr/051912-001/28-minutes
            // on en extrait le code ou en faisant
            //       <input name='id' type='hidden' value=****ici se trouve l'identifiant du film***
            //    ou en le prenant de l'URL
            // Et on reconstruit l'URL ainsi :  http://org-www.arte.tv/papi/tvguide/videos/stream/player/F/{identifiant du film}_PLUS7-F/ALL/ALL.json
            QStringList splittenUrl = reply->url().toString().split('/');
            /*QRegExp regexp1("<input name='id' type='hidden' value='([0-9-]+)'");
            regexp1.setMinimal(true);
            regexp1.indexIn(page);*/
            QString filmId = splittenUrl.size() > 5 ? splittenUrl.at(5) : "";
            film->m_arteId = filmId;
            QStringList splittenCode = filmId.split("-");
            film->episodeNumber = splittenCode.value(1).toInt();

            QString languageCharacter = Preferences::getInstance()->applicationLanguage().left(1).toUpper();

            QString jsonUrl = filmId.isEmpty() ? "" : QString("http://org-www.arte.tv/papi/tvguide/videos/stream/player/%0/%1_PLUS7-%2/ALL/ALL.json")
                                                 .arg(languageCharacter)
                                                 .arg(filmId)
                                                 .arg(languageCharacter);

            if (jsonUrl.isEmpty())
            {
                emit(errorOccured(film->m_infoUrl, tr("Cannot find the main information for the movie")));
                qDebug() << "[ERROR] No json link in page" << reply->request().url().toString();
            }
            else
            {
                downloadUrl(pair->catalogName, jsonUrl, pageRequestId, film->m_infoUrl, MAPPER_STEP_CODE_2_XML);
            }


            // Fetch all images for the page
            fetchImagesFromUrlsInPage(pair->catalogName, page, film, pageRequestId);



            // jsonUrl = http://org-www.arte.tv/papi/tvguide/videos/stream/player/F/048473-089_PLUS7-F/ALL/ALL.json
        }
        else if (itemStep == MAPPER_STEP_CODE_2_XML)
        {
            const QString page(QString::fromUtf8(reply->readAll()));

            /*
             * Nouvelle version
             * Quand on lit le json de premier niveau (celui mentionné dans le HTML), c'est à dire :
             *   http://org-www.arte.tv/papi/tvguide/videos/stream/player/F/048373-005_PLUS7-F/ALL/ALL.json
             *
             *     avant l'URL json des vidéos qu'on trouvait dans le JSON de premier niveau était :
             *       http://www.arte.tv/papi/tvguide/videos/stream/F/048120-000_PLUS7-F/ALL/ALL.json
             *
             *     maintenant on ne trouve que le lien vers ce json.
             *
             * Mais il n'est pas difficile de convertir un lien vers l'autre à partir du moment où on a l'ID du film (048373-005 ou 048120-000).
             */
            if (reply->url().toString().split("/").size() < 10)
            {
                reply->deleteLater();
                return;
            }
            QString filmCode = reply->url().toString().split("/").at(9);

            QString videoStreamUrl = "http://www.arte.tv/papi/tvguide/videos/stream/";
            videoStreamUrl.append(Preferences::getInstance()->applicationLanguage() == "fr" ? "F/" : "D/");
            videoStreamUrl.append(filmCode);
            videoStreamUrl.append("/ALL/ALL.json");

            QScriptEngine engine;
            QScriptValue json = engine.evaluate("JSON.parse").call(QScriptValue(),
                                                                   QScriptValueList() << QString(page));

            QMap<QString, QVariant> mymap = json.toVariant().toMap().value("videoJsonPlayer").toMap();
            if (mymap.isEmpty())
            {
                qDebug() << "[ERROR] Cannot find 'videoJsonPlayer' for" << film->m_infoUrl << " in" << reply->request().url() << page;
                emit(errorOccured(film->m_infoUrl,tr("Cannot load stream details")));
            }
            else {

                if (mymap.value(JSON_FILMPAGE_DURATION_SECONDS).toString() != "" && film->m_title == "")
                {
                    film->m_title = mymap.value(JSON_FILMPAGE_DURATION_SECONDS).toString();
                }
                film->m_durationInMinutes = mymap.value("videoDurationSeconds").toInt() /60;
                film->m_summary = mymap.value(JSON_FILMPAGE_SUMMARY).toString();

                addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_TYPE,              Type);
                addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_FIRST_BROADCAST,   RAW_First_Broadcast, true); // 25/04/2013 20:50:30 +0200
                addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_AVAILABILITY,      RAW_Available_until, true); // 02/05/2013 20:20:30 +0200
                addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_VIDEO_TYPE,        Preview_Or_ArteP7); // EXTRAIT (AUSSCHNITT in german) or ARTE+7
                addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_VSU,               Episode_name); // if not null, it belongs to a serie
                addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_VIEWS,             Views); // different from the one in the catalog: this is just a number
                addMetadataIfNotEmpty(film, mymap, JSON_FILMPAGE_DESCRIPTION,       Description);

                QStringList labels;
                foreach (QVariant channelItem, mymap.value(JSON_FILMPAGE_CHANNELS).toList())
                {
                    labels << channelItem.toMap().value(JSON_FILMPAGE_CHANNELS_LABEL).toString();
                }
                if (!labels.isEmpty())
                {
                    film->m_metadata.insert(Channels, labels.join(", "));
                }

                if (mymap.value("videoSwitchLang").toMap().size() > 1)
                {
                    qDebug () << "[Warning] more than german and french available";
                }

                QString thumbnail = mymap.value(JSON_FILMPAGE_PREVIEW).toMap()
                        .value(JSON_FILMPAGE_PREVIEW_URL).toString();
                if (!thumbnail.isEmpty() && !film->m_preview.contains(thumbnail))
                {
                    downloadUrl(pair->catalogName, thumbnail, pageRequestId, film->m_infoUrl, MAPPER_STEP_CODE_4_PREVIEW);
                }

                foreach (QVariant streamJson, mymap.value("VSR").toMap().values()){
                    QMap<QString, QVariant> map = streamJson.toMap();
                    if (map.value("videoFormat").toString() == "HBBTV" && map.value("VQU").toString().toLower() == Preferences::getInstance()->selectedQuality())
                    {
                        film->m_allStreams[map.value("versionLibelle").toString()] = map.value("url").toString();
                    }
                }

                emit filmHasBeenUpdated(film);
            }
        }
        else if (itemStep == MAPPER_STEP_CODE_4_PREVIEW)
        {
            if (!film->m_preview.contains(reply->url().toString()))
            {
                QImage image;
                if (! reply->error()){
                    image.load(reply, "jpg");

                    if (! image.isNull())
                    {
                        film->m_preview[reply->url().toString()] = image.scaled(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                        // No need to update current image if an image has already been loaded
                        if (film->m_preview.size() == 1)
                            emit filmHasBeenUpdated(film);
                    }
                    else
                    {
                        emit(errorOccured(film->m_infoUrl,tr("Cannot load the preview image")));
                    }
                }
            }
        }
    }

    reply->deleteLater();

}

QString FilmDelegate::getStreamUrlFromResponse(const QString& page, const QString& quality)
{
    return extractUniqueResult(page, QString("/video/urls/url[@quality='%1']/string()").arg(quality));
}

StreamType FilmDelegate::getStreamTypeByHumanName(const QString& humanName) throw (NotFoundException)
{
    foreach (StreamType current, listStreamTypes())
    {
        if (current.humanCode == humanName)
            return current;
    }
    throw NotFoundException(humanName);
}


QList<int> FilmDelegate::getLineForUrl(QString filmUrl)
{
    QList<int> indexes;
    int offset(0);
    while ((offset = m_visibleFilms.indexOf(filmUrl, offset)) >= 0)
    {
        indexes << offset;
        offset++;
    }
    return indexes;
}

/**
 * @brief findFilmByUrl find the film for a given url
 * @param filmUrl url of the film description page
 * @return  the line in the view containing this film (of NULL if not found)
 */
FilmDetails* FilmDelegate::findFilmByUrl(QString filmUrl)
{
    return m_films.contains(filmUrl) ? m_films[filmUrl] : NULL;
}

QStringList FilmDelegate::listCatalogNames() const {
    QStringList list;
    foreach (ICatalog* catalog, m_catalogs){
        foreach(QString name, catalog->listSupportedCatalogNames()){
            list << name;
        }
    }
    return list;
}

double FilmDelegate::computeTotalDownloadProgress() const {
    double totalDurationInMinute(0);
    double completedDurationInMinute(0);
    foreach (QString url, m_currentDownloads){
        FilmDetails* film = m_films.value(url);
        switch (film->m_downloadStatus)
        {
        case DL_DOWNLOADING:
            totalDurationInMinute += (double) film->m_durationInMinutes;
            completedDurationInMinute += ((double)film->m_durationInMinutes * ((double) film->m_downloadProgress / 100.));
            break;
        case DL_DOWNLOADED:
            totalDurationInMinute += (double) film->m_durationInMinutes;
            completedDurationInMinute += (double) film->m_durationInMinutes;
            break;
        case DL_REQUESTED:
            totalDurationInMinute += (double) film->m_durationInMinutes;
            break;
        case DL_CANCELLED:
        case DL_ERROR:
        case DL_NONE:
        default:
            break;
        }
    }
    return totalDurationInMinute > 0 ? 100.*completedDurationInMinute/totalDurationInMinute : 0;
}

double FilmDelegate::computeTotalDownloadRequestedDuration() const {
    int totalDurationInMinute(0);
    foreach (QString url, m_currentDownloads){
        FilmDetails* film = m_films.value(url);
        switch (film->m_downloadStatus)
        {
        case DL_DOWNLOADING:
        case DL_DOWNLOADED:
        case DL_REQUESTED:
            totalDurationInMinute += film->m_durationInMinutes;
            break;
        case DL_CANCELLED:
        case DL_ERROR:
        case DL_NONE:
        default:
            break;
        }
    }
    return totalDurationInMinute;
}

StreamType FilmDelegate::getStreamTypeByLanguageAndQuality(QString languageCode, QString qualityCode) throw (NotFoundException)
{
    QList<StreamType> & streamTypeList = listStreamTypes();
    for (QList<StreamType>::const_iterator it = streamTypeList.constBegin();
         it != streamTypeList.constEnd(); ++it)
    {
        StreamType streamType = *it;
        if (streamType.languageCode.compare(languageCode) == 0
                && streamType.qualityCode.compare(qualityCode) == 0)
            return streamType;
    }
    throw NotFoundException(QString("%1/%2").arg(languageCode, qualityCode));
}

void FilmDelegate::reloadFilm(FilmDetails* film)
{
    QString videoPageUrl(film->m_infoUrl);
    // Download video web page:
    film->m_errors.clear();
    downloadUrl(film->m_catalogName, videoPageUrl, m_lastRequestPageId, film->m_infoUrl, MAPPER_STEP_CODE_1_HTML);
}

bool FilmDelegate::addMovieFromUrl(QString catalogName, const QString url, QString title)
{
    if (!url.startsWith("http://www.arte.tv/guide/"))
    {
        qWarning() << QString("Warning: cannot add %1: %2 is not compatible with ArteFetcher").arg(title, url);
        return false;
    }

    FilmDetails* film;
    if (m_films.contains(url))
    {
        film = m_films[url];
    }
    else
    {
        film = new FilmDetails(catalogName, title, url, /*TODO*/ "arteId");
        m_films.insert(film->m_infoUrl, film);
        reloadFilm(film);
    }

    m_visibleFilms << film->m_infoUrl;
    emit(playListHasBeenUpdated());

    return true;
}

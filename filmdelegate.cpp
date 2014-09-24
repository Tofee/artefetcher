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
#include <film/filmdetails.h>
#include <QXmlQuery>
#include <QXmlFormatter>
#include <QXmlResultItems>
#include <QAbstractNetworkCache>
#include <QDebug>
#include <QScriptEngine>
#include <QScriptValue>
#include <catalogs/icatalog.h>
#include "context.h"

#define MAPPER_STEP_CATALOG "CATALOG"
#define MAPPER_STEP_CODE_2_XML "XML"
#define MAPPER_STEP_CODE_4_IMAGE "PREVIEW"



FilmDelegate::FilmDelegate(QNetworkAccessManager * in_manager)
    :m_manager(in_manager), m_signalMapper(new QSignalMapper(this)), m_currentPageCount(0), m_lastRequestPageId(0)
{
    connect(m_signalMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(requestReadyToRead(QObject*)));
    m_currentDownloads = Preferences::getInstance()->pendingDownloads();
}

void FilmDelegate::addCatalog(ICatalog* catalog) {
    m_catalogs << catalog;
    connect(dynamic_cast<QObject*>(catalog), SIGNAL(requestImageDownload(FilmDetails*, QString)),
            SLOT(downloadImage(FilmDetails*, QString)));
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
    foreach(film, m_films) {
        delete film;
    }

    while(!m_catalogs.empty()) {
        delete m_catalogs.takeFirst();
    }
}

void FilmDelegate::loadPlayList(QString catalogName, QDate date)
{
    ICatalog* catalog = getCatalogForName(catalogName);
    if (!date.isValid()) {
        return;
    }
    if (!catalog) {
        // We could write a catalog dedicated to all current downloads
        // But the related movies are already in cache, thus there is no need to fetch anything
        // If the previously downloaded films are getting managed,
        // we can put a dedicated catalog which lists the films in the hard drive and fetch the related metadata
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
    }

    abortDownloadItemsInProgress();

    m_currentPage = 1;
    m_lastPlaylistUrl = catalog->getUrlForCatalogNames(catalogName, date);;
    commonLoadPlaylist(catalogName, MAPPER_STEP_CATALOG);
}

bool FilmDelegate::isDateCatalog(QString catalogName) {
    ICatalog* catalog = getCatalogForName(catalogName);
    return catalog && catalog->isDateCatalog();
}

void FilmDelegate::loadNextPage(QString catalogName){
    if (m_currentPage >= m_currentPageCount)
        return;
    ++m_currentPage;
    abortDownloadItemsInProgress();
    commonLoadPlaylist(catalogName, MAPPER_STEP_CATALOG);
}

void FilmDelegate::loadPreviousPage(QString catalogName){
    if (m_currentPage <= 1)
        return;
    --m_currentPage;
    abortDownloadItemsInProgress();
    commonLoadPlaylist(catalogName, MAPPER_STEP_CATALOG);
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

void FilmDelegate::downloadUrl(const QString& catalogName, const QString& url, int requestPageId, const QString& destinationKey, const QString& step)
{
    if (requestPageId < m_lastRequestPageId)
    {
        // No need to continue, this download request is out dated
        return;
    }
    QNetworkReply* xmlReply = m_manager->get(QNetworkRequest(QUrl(url)));
    m_signalMapper->setMapping(xmlReply, new Context(catalogName, requestPageId, destinationKey, step));
    connect(xmlReply, SIGNAL(finished()), m_signalMapper, SLOT(map()));
}

void FilmDelegate::downloadImage(FilmDetails *film, QString imageUrl){
    downloadUrl(film->m_catalogName, imageUrl, m_lastRequestPageId, film->m_infoUrl, MAPPER_STEP_CODE_4_IMAGE);
}


ICatalog* FilmDelegate::getCatalogForName(QString catalogName) {
    foreach(ICatalog* catalog, m_catalogs){
        if (catalog->accept(catalogName)){
            return catalog;
        }
    }
    return NULL;
}

void FilmDelegate::requestReadyToRead(QObject* object)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply *>(m_signalMapper->mapping(object));

    if (reply == NULL || object == NULL)
        return;

    Context* context = qobject_cast<Context* >(object) ;
    if (context == NULL)
        return;
    QString itemName = context->destinationKey;
    QString itemStep = context->step;
    int pageRequestId = context->pageRequestId;

    context->deleteLater();
    if (pageRequestId < m_lastRequestPageId)
    {
        // This download request is out dated. No need to continue
        return;
    }

    if (itemName.isEmpty())
    {
        if (itemStep == MAPPER_STEP_CATALOG) {
            const int resultCountPerPage(Preferences::getInstance()->resultCountPerPage());
            const QString page(QString::fromUtf8(reply->readAll()));

            int i;
            QList<FilmDetails*> foundInCatalogPage = getCatalogForName(context->catalogName)
                    ->listFilmsFromCatalogAnswer(context->catalogName, page, resultCountPerPage * (m_currentPage - 1), resultCountPerPage * m_currentPage, i);

            foreach(FilmDetails* film, foundInCatalogPage){
                QString url = film->m_infoUrl;
                m_visibleFilms << url;

                if (m_films.contains(url)) {
                    if (m_films.value(url)->m_preview.isEmpty() || m_films.value(url)->m_allStreams.isEmpty()) {
                        // refresh incomplete cache
                        reloadFilm(m_films.value(url));
                    }
                    // Continue with cache
                    continue;
                } else {
                    m_films.insert(film->m_infoUrl, film);
                    reloadFilm(m_films.value(url));
                }
            }

            if (i % resultCountPerPage == 0) {
                m_currentPageCount = i/ resultCountPerPage;
            }
            else {
                m_currentPageCount = (i / resultCountPerPage) + 1;
            }
            emit streamIndexLoaded(i, m_currentPage, m_currentPageCount);
            emit playListHasBeenUpdated();
        }
    }
    else
    {
        if (!m_films.contains(itemName))
            return;
        FilmDetails* film = m_films[itemName];
        if (itemStep == MAPPER_STEP_CODE_2_XML)
        {
            getCatalogForName(context->catalogName)->processFilmDetails(film, QString::fromUtf8(reply->readAll()));
            emit filmHasBeenUpdated(film);
        }
        else if (itemStep == MAPPER_STEP_CODE_4_IMAGE)
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

QList<int> FilmDelegate::getLineForUrl(QString filmUrl)
{
    QList<int> indexes;
    int offset(0);
    while ((offset = m_visibleFilms.indexOf(filmUrl, offset)) >= 0) {
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
        list << catalog->listSupportedCatalogNames();
    }
    return list;
}

double FilmDelegate::computeTotalDownloadProgress() const {
    double totalDurationInMinute(0);
    double completedDurationInMinute(0);
    foreach (QString url, m_currentDownloads) {
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

StreamType FilmDelegate::getStreamTypeByLanguageAndQuality(QString languageCode, QString qualityCode)
{
    QList<StreamType> & streamTypeList = StreamType::listStreamTypes();
    for (QList<StreamType>::const_iterator it = streamTypeList.constBegin();
         it != streamTypeList.constEnd(); ++it)
    {
        StreamType streamType = *it;
        if (streamType.languageCode.compare(languageCode) == 0
                && streamType.qualityCode.compare(qualityCode) == 0)
            return streamType;
    }
    qCritical() << "Cannot find StreamType for:" << languageCode << qualityCode;
    return StreamType();
}

void FilmDelegate::reloadFilm(FilmDetails* film)
{
    QString jsonUrl = getCatalogForName(film->m_catalogName)->getFilmDetailsUrl(film);
    if (film->m_replayAvailable && !jsonUrl.isEmpty()){
        film->m_errors.clear();
        downloadUrl(film->m_catalogName, jsonUrl, m_lastRequestPageId, film->m_infoUrl, MAPPER_STEP_CODE_2_XML);
    }
}

bool FilmDelegate::addMovieFromUrl(QString /*catalogName*/, const QString url, QString /*title*/)
{
    FilmDetails* film;
    if (m_films.contains(url)) {
        film = m_films[url];
    } else {
        // TODO
//        film = new FilmDetails(catalogName, title, url, arteId);
//        m_films.insert(film->m_infoUrl, film);
//        reloadFilm(film);
    }
    m_visibleFilms << film->m_infoUrl;
    emit(playListHasBeenUpdated());
    return true;
}

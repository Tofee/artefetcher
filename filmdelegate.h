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

#ifndef FILMDELEGATE_H
#define FILMDELEGATE_H
#include <QMap>
#include <QObject>
#include <QNetworkReply>
#include <film/filmdetails.h>
#include <QDebug>
#include <preferences.h>
#include <film/streamtype.h>

#define MAX_IMAGE_WIDTH 400
#define MAX_IMAGE_HEIGHT 226

class ICatalog;
class FilmDetails;
class QNetworkAccessManager;
class QSignalMapper;

class FilmDelegate: public QObject
{
    Q_OBJECT
public:
    FilmDelegate(QNetworkAccessManager*);
    virtual ~FilmDelegate();
    /**
     * Call this method to launch the request.
     * playlistLoaded() will be called when the page is ready.
     */
    void loadPlayList(QString catalogName, QDate date);

    bool isDateCatalog(QString catalogName);

    const QList<FilmDetails*> visibleFilms() const { QList<FilmDetails*> result;
                                              foreach(QString key, m_visibleFilms)
                                              {
                                                  result << m_films[key];
                                              }
                                                                                    return result; }

    void reloadFilm(FilmDetails* film);

    void loadPage(QString catalogName, int offset);

    /**
     * @brief getLineForUrl for an URL of a film
     * @param filmUrl url of the film description page
     * @return the lines in the view containing this film (or empty list if not found)
     */
    QList<int> getLineForUrl(QString filmUrl);

    /**
     * @brief findFilmByUrl find the film for a given url
     * @param filmUrl url of the film description page
     * @return  the line in the view containing this film (of NULL if not found)
     */
    FilmDetails* findFilmByUrl(QString filmUrl);


    void addUrlToDownloadList(QString url)
    {
        if (!m_currentDownloads.contains(url))
            m_currentDownloads << url;
    }

    QStringList downloadList() const {
        return m_currentDownloads;
    }

    void addCatalog(ICatalog* catalog);
    QStringList listCatalogNames() const;

    double computeTotalDownloadProgress() const;
    double computeTotalDownloadRequestedDuration() const;

    static StreamType getStreamTypeByLanguageAndQuality(QString languageCode, QString qualityCode);



signals:
    void playListHasBeenUpdated();
    void filmHasBeenUpdated(const FilmDetails * const film);
    void errorOccured(QString filmUrl, QString errorMessage);
    void streamIndexLoaded(int resultCount, int currentPage, int pageCount);

private slots:
    void requestReadyToRead(QObject*);
    void downloadImage(FilmDetails*, QString);

private:
    const QMap<QString, FilmDetails*> films() const { return m_films; }

    void playListLoaded(const QString page);
    /**
     * @brief downloadUrl Download a page from the url
     * @param catalogName catalog that requested the download
     * @param url remote url where the page is available
     * @param destinationKey index of the film in the map where the movie has been downloaded,<br/>
     * NULL if it's index page.
     * @param step step of the download (e.g. "HTML", "XML", "RTMP_FR_XML"...)<br/>
     * NULL if it's index page.<br/>
     * <br/>
     * This method is asynchronous. It starts the download and as soon as the page is downloaded,<br/>
     * requestReadyToRead() is called with a Context
     * TODO use an enum for step
     */
    void downloadUrl(const QString &catalogName, const QString& url, int requestPageId, const QString& destinationKey, const QString &step);

    void commonLoadPlaylist(QString catalogName);

    void abortDownloadItemsInProgress();

    ICatalog* getCatalogForName(QString catalogName);

    // List of the film description paged shown in the UI
    QList<QString> m_visibleFilms;
    QList<QString> m_currentDownloads;

    // Indexed by the url of the film description page
    QMap<QString, FilmDetails*> m_films;
    QNetworkAccessManager* m_manager;
    QSignalMapper* m_signalMapper;

    int m_currentPage;
    int m_currentPageCount;
	int m_lastRequestPageId;

    QString m_lastPlaylistUrl;

    QList<ICatalog*> m_catalogs;

};


#endif // FILMDELEGATE_H

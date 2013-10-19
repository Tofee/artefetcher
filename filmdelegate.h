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
#include <FilmDetails.h>
#include <QDebug>
#include <preferences.h>


#define MAX_IMAGE_WIDTH 400
#define MAX_IMAGE_HEIGHT 226

#define DOWNLOAD_STREAM     "about:downloads"
#define DATE_STREAM_PREFIX  "about:date:"
#define SEARCH_PREFIX       "about:search:"

class FilmDetails;
class QNetworkAccessManager;
class QSignalMapper;

class NotFoundException{
public:
    NotFoundException(QString itemSearched)
        :m_searchedItem(itemSearched)
    {
        qDebug() << "Not found exception for " << itemSearched;

    }
    QString itemSearched() const;
private:
    QString m_searchedItem;

};

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
    void loadPlayList(QString url);

    const QList<FilmDetails*> visibleFilms() const { QList<FilmDetails*> result;
                                              foreach(QString key, m_visibleFilms)
                                              {
                                                  result << m_films[key];
                                              }
                                                                                    return result; }

    void reloadFilm(FilmDetails* film);
    bool addMovieFromUrl(const QString url, QString title = QString());

    void loadNextPage();
    void loadPreviousPage();

    /**
     * @brief getLineForUrl for an URL of a film
     * @param filmUrl url of the film description page
     * @return the line in the view containing this film (or -1 if not found)
     */
    int getLineForUrl(QString filmUrl);

    /**
     * @brief findFilmByUrl find the film for a given url
     * @param filmUrl url of the film description page
     * @return  the line in the view containing this film (of NULL if not found)
     */
    FilmDetails* findFilmByUrl(QString filmUrl);


    void addUrlToDownloadList(QString url)
    {
        m_currentDownloads << url;
    }

    QStringList downloadList() const {
        return m_currentDownloads.toList();
    }

    static QString getStreamHumanName(int i);
    static QString getStreamLanguageCode(int i);
    static QString getStreamQualityCode(int i);
    static StreamType getStreamTypeByLanguageAndQuality(QString languageCode, QString qualityCode) throw (NotFoundException);
    static QList<StreamType> &listStreamTypes();
    static StreamType getStreamTypeByHumanName(const QString &humanName) throw (NotFoundException);
    static QList<QString> listLanguages();
    static QList<QString> listQualities();

signals:
    void playListHasBeenUpdated();
    void errorOccured(QString filmUrl, QString errorMessage);
    void streamIndexLoaded(int resultCount, int currentPage, int pageCount);
private slots:
    void requestReadyToRead(QObject*);

private:
    const QMap<QString, FilmDetails*> films() const { return m_films; }

    void playListLoaded(const QString page);
    /**
     * @brief downloadUrl Download a page from the url
     * @param url remote url where the page is available
     * @param destinationKey index of the film in the map where the movie has been downloaded,<br/>
     * NULL if it's index page.
     * @param step step of the download (e.g. "HTML", "XML", "RTMP_FR_XML"...)<br/>
     * NULL if it's index page.<br/>
     * <br/>
     * This method is asynchronous. It starts the download and as soon as the page is downloaded,<br/>
     * requestReadyToRead() is called with a MyPair key of the destinationKey and the step.
     * TODO use an enum for step
     */
    void downloadUrl(const QString& url, int requestPageId, const QString& destinationKey, const QString &step);
    /**
     * @brief getStreamUrlFromResponse Get the Flash stream url from the video xml page.<br/>
     * There is one page per language.
     * @param page content of the page where the flash stream url is written
     * @param quality the quality of the stream
     * @return an empty string if the page parsing fails, else the stream url
     */
    QString getStreamUrlFromResponse(const QString &page, const QString &quality);

    int getFilmId(FilmDetails*film) const;
    void commonLoadPlaylist(QString type);


    // List of the film description paged shown in the UI
    QSet<QString> m_visibleFilms;
    QSet<QString> m_currentDownloads;

    // Indexed by the url of the film description page
    QMap<QString, FilmDetails*> m_films;
    QNetworkAccessManager* m_manager;
    QSignalMapper* m_signalMapper;

    int m_currentPage;
    int m_currentPageCount;
	int m_lastRequestPageId;

    QString m_lastPlaylistUrl;
    /**
     * @brief m_initialyCatalog is true if the initial request is a catalog fetch,
     * i.e. is not a date fetch
     */
    bool m_initialyCatalog;

};

class MyPair : public QObject{
    Q_OBJECT
public:

    MyPair(int pageRequestId, QString s1, QString s2): pageRequestId(pageRequestId), first(s1), second(s2)
    {
        // qDebug() << "on en est a" << currentCountReference(true);

    }
    ~MyPair()
    {
        // qDebug() << "on en est a" << currentCountReference(false);
    }

    int pageRequestId;
    QString first;
    QString second;
private:
    int currentCountReference(bool add)
    {
        static int count(0);
        if (add)
            count++;
        else
            count --;
        return count;
    }
};

#endif // FILMDELEGATE_H

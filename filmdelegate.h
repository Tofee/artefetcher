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
    FilmDelegate(QNetworkAccessManager*, const Preferences& pref);
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

    static QString getStreamHumanName(int i);
    static QString getStreamLanguageCode(int i);
    static QString getStreamQualityCode(int i);
    static StreamType getStreamTypeByLanguageAndQuality(QString languageCode, QString qualityCode) throw (NotFoundException);
    static QList<StreamType> &listStreamTypes();
    static StreamType getStreamTypeByHumanName(const QString &humanName) throw (NotFoundException);

    void reloadFilm(FilmDetails* film);
    bool addMovieFromUrl(const QString url, QString title = QString());

    static QList<QString> listLanguages();
    static QList<QString> listQualities();

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
    void downloadUrl(const QString& url, const QString& destinationKey, const QString &step);
    /**
     * @brief getStreamUrlFromResponse Get the Flash stream url from the video xml page.<br/>
     * There is one page per language.
     * @param page content of the page where the flash stream url is written
     * @param quality the quality of the stream
     * @return an empty string if the page parsing fails, else the stream url
     */
    QString getStreamUrlFromResponse(const QString &page, const QString &quality);

    int getFilmId(FilmDetails*film) const;
    void commonLoadPlaylist();


    // List of the film description paged shown in the UI
    QList<QString> m_visibleFilms;
    QList<QString> m_currentDownloads;

    // Indexed by the url of the film description page
    QMap<QString, FilmDetails*> m_films;
    QNetworkAccessManager* m_manager;
    QSignalMapper* m_signalMapper;
    int m_currentPage;
    QString m_lastPlaylistUrl;
    const Preferences& m_preferences;


};

class MyPair : public QObject{
    Q_OBJECT
    // TODO surtout pas un truc comme ça
public:

    MyPair(QString s1, QString s2): first(s1), second(s2)
    {
        //qDebug() << "on en est a" << currentCountReference(true);

    }
    ~MyPair()
    {
        //qDebug() << "on en est a" << currentCountReference(false);
    }

    QString first;
    QString second;
private:
    int currentCountReference(bool add)
    {
        static int count(0);
        if (add) count++;
        else
             count --;
        return count;
    }
};

#endif // FILMDELEGATE_H

#include "filmdelegate.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtGui>
#include <FilmDetails.h>
#include <QXmlQuery>
#include <QXmlFormatter>
#include <QXmlResultItems>
#include <QAbstractNetworkCache>
#include <QDebug>
#include <QScriptEngine>
#include <QScriptValue>
#define ARTE_PLAYLIST_URL "http://videos.arte.tv/fr/videos/playlistplaylist/index--3259492.html"
#define VIDEO_LINE_HTML_BEGIN "<div id=\"myPlaylistCont\">" //aaa<h2><a href=\"/fr/videos/"

#define VIDEO_URL_PREFIX "http://videos.arte.tv/fr/videos/"

#define MAPPER_STEP_CATALOG "CATALOG"
#define MAPPER_STEP_CODE_1_HTML "HTML"
#define MAPPER_STEP_CODE_2_XML "XML"
#define MAPPER_STEP_CODE_3_RTMP "RTMP_XML"
#define MAPPER_STEP_CODE_4_PREVIEW "PREVIEW"
#define RESULT_PER_PAGE 10

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

FilmDelegate::FilmDelegate(QNetworkAccessManager * in_manager, const Preferences &pref)
    :m_manager(in_manager), m_signalMapper(new QSignalMapper(this)), m_preferences(pref)
{
    connect(m_signalMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(requestReadyToRead(QObject*)));
}

FilmDelegate::~FilmDelegate()
{
    FilmDetails* film;
    foreach(film, m_films)
    {
        delete film;
    }
}

void FilmDelegate::loadPlayList(QString url)
{
    if (url == "about:downloads")
    {
        m_visibleFilms.clear();
        m_visibleFilms << m_currentDownloads;
        emit(playListHasBeenUpdated());
        return;
    }
    m_currentPage = 1;
    m_lastPlaylistUrl = url;
    commonLoadPlaylist();
}


void FilmDelegate::loadNextPage(){
    ++m_currentPage;
    commonLoadPlaylist();
}
void FilmDelegate::loadPreviousPage(){
    --m_currentPage;
    commonLoadPlaylist();
}

void FilmDelegate::commonLoadPlaylist(){
    m_visibleFilms.clear();
    //emit(playListHasBeenUpdated());

    downloadUrl(m_lastPlaylistUrl, QString(), MAPPER_STEP_CATALOG);
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

void FilmDelegate::downloadUrl(const QString& url, const QString& destinationKey, const QString& step)
{
    QNetworkReply* xmlReply = m_manager->get(QNetworkRequest(QUrl(url)));
    m_signalMapper->setMapping(xmlReply, new MyPair(destinationKey, step));
    connect(xmlReply, SIGNAL(finished()), m_signalMapper, SLOT(map()));
}

int FilmDelegate::getFilmId(FilmDetails * film) const
{
    return m_films.values().indexOf(film);
}

void addMetadataIfNotEmpty(FilmDetails* film, QVariantMap inputMap, QString fieldName, MetaType internalFieldName)
{
    if (!inputMap.value(fieldName).isValid())
        return;
    QString value = inputMap.value(fieldName).toString();
    film->m_metadata.insert(internalFieldName, value);
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

    delete pair;

    if (itemName.isEmpty())
    {
        if (itemStep == MAPPER_STEP_CATALOG) {
            const QString page(QString::fromUtf8(reply->readAll()));
            QScriptEngine engine;
            QScriptValue json = engine.evaluate("JSON.parse").call(QScriptValue(),
                                                                   QScriptValueList() << QString(page));
            int i = 0;
            foreach(QVariant catalogItem, json.toVariant().toMap().value("videos").toList())
            {
                ++i;
                if (i > RESULT_PER_PAGE * (m_currentPage - 1) &&
                        i <= RESULT_PER_PAGE * m_currentPage) {
                    QString url = catalogItem.toMap().value("url").toString();
                    url.prepend("http://www.arte.tv");
                    QString title = catalogItem.toMap().value("title").toString();

                    if (m_films.contains(url))
                    {
                        m_visibleFilms << url;
                        qDebug() << "[CACHE]" << url;
                        continue;
                    }

                    FilmDetails* newFilm = new FilmDetails();
                    newFilm->m_title = title;
                    newFilm->m_infoUrl = url;

                    addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), "airdate_long", First_broadcast);
                    addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), "desc", Description);
                    addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), "video_rights_until", Available_until);
                    addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), "video_views", Views);
                    addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), "video_channels", Channels);
                    addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), "video_rank", Rank);

                    m_films.insert(newFilm->m_infoUrl, newFilm);
                    m_visibleFilms << newFilm->m_infoUrl;
                    reloadFilm(newFilm);
                }
            }

            if (i % RESULT_PER_PAGE== 0)
                emit(streamIndexLoaded(i, m_currentPage, i/ RESULT_PER_PAGE));
            else
                emit(streamIndexLoaded(i, m_currentPage, (i / RESULT_PER_PAGE) + 1));
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
            QRegExp regexp("\"([^\"]+\\.json)\"");
            regexp.setMinimal(true);
            regexp.indexIn(page);
            QString jsonUrl = regexp.cap(1);

            if (jsonUrl.isEmpty())
            {
                emit(errorOccured(film->m_infoUrl, tr("Cannot find the main information for the movie")));
                qDebug() << "[ERROR] No json link in page " << reply->request().url();
            }
            else
            {
                downloadUrl(jsonUrl, film->m_infoUrl, MAPPER_STEP_CODE_2_XML);
            }
            // jsonUrl = http://org-www.arte.tv/papi/tvguide/videos/stream/player/F/048473-089_PLUS7-F/ALL/ALL.json
        }
        else if (itemStep == MAPPER_STEP_CODE_2_XML)
        {
            const QString page(QString::fromUtf8(reply->readAll()));

            QScriptEngine engine;
            QScriptValue json = engine.evaluate("JSON.parse").call(QScriptValue(),
                                                                   QScriptValueList() << QString(page));
// // récupérer l'ID
            // http://www.arte.tv/papi/tvguide/videos/stream/F/048373-005_PLUS7-F/ALL/ALL.json l'url de stream ressemble à ça
//            url = http://org-www.arte.tv/papi/tvguide/videos/stream/player/F/048373-005_PLUS7-F/ALL/ALL.json

//            elle affiche un json qui contient cette url http://www.arte.tv/papi/tvguide/videos/stream/F/048373-005_PLUS7-F/ALL/ALL.json
            // et ce json indique cette URL http://artestras.vo.llnwxd.net/o35/nogeo/HBBTV/048373-005-A_SQ_2_VF_00122046_MP4-2200_AMM-HBBTV.mp4
            QMap<QString, QVariant> mymap = json.toVariant().toMap().value("videoJsonPlayer").toMap();
            if (mymap.isEmpty())
            {
                qDebug() << "[ERROR] Cannot find 'videoJsonPlayer' for" << film->m_infoUrl << " in" << reply->request().url() << page;
                emit(errorOccured(film->m_infoUrl,tr("Cannot load stream details")));
            }
            else {

                if (mymap.value("VTI").toString() != "" && film->m_title == "")
                {
                    film->m_title = mymap.value("VTI").toString();
                }
                film->m_durationInMinutes = mymap.value("videoDurationSeconds").toInt() /60;
                film->m_summary = mymap.value("VDE").toString();

                addMetadataIfNotEmpty(film, mymap, "VCG", Type);
                addMetadataIfNotEmpty(film, mymap, "VDA", RAW_First_Broadcast); // 25/04/2013 20:50:30 +0200
                addMetadataIfNotEmpty(film, mymap, "VRU", RAW_Available_until); // 02/05/2013 20:20:30 +0200


                if ( mymap.value("videoIsoLang").toString().left(2).toLower() == m_preferences.selectedLanguage().toLower()) {
                    downloadUrl(mymap.value("videoStreamUrl").toString(), film->m_infoUrl, QString(MAPPER_STEP_CODE_3_RTMP));
                }
                else {
                    QString expLanguage = m_preferences.selectedLanguage();
                    QString externalLanguage = QString("%1_%2").arg(expLanguage.toLower(), expLanguage.toUpper());
                    QString languageUrl = mymap.value("videoSwitchLang").toMap().value(externalLanguage).toString();
                    if (!languageUrl.isEmpty())
                        downloadUrl(languageUrl, film->m_infoUrl, QString(MAPPER_STEP_CODE_3_RTMP));
                }

                if (mymap.value("videoSwitchLang").toMap().size() > 1)
                {
                    qDebug () << "[Warning] more than german and french available";
                }
                emit(playListHasBeenUpdated());
            }
        }
        else if (itemStep == MAPPER_STEP_CODE_3_RTMP)
        {
            const QString page(QString::fromUtf8(reply->readAll()));

            QScriptEngine engine;
            QScriptValue json = engine.evaluate("JSON.parse").call(QScriptValue(),
                                                                   QScriptValueList() << QString(page));
            QMap<QString, QVariant> mymap = json.toVariant().toMap().value("video").toMap();
            foreach(QVariant stream, mymap.value("VSR").toList())
            {
                QString type = stream.toMap().value("VFO").toString();
                if (type == "HBBTV")
                {
                    QString quality = stream.toMap().value("VQU").toString().toLower();
                    if (quality == m_preferences.selectedQuality())
                        film->m_streamUrl = stream.toMap().value("VUR").toString();
                }
            }

            emit playListHasBeenUpdated();

            QString thumbnail = mymap.value("programImage").toString();
            if (!thumbnail.isEmpty())
            {
                downloadUrl(thumbnail, film->m_infoUrl, MAPPER_STEP_CODE_4_PREVIEW);
            }
            else
            {
                emit(errorOccured(film->m_infoUrl,tr("Cannot find the preview image")));
            }
        }
        else if (itemStep == MAPPER_STEP_CODE_4_PREVIEW)
        {
            film->m_preview.load(reply,"jpg");
            if (! film->m_preview.isNull())
            {
                film->m_preview = film->m_preview.scaled(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT, Qt::KeepAspectRatio);
                emit playListHasBeenUpdated();
            }
            else
            {
                emit(errorOccured(film->m_infoUrl,tr("Cannot load the preview image")));
            }
        }
    }

    reply->deleteLater();

}
// TODO y'a plein de NEW sur les my pair, et ils ne sont pas forcément delete

QString FilmDelegate::getStreamUrlFromResponse(const QString& page, const QString& quality)
{
    return extractUniqueResult(page, QString("/video/urls/url[@quality='%1']/string()").arg(quality));
}

StreamType FilmDelegate::getStreamTypeByHumanName(const QString& humanName) throw (NotFoundException)
{
    StreamType current("","","");// TODO c'est pas beau
    foreach (current, listStreamTypes())
    {
        if (current.humanCode == humanName)
            return current;
    }
    throw NotFoundException(humanName);
}


int FilmDelegate::getLineForUrl(QString filmUrl)
{
    return m_visibleFilms.indexOf(filmUrl);
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
    downloadUrl(videoPageUrl, film->m_infoUrl, MAPPER_STEP_CODE_1_HTML);
}

bool FilmDelegate::addMovieFromUrl(const QString url, QString title)
{
    if (!url.startsWith("http://www.arte.tv/guide/fr/"))
        return false;

    FilmDetails* film;
    if (m_films.contains(url))
    {
        film = m_films[url];
        qDebug() << url << "is already in cache";
    }
    else
    {
        film = new FilmDetails();
        film->m_title = title;
        film->m_infoUrl = url;
        m_films.insert(film->m_infoUrl, film);
        reloadFilm(film);
    }

    m_visibleFilms << film->m_infoUrl;
    emit(playListHasBeenUpdated());

    return true;
}

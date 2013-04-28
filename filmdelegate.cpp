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
#define MAPPER_STEP_CODE_3_RTMP "RTMP_XML_"
#define MAPPER_STEP_CODE_4_PREVIEW "PREVIEW"
#define RESULT_PER_PAGE 15

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
    :m_manager(in_manager), m_signalMapper(new QSignalMapper(this))
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
    QMap<QString, FilmDetails*> oldFilms(m_films);
    m_films.clear();
    downloadUrl(m_lastPlaylistUrl, QString(), MAPPER_STEP_CATALOG);
    foreach(FilmDetails* film, oldFilms)
    {
        delete film;
    }
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
                    addMovieFromUrl(url, title);
                }
            }

            if (i % RESULT_PER_PAGE== 0)
                emit(streamIndexLoaded(i, m_currentPage, i/ RESULT_PER_PAGE));
            else
                emit(streamIndexLoaded(i, m_currentPage, (i / RESULT_PER_PAGE) + 1));

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
            QRegExp regexp("\"([^\"]+.json)\"");
            regexp.setMinimal(true);
            regexp.indexIn(page);
            QString jsonUrl = regexp.cap(1);

            downloadUrl(jsonUrl, film->m_url, MAPPER_STEP_CODE_2_XML);
        }
        else if (itemStep == MAPPER_STEP_CODE_2_XML)
        {
            const QString page(QString::fromUtf8(reply->readAll()));

            QScriptEngine engine;
            QScriptValue json = engine.evaluate("JSON.parse").call(QScriptValue(),
                                                                   QScriptValueList() << QString(page));

            QMap<QString, QVariant> mymap = json.toVariant().toMap().value("videoJsonPlayer").toMap();

            film->m_title = mymap.value("VTI").toString();
            film->m_durationInMinutes = mymap.value("videoDurationSeconds").toInt() /60;
            film->m_summary = mymap.value("VDE").toString();

            // TODO gérer la liste des langues dynamiquement, en plus on n'est pas forcément sur FR en première langue
            downloadUrl(mymap.value("videoStreamUrl").toString(), film->m_url, QString(MAPPER_STEP_CODE_3_RTMP).append("fr"));
            downloadUrl(mymap.value("videoSwitchLang").toMap().value("de_DE").toString(), film->m_url, QString(MAPPER_STEP_CODE_3_RTMP).append("de"));
            if (mymap.value("videoSwitchLang").toMap().size() > 1)
            {
                qDebug () << "Warning, more than german and french available";
            }
            emit(playListHasBeenUpdated());
        }
        else if (itemStep.startsWith(MAPPER_STEP_CODE_3_RTMP))
        {
            const QString page(QString::fromUtf8(reply->readAll()));

            QString language = itemStep.mid(QString(MAPPER_STEP_CODE_3_RTMP).length());

            QScriptEngine engine;
            QScriptValue json = engine.evaluate("JSON.parse").call(QScriptValue(),
                                                                   QScriptValueList() << QString(page));
            QMap<QString, QVariant> mymap = json.toVariant().toMap().value("video").toMap();
            foreach(QVariant stream, mymap.value("VSR").toList())
            {
                QString type = stream.toMap().value("VFO").toString();
                if (type == "HBBTV")
                {
                    QString quality = stream.toMap().value("VQU").toString();
                    StreamType streamType = getStreamTypeByLanguageAndQuality(language, quality.toLower());
                    film->m_streamsByType[streamType].m_rtmpStreamUrl = stream.toMap().value("VUR").toString();
                }
            }

            emit playListHasBeenUpdated();

            QString thumbnail = mymap.value("programImage").toString();
            if (!thumbnail.isEmpty())
            {
                downloadUrl(thumbnail, film->m_url, MAPPER_STEP_CODE_4_PREVIEW);
            }
            else
            {
                emit(errorOccured(getFilmId(film),tr("Cannot find the preview image")));
            }
        }
        else if (itemStep == MAPPER_STEP_CODE_4_PREVIEW)
        {
            film->m_preview.load(reply,"jpg");
            if (! film->m_preview.isNull())
            {
                film->m_preview = film->m_preview.scaled(600,340);
                emit playListHasBeenUpdated();
            }
            else
            {
                emit(errorOccured(getFilmId(film),tr("Cannot load the preview image")));
            }
        }
    }

    reply->deleteLater();

    // TODO plutot qu'utiliser des signals mapper on peut surement mettre une property (ou +) dans la reply
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
    QString videoPageUrl(film->m_url);
    // Download video web page:
    downloadUrl(videoPageUrl, film->m_url, MAPPER_STEP_CODE_1_HTML);
}

bool FilmDelegate::addMovieFromUrl(const QString url, QString title)
{
    if (!url.startsWith("http://www.arte.tv/guide/fr/"))
        return false;

    QString internalUrl = url.mid(QString("http://www.arte.tv/guide/fr/").length());

    if (m_films.contains(internalUrl))
        return false;

    FilmDetails* newFilm = new FilmDetails();
    newFilm->m_title = title;
    newFilm->m_url = url;
    m_films.insert(newFilm->m_url, newFilm);
    reloadFilm(newFilm);
    return true;
}

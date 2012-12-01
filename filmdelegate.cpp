#include "filmdelegate.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QNetworkReply>
#include <QtGui>
#include <FilmDetails.h>
#include <QXmlQuery>
#include <QXmlFormatter>
#include <QXmlResultItems>
#define ARTE_PLAYLIST_URL "http://videos.arte.tv/fr/videos/playlistplaylist/index--3259492.html"
#define VIDEO_LINE_HTML_BEGIN "<h2><a href=\"/fr/videos/"

#define VIDEO_URL_PREFIX "http://videos.arte.tv/fr/videos/"

#define MAPPER_STEP_CODE_1_HTML "HTML"
#define MAPPER_STEP_CODE_2_XML "XML"
#define MAPPER_STEP_CODE_3_RTMP "RTMP_XML_"
#define MAPPER_STEP_CODE_4_PREVIEW "PREVIEW"
#define MAPPER_STEP_CODE_5_REMOVE "REMOVE"

QList<QString> FilmDelegate::listLanguages()
{
    return QList<QString>() << "fr" << "de";
}
QList<QString> FilmDelegate::listQualities()
{
    return QList<QString>() << "sd" << "hd";
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

void FilmDelegate::loadPlayList()
{
    FilmDetails* film;
    foreach(film, m_films)
    {
        delete film;
    }
    m_films.clear();
    downloadUrl(ARTE_PLAYLIST_URL, "", "");
}

void FilmDelegate::playListLoaded(const QString page)
{

    static int beginTagLength = QString(VIDEO_LINE_HTML_BEGIN).length();

    int currentPos=0;
    while (currentPos >=0 && currentPos < page.length())
    {
        int beginIndex = page.indexOf(VIDEO_LINE_HTML_BEGIN, currentPos);
        int endIndex = page.indexOf("</a>", beginIndex);

        if (beginIndex>0 && endIndex >0)
        {
            const QString line = page.mid(beginIndex + beginTagLength, endIndex - beginIndex - beginTagLength);
            const QStringList lineInfos = line.split("\">");
            if (lineInfos.size()==2)
            {
                FilmDetails* details = new FilmDetails();
                details->m_title = lineInfos.at(1);
                details->m_url = lineInfos.at(0);
                QString titleInUrl = details->m_url.left(details->m_url.indexOf(QRegExp("-")));
                QString regExpText("/fr/do_removeFromPlaylist/videos/playlistplaylist/%1[^']+");
                regExpText = regExpText.arg(titleInUrl);
                QRegExp removeRegExp(regExpText);
                removeRegExp.indexIn(page);
                details->m_removeUrl = removeRegExp.cap(0);
                m_films.insert(details->m_url, details);
            }

            currentPos = endIndex + 1;

        }
        else
        {
            break;
        }
    }

    emit playListHasBeenUpdated();

    // Now we load each film in details
    FilmDetails* film;
    foreach(film, m_films.values())
    {

        QString videoPageUrl(film->m_url);
        videoPageUrl.prepend("http://videos.arte.tv/fr/videos/");
        // Download video web page:
        downloadUrl(videoPageUrl, film->m_url, MAPPER_STEP_CODE_1_HTML);
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
        // this page is the playlist
        const QString page(QString::fromUtf8(reply->readAll()));
        playListLoaded(page);
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
            QStringList javascriptLines = page.split(";");

            QString playerUrl = javascriptLines.filter("url_player").value(0).split("\"").value(1);
            if (playerUrl.isEmpty())
            {
                emit(errorOccured(getFilmId(film),tr("Cannot find the player url of this video")));
            }
            else{
                film->m_flashPlayerUrl = playerUrl;
            }

            QString xmlVideoUrl = javascriptLines.filter("vars_player.videorefFileUrl").value(0).split("\"").value(1);
            if (xmlVideoUrl.isEmpty())
            {
                emit (errorOccured(getFilmId(film),tr("Cannot find the XML page about all the videos")));
            }
            else
            {
                downloadUrl(xmlVideoUrl, film->m_url, "XML");
            }

            if (film->m_title.isEmpty())
            {
                QRegExp titleRegExp("<title>([^<]+) - videos\\.arte\\.tv</title>");
                titleRegExp.setMinimal(true);
                titleRegExp.indexIn(page);
                film->m_title = titleRegExp.cap(1);
            }

            QRegExp summaryRegExp("<div class=.recentTracksCont.>\\W?<div>\\W?<p>(.+)</p>");
            summaryRegExp.setMinimal(true);
            summaryRegExp.indexIn(page);
            film->m_summary = summaryRegExp.cap(1);

            {
                // Note: this part is not always available
                QRegExp durationYearAndCountry("<div id=\"more\">\\W?<p style=\"margin-top: 0\">\\(([^)]+)\\mn\\)");
                durationYearAndCountry.setMinimal(true);
                durationYearAndCountry.indexIn(page);
                QStringList fields = durationYearAndCountry.cap(1).split(", ");
                if (fields.size()> 2)
                {
                    bool valid;
                    film->m_countries = fields.mid(0, fields.size()-2);

                    int year = fields.at(fields.size()-2).toInt(&valid);
                    if (valid)
                    {
                        film->m_year = year;
                    }
                    int duration = fields.at(fields.size()-1).toInt(&valid);
                    if (valid)
                    {
                        film->m_durationInMinutes = duration;
                    }
                }
            }

            emit playListHasBeenUpdated();
        }
        else if (itemStep == MAPPER_STEP_CODE_2_XML)
        {
            const QString page(QString::fromUtf8(reply->readAll()));

            QString language;
            foreach (language, listLanguages())
            {
                QString result = extractUniqueResult(page, QString("/videoref/videos/video[@lang='%1']/@ref/string()").arg(language));

                if (!result.isEmpty())
                {
                    downloadUrl(result, film->m_url, QString(MAPPER_STEP_CODE_3_RTMP).append(language));
                }
                else
                {
                    emit(errorOccured(getFilmId(film),tr("Cannot find the movies for language: %1").arg(language)));
                }
            }
        }
        else if (itemStep.startsWith(MAPPER_STEP_CODE_3_RTMP))
        {
            const QString page(QString::fromUtf8(reply->readAll()));

            QString language = itemStep.mid(QString(MAPPER_STEP_CODE_3_RTMP).length());

            bool changed = false;
            QString quality;
            foreach (quality, listQualities())
            {
                QString streamUrl = getStreamUrlFromResponse(page, quality);

                if (streamUrl.isEmpty())
                {
                    emit(errorOccured(getFilmId(film),tr("Cannot find the video stream in %1 %2").arg(language, quality)));
                    continue;
                }

                try {
                    StreamType streamType = getStreamTypeByLanguageAndQuality(language, quality);

                    if (!film->m_streamsByType.contains(streamType))
                    {
                        Stream s;
                        s.use = false;
                        s.downloaded = false;
                        film->m_streamsByType.insert(streamType, s);
                    }
                    film->m_streamsByType[streamType].m_rtmpStreamUrl = streamUrl;

                } catch(NotFoundException)
                {
                    // NOTHING TO DO
                }
            }

            bool valid = false;

            if (film->m_numberOfViews == 0)
            {
                ushort numberOfViews = extractUniqueResult(page, "/video/numberOfViews/string()")
                        .toUShort(&valid);
                if (valid)
                {
                    changed = true;
                    film->m_numberOfViews = numberOfViews;
                }
                else
                {
                    emit(errorOccured(getFilmId(film),tr("Cannot find the number of views")));
                }
            }

            if (film->m_rating == 0)
            {
                double rating = extractUniqueResult(page, "/video/rating/string()").toDouble(&valid);
                if (valid)
                {
                    changed = true;
                    film->m_rating = rating;
                }
                else
                {
                    emit(errorOccured(getFilmId(film),tr("Cannot find the rating")));
                }
            }

            if (changed)
                 emit playListHasBeenUpdated();

            QString thumbnail = extractUniqueResult(page, "/video/firstThumbnailUrl/string()");
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
                film->m_preview = film->m_preview.scaled(160,90);
                emit playListHasBeenUpdated();
            }
            else
            {
                emit(errorOccured(getFilmId(film),tr("Cannot load the preview image")));
            }
        }
        else if (itemStep == MAPPER_STEP_CODE_5_REMOVE)
        {
            const QString page(QString::fromUtf8(reply->readAll()));
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
    videoPageUrl.prepend("http://videos.arte.tv/fr/videos/");
    // Download video web page:
    downloadUrl(videoPageUrl, film->m_url, MAPPER_STEP_CODE_1_HTML);
}

bool FilmDelegate::addMovieFromUrl(const QString url)
{
    if (!url.startsWith(VIDEO_URL_PREFIX))
        return false;

    QString internalUrl = url.mid(QString(VIDEO_URL_PREFIX).length());

    if (m_films.contains(internalUrl))
        return false;

    FilmDetails* newFilm = new FilmDetails();
    newFilm->m_title = "";
    newFilm->m_url = internalUrl;
    m_films.insert(newFilm->m_url, newFilm);
    reloadFilm(newFilm);
    return true;
}

void FilmDelegate::removeFilm(FilmDetails *film)
{
    QString urlString("http://videos.arte.tv");
    urlString.append(film->m_removeUrl);
    downloadUrl(urlString, film->m_url, MAPPER_STEP_CODE_5_REMOVE);
}

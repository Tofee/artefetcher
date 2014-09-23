#include "artedatecatalog.h"
#include <QDate>
#include <QDebug>//TODO remove that ugly thing
#include <QScriptEngine>
#include <preferences.h>

ArteDateCatalog::ArteDateCatalog(QObject *parent)
    :QObject(parent)
{
    m_urlByCatalogName.insert(tr("By date"),"::arteDate::");
    // No need to provide a real url, there is only one url for dates and the final url
    // will be built according to the given date
}


QString ArteDateCatalog::getUrlForCatalogNames(QString, QDate catalogDate) const {
    QString baseUrl("http://www.arte.tv/papi/tvguide/epg/schedule/F/L3/%1-%2-%3/%4-%5-%6.json");
    return baseUrl.arg(catalogDate.year())
                .arg(catalogDate.month(), 2, 10, QChar('0'))
                .arg(catalogDate.day(), 2, 10, QChar('0'))
                .arg(catalogDate.year())
                .arg(catalogDate.month())
                .arg(catalogDate.day());
}

QList<FilmDetails*> ArteDateCatalog::listFilmsFromCatalogAnswer(QString catalogName, const QString &catalogAnswer, int fromIndex, int toIndex, int &lastIndex)
{
    QList<FilmDetails*> result;

    QScriptEngine engine;
    QScriptValue json = engine.evaluate("JSON.parse").call(QScriptValue(),
                                                           QScriptValueList() << QString(catalogAnswer));
    int i = -1;
    QList<QVariant> list = json.toVariant().toMap().value("abstractBroadcastList").toList();

    foreach(QVariant catalogItem, list)
    {
        ++i;

        if (i >= fromIndex && i < toIndex) {
            QString url = catalogItem.toMap().value("PUR").toString();
            QString title = catalogItem.toMap().value("TIT").toString();
            QString arteId = catalogItem.toMap().value("PID").toString();

            FilmDetails* newFilm = new FilmDetails(catalogName, title, url, arteId);

            addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), "DLO", Description);
            //TODO addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIEWS, Views);
            // TODO addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIDEO_CHANNEL, Channels);

            newFilm->m_replayAvailable = catalogItem.toMap().value("VDO").toMap().value("VTY").toString() == QString("ARTE_PLUS_SEVEN");

            result << newFilm;

            QString imageUrl = catalogItem.toMap().value("VDO").toMap().value("programImage").toString();
            // Other possibility IMG/IUR but it seems the format is not always valid.
            if (!imageUrl.isEmpty()){
                emit requestImageDownload(newFilm, imageUrl);
            }
        }
    }

    lastIndex = i+1;// TODO c'est moche!!!
    return result;
}

QString ArteDateCatalog::getFilmDetailsUrl(FilmDetails *film){
    QString languageCharacter = Preferences::getInstance()->applicationLanguage().left(1).toUpper();

    QString jsonUrl = film->m_arteId.isEmpty() ? "" : QString("http://org-www.arte.tv/papi/tvguide/videos/stream/player/%0/%1_PLUS7-%2/ALL/ALL.json")
                                         .arg(languageCharacter)
                                         .arg(film->m_arteId)
                                         .arg(languageCharacter);
    return jsonUrl;
}

void ArteDateCatalog::processFilmDetails(FilmDetails *film, QString httpAnswer){
    // TODO c'est un vulgaire copier coller depuis ArteMainCatalog
    QScriptEngine engine;
    QScriptValue json = engine.evaluate("JSON.parse").call(QScriptValue(),
                                                           QScriptValueList() << QString(httpAnswer));

    QMap<QString, QVariant> mymap = json.toVariant().toMap().value("videoJsonPlayer").toMap();
    if (mymap.isEmpty())
    {
        qDebug() << "[ERROR] Cannot find 'videoJsonPlayer' for" << film->m_infoUrl << " in" << httpAnswer;
        //TODO emit(errorOccured(film->m_infoUrl,tr("Cannot load stream details")));
        return;
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
            // TODO estce vraiment utile ?
            emit requestImageDownload(film, thumbnail);
        }

        foreach (QVariant streamJson, mymap.value("VSR").toMap().values()){
            QMap<QString, QVariant> map = streamJson.toMap();
            if (map.value("videoFormat").toString() == "HBBTV" && map.value("VQU").toString().toLower() == Preferences::getInstance()->selectedQuality())
            {
                film->m_allStreams[map.value("versionLibelle").toString()] = map.value("url").toString();
            }
        }
    }

}

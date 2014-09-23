#include "artelivecatalog.h"
#include <preferences.h>
#include <QScriptEngine>
#include <QScriptValue>
#include <filmdetails.h>
#include <QDebug> // TODO

ArteLiveCatalog::ArteLiveCatalog(QObject *parent) : QObject(parent)
{
    m_urlByCatalogName.insert(tr("Live - Selection"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/selection");
    m_urlByCatalogName.insert(tr("Live - Classical"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/classic");
    m_urlByCatalogName.insert(tr("Live - Rock"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/rock");
    m_urlByCatalogName.insert(tr("Live - Jazz"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/jazz");
    m_urlByCatalogName.insert(tr("Live - World"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/world");
    m_urlByCatalogName.insert(tr("Live - Dance"), "http://www.arte.tv/concert/" +  Preferences::getInstance()->applicationLanguage() + "/api/hbbtv/dance");
}

QList<FilmDetails*> ArteLiveCatalog::listFilmsFromCatalogAnswer(QString catalogName, const QString& catalogAnswer, int fromIndex, int toIndex, int& lastIndex){
    QList<FilmDetails*> result;

    QScriptEngine engine;
    QScriptValue json = engine.evaluate("JSON.parse").call(QScriptValue(),
                                                           QScriptValueList() << QString(catalogAnswer));
    int i = -1;
    QList<QVariant> list = json.toVariant().toMap().value("videos").toList();

    foreach(QVariant catalogItem, list)
    {
        ++i;

        if (i >= fromIndex && i < toIndex) {
            QString url = catalogItem.toMap().value("VTR").toString(); // or VUP
            QString title = catalogItem.toMap().value("VTI").toString();
            QString arteId = catalogItem.toMap().value("VPI").toString();

            FilmDetails* newFilm = new FilmDetails(catalogName, title, url, arteId);

            addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), "VDE", Description);
            addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), "VCG", Genre);
            //addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIEWS, Views);
            //addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIDEO_CHANNEL, Channels);

            foreach (QVariant streamJson, catalogItem.toMap().value("VSR").toMap().values()){
                QMap<QString, QVariant> map = streamJson.toMap();
                if (map.value("VQU").toString().toLower() == Preferences::getInstance()->selectedQuality())
                {
                    newFilm->m_allStreams[map.value("versionLibelle").toString()] = map.value("url").toString();
                }
            }

            result << newFilm;

            QString imageUrl = catalogItem.toMap().value("VTU").toMap().value("IUR").toString();
            if (!imageUrl.isEmpty()){
                emit requestImageDownload(newFilm, imageUrl);
            }
        }
    }

    lastIndex = i+1;// TODO c'est moche!!!
    return result;
}

QString ArteLiveCatalog::getFilmDetailsUrl(FilmDetails*){
    // Nothing to do here. Everything is fetched in listFilmsFromCatalogAnswer
    return QString();
}

void ArteLiveCatalog::processFilmDetails(FilmDetails*, QString){
    // Nothing to do here. Everything is fetched in listFilmsFromCatalogAnswer
}

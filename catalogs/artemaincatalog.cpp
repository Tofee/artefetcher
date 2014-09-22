#include "artemaincatalog.h"
#include <preferences.h>
#include <QScriptEngine>
#include <QScriptValue>
#include <filmdetails.h>
#include <QDebug> // TODO

ArteMainCatalog::ArteMainCatalog(QObject *parent)
    :QObject(parent)
{
    m_urlByCatalogName[tr("Arte selection")] = "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7/selection.json";
    m_urlByCatalogName[tr("Most recent")] = "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7/plus_recentes.json";
    m_urlByCatalogName[tr("Most seen")] = "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7/plus_vues.json";
    m_urlByCatalogName[tr("Last chance")] = "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7/derniere_chance.json";
    // TODO je commente parce qu'il est trop volumineux
    // m_urlByCatalogName[tr("All")] = "http://www.arte.tv/guide/"+ Preferences::getInstance()->applicationLanguage() + "/plus7.json";
}


QList<FilmDetails*> ArteMainCatalog::listFilmsFromCatalogAnswer(QString catalogName, const QString& catalogAnswer, int fromIndex, int toIndex, int& lastIndex){


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
            QString url = catalogItem.toMap().value("url").toString();
            url.prepend("http://www.arte.tv");
            QString title = catalogItem.toMap().value("title").toString();
            QString arteId = catalogItem.toMap().value("em").toString();

            FilmDetails* newFilm = new FilmDetails(catalogName, title, url, arteId);

            addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_DESC, Description);
            addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIEWS, Views);
            addMetadataIfNotEmpty(newFilm, catalogItem.toMap(), JSON_VIDEO_CHANNEL, Channels);

            qDebug() << "Ajout de " << newFilm->title();
            result << newFilm;
        }
    }

    lastIndex = i+1;// TODO c'est moche!!!
    return result;
}

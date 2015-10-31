#include <catalogs/icatalog.h>
#include <QScriptEngine>
#include <preferences.h>
#include "artedefinitions.h"

void addMetadataIfNotEmpty(FilmDetails *film, QVariantMap inputMap, QString fieldName, MetaType internalFieldName, bool isDate) {
    if (!inputMap.value(fieldName).isValid())
        return;
    QString value = inputMap.value(fieldName).toString();
    if (!isDate) {
        film->m_metadata.insert(internalFieldName, value);
    }
    else {
        // convert "25/10/2013 08:30:09 +0200" into "25/10/2013 08:30:09"
        film->m_metadata.insert(internalFieldName, value.left(19));
    }
}

QMap<QString, QVariant> extractJsonMapFromAnswer(QString htmlPage){
    QScriptEngine engine;
    QScriptValue json = engine.evaluate("JSON.parse").call(QScriptValue(),
                                                           QScriptValueList() << QString(htmlPage));
    return json.toVariant().toMap();
}

QList<FilmDetails*> ICatalog::listFilmsFromCatalogAnswer(QString catalogName, const QString& catalogAnswer, int fromIndex, int toIndex, int& lastIndex){
    QString programListProperty = QString("program%1List").arg( Preferences::getInstance()->applicationLanguage().toUpper());
    QList<QVariant> list = extractJsonMapFromAnswer(catalogAnswer).value(programListProperty).toList();
    lastIndex = list.size();
    QList<FilmDetails*> result;

    for (int i = fromIndex; i < toIndex && i < list.size(); ++i){
        QMap<QString, QVariant> catalogItem = list.at(i).toMap();
        QString url = catalogItem.value("videoStreamUrl").toString();
        QString title = catalogItem.value("TIT").toString();
        QString arteId = catalogItem.value("PID").toString();

        FilmDetails* newFilm = new FilmDetails(catalogName, title, url, arteId);
        updateArteEpisodeNumber(newFilm);
        addMetadataIfNotEmpty(newFilm, catalogItem, JSON_DESC, Description);
        addMetadataIfNotEmpty(newFilm, catalogItem, JSON_VIEWS, Views);
        addMetadataIfNotEmpty(newFilm, catalogItem, JSON_VIDEO_CHANNEL, Channels);

        result << newFilm;

        QString imageUrl = catalogItem.value("IMG").toMap().value("IUR").toString();
        if (!imageUrl.isEmpty()){
            emit requestImageDownload(newFilm, imageUrl);
        }
    }
    return result;
}

#include "artemaincatalog.h"
#include <preferences.h>

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


QList<FilmDetails*> ArteMainCatalog::listFilmsFromCatalogAnswer(QString catalogAnswer){
    qDebug() << catalogAnswer;
    return QList<FilmDetails*>();
}

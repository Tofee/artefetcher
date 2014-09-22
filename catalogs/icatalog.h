#ifndef ICATALOG_H
#define ICATALOG_H
#include <QStringList>
#include <QMap>
#include <QDate>
#include <QVariant>

#define JSON_AIRDATE        "airdate"
#define JSON_AIRDATE_LONG   "airdate_long"
#define JSON_AIRTIME        "airtime"
#define JSON_DESC           "desc"
#define JSON_RIGHTS_UNTIL   "video_rights_until"
#define JSON_VIEWS          "video_views"
#define JSON_VIDEO_CHANNEL  "video_channels"
#define JSON_RANK           "video_rank"

#include <filmdetails.h>
class ICatalog {
public:

    virtual ~ICatalog(){}

    QStringList listSupportedCatalogNames() const {
        return m_urlByCatalogName.keys();
    }

    /**
     * @brief Returns the url for a such catalog name if this catalog name is supported
     * @param catalogName name of the catalog
     * @param catalogDate date of the catalog
     * @return the url of the catalog if supported, otherwise a default constructed QString
     */
    virtual QString getUrlForCatalogNames(QString catalogName, QDate /*catalogDate*/) const {
        return m_urlByCatalogName.value(catalogName);
    }

    /**
     * @brief listFilmsFromCatalogAnswer
     * @param catalogAnswer
     * @param fromIndex first item to fetch
     * @param toIndex last item to fetch (included)
     * @return
     */
    virtual QList<FilmDetails*> listFilmsFromCatalogAnswer(QString catalogName, const QString& catalogAnswer, int fromIndex, int toIndex, int& finalIndex) = 0;

    virtual bool isDateCatalog() const { return false; }

    /**
     * @brief accept
     * @param catalogName
     * @return true if the catalog manage the catalogName
     */
    virtual bool accept(QString catalogName) const { return m_urlByCatalogName.contains(catalogName); }

    // TODO bouger dans un fichier d'implem !!
    void addMetadataIfNotEmpty(FilmDetails* film, QVariantMap inputMap, QString fieldName, MetaType internalFieldName, bool isDate = false)
    {
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

protected:
    QMap<QString, QString> m_urlByCatalogName;
};

#endif // ICATALOG_H

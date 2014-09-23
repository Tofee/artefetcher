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

#define JSON_FILMPAGE_TYPE              "VCG"
#define JSON_FILMPAGE_FIRST_BROADCAST   "VDA"
#define JSON_FILMPAGE_AVAILABILITY      "VRU"
#define JSON_FILMPAGE_VIDEO_TYPE        "VTX"
#define JSON_FILMPAGE_VSU               "VSU"
#define JSON_FILMPAGE_VIEWS             "VVI"
// #define JSON_FILMPAGE_RANK              "videoRank"// useless, it just gives the position in the carousel...
#define JSON_FILMPAGE_DESCRIPTION       "V7T"
#define JSON_FILMPAGE_DURATION_SECONDS  "VTI"
#define JSON_FILMPAGE_SUMMARY           "VDE"
#define JSON_FILMPAGE_CHANNELS          "VCH"
#define JSON_FILMPAGE_CHANNELS_LABEL    "label"
#define JSON_FILMPAGE_PREVIEW           "VTU"
#define JSON_FILMPAGE_PREVIEW_URL       "IUR"

#include <filmdetails.h>
class ICatalog {
public:

    virtual ~ICatalog(){}


    /**
     * @brief accept
     * @param catalogName
     * @return true if the catalog manage the catalogName
     */
    virtual bool accept(QString catalogName) const { return m_urlByCatalogName.contains(catalogName); }

    virtual bool isDateCatalog() const { return false; }

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

    // TODO renommer la méthode
    virtual QString fetchFilmDetails(FilmDetails* film) = 0;

    virtual void processFilmDetails(FilmDetails* film, QString httpAnswer) = 0;


protected:
    void addMetadataIfNotEmpty(FilmDetails* film, QVariantMap inputMap, QString fieldName, MetaType internalFieldName, bool isDate = false);
    QMap<QString, QString> m_urlByCatalogName;
};

#endif // ICATALOG_H

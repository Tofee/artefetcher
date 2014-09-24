#ifndef ICATALOG_H
#define ICATALOG_H
#include <QStringList>
#include <QMap>
#include <QDate>
#include <QVariant>


#include <film/filmdetails.h>
class ICatalog {
public:

    virtual ~ICatalog(){}

// TODO super doc
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

    virtual QString getFilmDetailsUrl(FilmDetails* film) = 0;

    virtual void processFilmDetails(FilmDetails* film, QString httpAnswer) = 0;

    QMap<QString, QVariant> extractJsonMapFromAnswer(QString httpAnswer);

protected:
    void addMetadataIfNotEmpty(FilmDetails* film, QVariantMap inputMap, QString fieldName, MetaType internalFieldName, bool isDate = false);
    QMap<QString, QString> m_urlByCatalogName;
};

#endif // ICATALOG_H

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
     * @brief listFilmsFromCatalogAnswer List the films of the catalog in the range of the page
     * @param catalogName name of the catalog
     * @param catalogAnswer html answer page
     * @param fromIndex index of the first item in the page
     * @param toIndex index of the last item in the page (excluded)
     * @param totalCount count of all the items in the catalog (even if not in the current page)
     * @return list of the items in the page
     */
    virtual QList<FilmDetails*> listFilmsFromCatalogAnswer(QString catalogName, const QString& catalogAnswer, int fromIndex, int toIndex, int& totalCount) = 0;

    virtual QString getFilmDetailsUrl(FilmDetails* film) = 0;

    virtual void processFilmDetails(FilmDetails* film, QString httpAnswer) = 0;

protected:

    QMap<QString, QString> m_urlByCatalogName;
};

void addMetadataIfNotEmpty(FilmDetails* film, QVariantMap inputMap, QString fieldName, MetaType internalFieldName, bool isDate = false);
QMap<QString, QVariant> extractJsonMapFromAnswer(QString httpAnswer);

#endif // ICATALOG_H

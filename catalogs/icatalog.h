#ifndef ICATALOG_H
#define ICATALOG_H
#include <QStringList>
#include <QMap>
#include <QDate>
#include <QVariant>
#include <film/filmdetails.h>

/**
 * @brief The ICatalog class
 * Implement this interface if you want to add additionnal catalog to arteFetcher.
 *
 * Laziest implemention aims at:
 *   - adding all catalog names and url in m_urlByCatalogName in the constructor (the catalog name should be in english and translatable (use tr("name of the catalog"))
 *   - implements listFilmsFromCatalogAnswer()
 *   - implements getFilmDetailsUrl()
 *   - implements processFilmDetails()
 *   - declare your catalog in MainWindow::MainWindow()
 *
 * The workflow is following :
 *   1 - arteFetcher calls listSupportedCatalogNames() for each catalog, this fills the dropdown item in main view
 *   2 - once user selects a catalog:
 *          2.1 - the appropriate catalog class is the first catalog class answering accept(catalogName) == true
 *          2.2 - isDateCatalog() is called to check whether this catalog supports a date format. This updates accordingly the visibility of the date input in main window
 *          2.3 - getUrlForCatalogNames() returns the url where the film list is available
 *          2.4 - once we get the html reply of getUrlForCatalogNames(), the html reply is sent to listFilmsFromCatalogAnswer() which will return the list of films in the catalog
 *          2.5 - then, for each film in the catalog:
 *              2.5.1 - getFilmDetailsUrl() is called to know where to get further information about the film
 *              2.5.2 - once the html result of getFilmDetailsUrl() has been retrieved, it's sent to processFilmDetails()
 *
 *   When user clicks on "reload film details", step 2.5 is executed again.
 *
 * There is some mandatory data to retrieve and to store in FilmDetails class:
 *   - arteId should be unique
 *   - url should be unique
 *   - title
 * Highly recommended fields:
 *   - description
 *   - thumbnail of the film
 *
 * See the rest of the documentation for more advanced usages.
 */
class ICatalog {
public:
    virtual ~ICatalog(){}

    /**
     * @brief This method indicates if the object is the owner of the catalogName.
     * That means the catalog object can be requested to list all films of this catalogName
     * @see QStringList listSupportedCatalogNames() const
     * @param catalogName catalog name to check
     * @return true if the catalog manages the catalogName
     */
    virtual bool accept(QString catalogName) const { return m_urlByCatalogName.contains(catalogName); }

    /**
     * @brief isDateCatalog is a date catalog
     * @return true if the film list result depends on a date parameter
     */
    virtual bool isDateCatalog() const { return false; }

    /**
     * @brief listSupportedCatalogNames list all the catalog names supported
     * When a catalogName is supported, accept(catalogName) should return true
     * A catalog name should be unique among all catalogs !
     * @see accept(QString catalogName) const
     * @return list of all catalog names supported
     */
    QStringList listSupportedCatalogNames() const {
        return m_urlByCatalogName.keys();
    }

    /**
     * @brief Returns the url for a such catalog name if this catalog name is supported
     * @param catalogName name of the catalog
     * @param catalogDate date of the catalog (used if the catalog is a date catalog)
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
    virtual QList<FilmDetails*> listFilmsFromCatalogAnswer(QString catalogName, const QString& catalogAnswer, int fromIndex, int toIndex, int& totalCount);

    /**
     * @brief getFilmDetailsUrl Some catalogs needs to fetch an extra webpage to get more details of
     * the film. This is the purpose of this method. Provide the extra webpage URL here and the answer
     * will be provided in processFilmDetails
     * @see processFilmDetails(FilmDetails* film, QString htmlPage)
     * @param film film with incomplete details
     * @return the URL to fetch more details about film, or "" if nothing more has to be fetched
     */
    virtual QString getFilmDetailsUrl(FilmDetails* film) = 0;

    /**
     * @brief processFilmDetails enriches the film with the html response of the getFilmDetailsUrl(FilmDetails*)
     * @see getFilmDetailsUrl(FilmDetails*)
     * @param film film to enrich
     * @param htmlPage additionnal information for film (html page of getFilmDetailsUrl())
     */
    virtual void processFilmDetails(FilmDetails* film, QString htmlPage) = 0;

protected:
    // SIGNAL:
    virtual void requestImageDownload(FilmDetails* film, QString imageUrl) = 0;

    /**
     * @brief m_urlByCatalogName This is a list of the url indexed by their catalog name.
     * Most of implementation of ICatalog will use default implementation of listSupportedCatalogNames()
     * and getUrlForCatalogNames(). To use this implementation, just fill m_urlByCatalogName adding all the
     * catalogs you wish with the urls providing the film list.
     */
    QMap<QString, QString> m_urlByCatalogName;
};

/**
 * @brief addMetadataIfNotEmpty Convenient function. It extracts from the inputMap the value of key fieldName and adds it to the film
 * into the metadata named internalFieldName
 * @param film film to enrich
 * @param inputMap map having the metadata input
 * @param fieldName field name to get in the inputMap
 * @param internalFieldName metadata field name to put in film
 * @param isDate true if the expected value is a date (format: "25/10/2013 08:30:09 +0200")
 */
void addMetadataIfNotEmpty(FilmDetails* film, QVariantMap inputMap, QString fieldName, MetaType internalFieldName, bool isDate = false);

/**
 * @brief extractJsonMapFromAnswer Convenient function. It extracts the json map from an HTML page
 * That uses QtScript
 * @param htmlPage html page
 * @return a conversion of the html page into a json map
 */
QMap<QString, QVariant> extractJsonMapFromAnswer(QString htmlPage);

#endif // ICATALOG_H

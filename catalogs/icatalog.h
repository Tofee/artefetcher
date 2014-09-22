#ifndef ICATALOG_H
#define ICATALOG_H
#include <QStringList>
#include <QMap>
#include <QDate>

class FilmDetails;

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

    virtual QList<FilmDetails*> listFilmsFromCatalogAnswer(QString catalogAnswer) = 0;

    virtual bool isDateCatalog(){ return false; }

protected:
    QMap<QString, QString> m_urlByCatalogName;
};

#endif // ICATALOG_H

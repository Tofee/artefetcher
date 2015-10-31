#ifndef ARTEDATECATALOG_H
#define ARTEDATECATALOG_H
#include <catalogs/icatalog.h>

class ArteDateCatalog : public QObject, public ICatalog
{
    Q_OBJECT
public:
    ArteDateCatalog(QObject* parent);

    bool isDateCatalog() const { return true; }

    QString getUrlForCatalogNames(QString, QDate catalogDate) const;

    QString getFilmDetailsUrl(FilmDetails* film);

    void processFilmDetails(FilmDetails* film, QString htmlPage);
signals:
    void requestImageDownload(FilmDetails* film, QString imageUrl);
};

#endif // ARTEDATECATALOG_H

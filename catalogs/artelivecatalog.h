#ifndef ARTELIVECATALOG_H
#define ARTELIVECATALOG_H
#include <catalogs/icatalog.h>

class ArteLiveCatalog : public QObject, public ICatalog
{
    Q_OBJECT
public:
    ArteLiveCatalog(QObject* parent);
    QList<FilmDetails*> listFilmsFromCatalogAnswer(QString catalogName, const QString& catalogAnswer, int fromIndex, int toIndex, int& index);

    QString getFilmDetailsUrl(FilmDetails* film);

    void processFilmDetails(FilmDetails* film, QString htmlPage);
signals:
    void requestImageDownload(FilmDetails* film, QString imageUrl);
};

#endif // ARTELIVECATALOG_H

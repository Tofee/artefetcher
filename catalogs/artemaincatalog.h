#ifndef ARTEMAINCATALOG_H
#define ARTEMAINCATALOG_H
#include <catalogs/icatalog.h>

class ArteMainCatalog : public QObject, public ICatalog
{
    Q_OBJECT

public:
    ArteMainCatalog(QObject* parent);

    QList<FilmDetails*> listFilmsFromCatalogAnswer(QString catalogName, const QString& catalogAnswer, int fromIndex, int toIndex, int& index);

    QString getFilmDetailsUrl(FilmDetails* film);

    void processFilmDetails(FilmDetails* film, QString htmlPage);
signals:
    void requestImageDownload(FilmDetails* film, QString imageUrl);
};

#endif // ARTEMAINCATALOG_H

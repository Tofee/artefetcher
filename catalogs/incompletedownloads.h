#ifndef INCOMPLETEDOWNLOADS_H
#define INCOMPLETEDOWNLOADS_H
#include <catalogs/icatalog.h>
#define INCOMPLETE_DOWNLOADS_URL "::incomplete::"


class IncompleteDownloads : public QObject, public ICatalog
{
    Q_OBJECT
public:
    IncompleteDownloads(QObject* parent);
    QList<FilmDetails*> listFilmsFromCatalogAnswer(QString catalogName, const QString& catalogAnswer, int fromIndex, int toIndex, int& index);

    QString getFilmDetailsUrl(FilmDetails* film);

    void processFilmDetails(FilmDetails* film, QString httpAnswer);
signals:
    void requestImageDownload(FilmDetails* film, QString imageUrl);
};

#endif // INCOMPLETEDOWNLOADS_H

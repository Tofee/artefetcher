#ifndef ARTEMAINCATALOG_H
#define ARTEMAINCATALOG_H
#include <catalogs/icatalog.h>

class ArteMainCatalog : public QObject, public ICatalog
{
    Q_OBJECT

public:
    ArteMainCatalog(QObject* parent);

    QList<FilmDetails*> listFilmsFromCatalogAnswer(QString catalogAnswer);
};

#endif // ARTEMAINCATALOG_H

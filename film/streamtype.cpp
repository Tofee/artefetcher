#include <film/streamtype.h>
#include <QList>

QList<QString> StreamType::listLanguages()
{
    return QList<QString>() << "fr" << "de";
}
QList<QString> StreamType::listQualities()
{
    return QList<QString>() << "sq" << "hq" << "eq";
}

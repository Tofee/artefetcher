#include <film/streamtype.h>
#include <QList>

QList<StreamType>& StreamType::listStreamTypes()
{
    static QList<StreamType> streamTypes;
    static bool first(true);
    if (first)
    {
        QString language;
        QString quality;
        foreach (language, listLanguages())
        {
            foreach (quality, listQualities())
            {
                QString humanLanguage(language);
                humanLanguage.replace(0, 1, language.left(1).toUpper());

                streamTypes << StreamType(QString("%1 %2").arg(humanLanguage, quality.toUpper()),
                                          language,
                                          quality);
            }
        }
        first = false;
    }
    return streamTypes;
}


QList<QString> StreamType::listLanguages()
{
    return QList<QString>() << "fr" << "de";
}
QList<QString> StreamType::listQualities()
{
    return QList<QString>() << "sq" << "hq" << "eq";
}

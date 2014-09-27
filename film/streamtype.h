#ifndef STREAMTYPE_H
#define STREAMTYPE_H
#include <QString>
#include <QList>

class StreamType {

public:
    StreamType(){}

    static QList<QString> listLanguages();
    static QList<QString> listQualities();

};


#endif // STREAMTYPE_H

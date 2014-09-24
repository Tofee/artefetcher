#ifndef STREAMTYPE_H
#define STREAMTYPE_H
#include <QString>
#include <QList>

class StreamType {

public:
    StreamType(){}
    StreamType(StreamType& other)
        :humanCode(other.humanCode), languageCode(other.languageCode), qualityCode(other.qualityCode)
    {}
    StreamType(const StreamType& other)
        :humanCode(other.humanCode), languageCode(other.languageCode), qualityCode(other.qualityCode)
    {}
    virtual ~StreamType(){}

    StreamType(const QString& in_humanCode, const QString& in_languageCode, const QString& in_qualityCode)
        :humanCode(in_humanCode), languageCode(in_languageCode), qualityCode(in_qualityCode)
    {}

    bool operator==(const StreamType& other) const
    {
        return other.humanCode == humanCode &&
                other.languageCode == languageCode &&
                other.qualityCode == qualityCode;
    }
    int operator<(const StreamType& other) const
    {
        if (other.humanCode == humanCode &&
                other.languageCode == languageCode &&
                other.qualityCode == qualityCode)
            return 0;
        return -1;
    }

    static QList<StreamType> &listStreamTypes();
    static QList<QString> listLanguages();
    static QList<QString> listQualities();

    QString humanCode;
    QString languageCode;
    QString qualityCode;
};


#endif // STREAMTYPE_H

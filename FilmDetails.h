#ifndef FILMDETAILS_H
#define FILMDETAILS_H
#include <QString>
#include <QImage>
#include <QMap>
#include <QStringList>

struct StreamType {

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

    QString humanCode;
    QString languageCode;
    QString qualityCode;
};
//Q_DECLARE_METATYPE ( StreamType );

class Stream
{
    //TODO mettre tout en privé sauf les accesseurs
public:
    bool use;
    bool downloaded;
    QString m_rtmpStreamUrl;
    // Full path of the future downloaded film.
    QString m_targetFileName;
};

class FilmDetails {
    //TODO mettre tout en privé sauf les accesseurs
public:
    QString m_title;
    QString m_flashPlayerUrl;
    QImage m_preview;
    QString m_summary;
    // TODO last availability date
    double m_rating;
    uint m_numberOfViews;
    QString m_url;
    QString m_removeUrl;
    QStringList m_countries;
    int m_durationInMinutes;
    int m_year;
    bool m_isDownloading;

    QMap<StreamType, Stream> m_streamsByType;


    QString title() const { return m_title; }
};

#endif // FILMDETAILS_H

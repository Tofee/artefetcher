#ifndef HTTPCONTEXT_H
#define HTTPCONTEXT_H
#include <QObject>

class Context : public QObject{
    Q_OBJECT
public:

    Context(QString catalogName, int pageRequestId, QString destinationKey, QString step)
        : catalogName(catalogName), pageRequestId(pageRequestId), destinationKey(destinationKey), step(step)
    {
    }
    ~Context()
    {
    }

    const QString catalogName;
    const int pageRequestId;
    const QString destinationKey;
    const QString step;
};

#endif // HTTPCONTEXT_H

#ifndef RTMPTHREAD_H
#define RTMPTHREAD_H

#include <QThread>
#include <QObject>
#include <QMap>
#include <QString>
#include <FilmDetails.h>

class FilmDetails;
class RTMPThread : public QThread
{
    Q_OBJECT
public:
    explicit RTMPThread(const QMap<int, FilmDetails>, QObject *parent = 0);
    void run();
signals:
    void updateProgress(int id, StreamType streamTypeId, double progress, int speed);
    void filmDownloaded(int filmId, StreamType streamTypeId);
public slots:
private:
    QMap<int, FilmDetails> m_details;
};



#endif // RTMPTHREAD_H

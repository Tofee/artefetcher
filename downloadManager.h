#ifndef RTMPTHREAD_H
#define RTMPTHREAD_H


#include <queuedownloader.h>
#include <FilmDetails.h>

class QNetworkAccessManager;




class DownloadManager : public QObject{
    Q_OBJECT
public:
DownloadManager(QObject *parent);

void addFilmToDownloadQueue(QString key, const FilmDetails& details);

int queueSize() const;

signals:
    void signalAllFilmDownloadFinished();
    void signalDownloadProgressed(QString key,double,double, double);
    void signalDownloadFinished(QString key);

private slots:
    void downloadProgressed(QString url, qint64 loadedSize, qint64 totalSize, double kbytesPerSecond, double remainingTimeInSecond);

    void downloadFinished(QString url);

    void downloadError(QString url, QString message);

    void allDownloadsFinished();


private:
    // Keys sent in emitted signals, indexed by their URL
    // here indexed by the URL of the movie page
    QMap<QString, QString> m_keysForSignalByUrl;
    QueueDownloader m_downloader;
};


#endif // RTMPTHREAD_H

#ifndef RTMPTHREAD_H
#define RTMPTHREAD_H


#include <queuedownloader.h>
#include <FilmDetails.h>

class QNetworkAccessManager;




class DownloadManager : public QObject{
    Q_OBJECT
public:
DownloadManager(const QMap<int, FilmDetails> details, QObject *parent);

void addFilmToDownloadQueue(int filmId, const FilmDetails& details);

int queueSize() const;

signals:
    void allFilmDownloadFinished();
    void downloadProgressed(int,double,double, double);
    void downloadFinished(int);

private slots:
    void downloadProgressed(QString url, qint64 loadedSize, qint64 totalSize, double kbytesPerSecond, double remainingTimeInSecond);

    void downloadFinished(QString url);

    void downloadError(QString url, QString message);

    void allDownloadsFinished();


private:
    // Keys sent in emitted signals, indexed by their URL
    QMap<QString, int> m_keysForSignalByUrl;
    QueueDownloader m_downloader;
};


#endif // RTMPTHREAD_H

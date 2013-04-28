#ifndef RTMPTHREAD_H
#define RTMPTHREAD_H

#include <QThread>
#include <QObject>
#include <QMap>
#include <QString>
#include <FilmDetails.h>
#include <QSignalMapper>
#include <QNetworkReply>
#include <QFile>
#include <QQueue>
#include <QPair>
#include <QMap>
#include <QTimer>
#include <QDebug>
#include <QTime>

class FilmDetails;
class QNetworkAccessManager;

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QObject *parent = 0)
        :m_currentDownload(NULL),m_manager(new QNetworkAccessManager(parent)), m_isWorking(false)
    {
    }

    void addDownload(QUrl url, QString filename) {
        if (m_pendingDonwloads.isEmpty() && ! m_isWorking)
                 QTimer::singleShot(0, this, SLOT(startNextDownload()));

        QPair<QUrl, QString> newDownload(url, filename);
        m_pendingDonwloads.enqueue(newDownload);

    }

    int queueSize() const {
        return m_pendingDonwloads.size();
    }


private slots:
    void startNextDownload(){
        m_isWorking = true;
        m_downloadTime.restart();

        if (m_pendingDonwloads.isEmpty()){
            emit(allDownloadsFinished());
            m_isWorking = false;
            return;
        }
        QPair<QUrl, QString> downloadToStart = m_pendingDonwloads.dequeue();

        m_outputFile.setFileName(downloadToStart.second);

        if (!m_outputFile.open(QIODevice::WriteOnly|QIODevice::Append)) {
            emit(downloadError(downloadToStart.first.toString(),
            QString("Problem opening save file '%1' for download '%2': %3\n").arg(
                    downloadToStart.second,
                    downloadToStart.first.toString(),
                    m_outputFile.errorString())));
            return;                 // skip this download
        }

        QNetworkRequest request(downloadToStart.first);

        m_previouslyDownloaded = 0;
        if (m_outputFile.size() > 0){
            m_previouslyDownloaded = m_outputFile.size();
            QByteArray rangeHeaderValue = "bytes=" + QByteArray::number( m_outputFile.size()) + "-";
            request.setRawHeader("Range", rangeHeaderValue);
        }
        m_currentDownload = m_manager->get(request);

        connect(m_currentDownload, SIGNAL(readyRead()),                      SLOT(downloadReadyRead()));
        connect(m_currentDownload, SIGNAL(downloadProgress(qint64,qint64)),  SLOT(downloadProgressed(qint64,qint64)));
        connect(m_currentDownload, SIGNAL(finished()),                       SLOT(downloadFinished()));
    }

    void downloadProgressed(qint64 bytesReceived, qint64 totalSize){
        if (m_lastNotifTime.elapsed() < 800 /* ms */)
            return;
        m_lastNotifTime.restart();
        m_isWorking = true;
        double bytesPerSecond = bytesReceived * 1000.0 / m_downloadTime.elapsed();
        double kbytesPerSecond = bytesPerSecond / 1000.0;
        double remainingTimeInSecond = (totalSize - bytesReceived) / bytesPerSecond;
        emit(downloadProgressed(m_currentDownload->url().toString(), m_previouslyDownloaded + bytesReceived, totalSize+m_previouslyDownloaded, kbytesPerSecond, remainingTimeInSecond));
    }

    void downloadFinished()
    {
        m_outputFile.close();

        if (m_currentDownload->error()) {
             emit(downloadError(m_currentDownload->url().toString(), QString("Failed: %1\n").arg(m_currentDownload->errorString())));
         } else {
             emit(downloadFinished(m_currentDownload->url().toString()));
         }

         m_currentDownload->deleteLater();
         startNextDownload();
    }

    void downloadReadyRead() {
        if (m_previouslyDownloaded >= m_currentDownload->header(QNetworkRequest::ContentLengthHeader).toLongLong())
        {
            m_currentDownload->disconnect();// prevent any error message
            m_currentDownload->abort();
            emit(downloadFinished(m_currentDownload->url().toString()));
        }
        else {
            m_outputFile.write(m_currentDownload->readAll());
        }
    }


signals:
    void downloadProgressed(QString url, qint64 loadedSize, qint64 totalSize, double kbytesPerSecond, double remainingTimeInSecond);
    void downloadFinished(QString url);
    void downloadError(QString url, QString message);
    void allDownloadsFinished();
private:

    QNetworkReply* m_currentDownload;
    QNetworkAccessManager* m_manager;
    QQueue<QPair<QUrl, QString> > m_pendingDonwloads;
    QFile m_outputFile;
    QTime m_downloadTime;
    QTime m_lastNotifTime;
    bool m_isWorking;
    qint64 m_previouslyDownloaded;
};


class RTMPThread : public QObject{
    Q_OBJECT
public:
RTMPThread(const QMap<int, FilmDetails> details, QObject *parent)
        : m_downloader(parent)
    {
    QMap<int, FilmDetails>::const_iterator it;
        for (it  = details.constBegin(); it != details.constEnd(); ++it)

        {
            addFilmToDownloadQueue(it.key(), it.value());
        }
        connect(&m_downloader, SIGNAL(downloadProgressed(QString,qint64,qint64, double, double)), SLOT(downloadProgressed(QString,qint64,qint64, double, double)));
        connect(&m_downloader, SIGNAL(downloadFinished(QString)), SLOT(downloadFinished(QString)));
        connect(&m_downloader, SIGNAL(downloadError(QString,QString)), SLOT(downloadError(QString,QString)));
        connect(&m_downloader, SIGNAL(allDownloadsFinished()), SLOT(allDownloadsFinished()));
    }

void addFilmToDownloadQueue(int filmId, const FilmDetails& details){

        if (details.m_hasBeenRequested)
        {

        m_downloader.addDownload(details.m_streamUrl, details.m_targetFileName);
        m_keysForSignalByUrl.insert(details.m_streamUrl, filmId);
        }
}
int queueSize() const {
    return m_downloader.queueSize();
}

signals:
    void allFilmDownloadFinished();
    void downloadProgressed(int,double,double, double);
    void downloadFinished(int);

private slots:
    void downloadProgressed(QString url, qint64 loadedSize, qint64 totalSize, double kbytesPerSecond, double remainingTimeInSecond){
        int id = m_keysForSignalByUrl.value(url);
        emit(downloadProgressed(id, 100.0 * loadedSize/ (double) totalSize, kbytesPerSecond, remainingTimeInSecond));
    }

    void downloadFinished(QString url)
    {
        int id = m_keysForSignalByUrl.value(url);
        emit(downloadFinished(id));
    }

    void downloadError(QString url, QString message)
    {
        qDebug() << "Download error" << url << message;
    }

    void allDownloadsFinished(){
        emit(allFilmDownloadFinished());
    }



private:
    // Keys sent in emitted signals, indexed by their URL
    QMap<QString, int> m_keysForSignalByUrl;
    Downloader m_downloader;
};


#endif // RTMPTHREAD_H

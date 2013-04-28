#ifndef QUEUEDOWNLOADER_H
#define QUEUEDOWNLOADER_H
#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <QFile>
#include <QQueue>
#include <QPair>
#include <QTime>

class QueueDownloader : public QObject
{
    Q_OBJECT
public:
    explicit QueueDownloader(QObject *parent = 0);

    void addDownload(QUrl url, QString filename) ;

    int queueSize() const;

private slots:
    void startNextDownload();

    void downloadProgressed(qint64 bytesReceived, qint64 totalSize);

    void downloadFinished();

    void downloadReadyRead() ;


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

#endif // QUEUEDOWNLOADER_H

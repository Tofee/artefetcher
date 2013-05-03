#include <downloadManager.h>


#include <QDebug>

DownloadManager::DownloadManager(QObject *parent)
        : m_downloader(parent)
    {

        connect(&m_downloader, SIGNAL(downloadProgressed(QString,qint64,qint64, double, double)), SLOT(downloadProgressed(QString,qint64,qint64, double, double)));
        connect(&m_downloader, SIGNAL(downloadFinished(QString)), SLOT(downloadFinished(QString)));
        connect(&m_downloader, SIGNAL(downloadError(QString,QString)), SLOT(downloadError(QString,QString)));
        connect(&m_downloader, SIGNAL(allDownloadsFinished()), SLOT(allDownloadsFinished()));
    }

void DownloadManager::addFilmToDownloadQueue(QString key, const FilmDetails& details){

        if (details.m_hasBeenRequested)
        {

        m_downloader.addDownload(details.m_streamUrl, details.m_targetFileName);
        m_keysForSignalByUrl.insert(details.m_streamUrl, key);
        }
}
int DownloadManager::queueSize() const {
    return m_downloader.queueSize();
}

void DownloadManager::downloadProgressed(QString url, qint64 loadedSize, qint64 totalSize, double kbytesPerSecond, double remainingTimeInSecond){
    QString key = m_keysForSignalByUrl.value(url);
    emit(signalDownloadProgressed(key, 100.0 * loadedSize/ (double) totalSize, kbytesPerSecond, remainingTimeInSecond));
}

void DownloadManager::downloadFinished(QString url)
{
    QString key = m_keysForSignalByUrl.value(url);
    emit(signalDownloadFinished(key));
}

void DownloadManager::downloadError(QString url, QString message)
{
    qDebug() << "Download error" << url << message;
}

void DownloadManager::allDownloadsFinished(){
    emit(signalAllFilmDownloadFinished());
}



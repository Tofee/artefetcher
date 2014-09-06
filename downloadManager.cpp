/****************************************************************************

    This file is part of ArteFetcher.

    ArteFetcher is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ArteFetcher is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ArteFetcher.  If not, see <http://www.gnu.org/licenses/>.
    
****************************************************************************/
#include <downloadManager.h>


#include <QDebug>

DownloadManager::DownloadManager(QObject *parent)
        : m_downloader(parent)
    {

        connect(&m_downloader, SIGNAL(downloadProgressed(QString,qint64,qint64, double, double)), SLOT(downloadProgressed(QString,qint64,qint64, double, double)));
        connect(&m_downloader, SIGNAL(downloadFinished(QString)), SLOT(downloadFinished(QString)));
        connect(&m_downloader, SIGNAL(downloadError(QString,QString)), SLOT(downloadError(QString,QString)));
        connect(&m_downloader, SIGNAL(allDownloadsFinished()), SLOT(allDownloadsFinished()));
        connect(&m_downloader, SIGNAL(paused()), SIGNAL(hasBeenPaused()));
        connect(&m_downloader, SIGNAL(downloadCancelled(QString)), SLOT(downloadCancelled(QString)));
    }

void DownloadManager::addFilmToDownloadQueue(const QString &key, const QString &remoteUrl, const QString &targetFileName){
    m_downloader.addDownload(remoteUrl, targetFileName);
    m_keysForSignalByUrl.insert(remoteUrl, key);
}

void DownloadManager::cancelDownloadInProgress() {
    m_downloader.cancelDownloadInProgress();
}

void DownloadManager::cancelDownload(QString key){
    QString streamUrl = m_keysForSignalByUrl.key(key);
    if (streamUrl.isEmpty())
    {
        return;
    }
    m_downloader.cancelDownload(streamUrl);
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
    QString key = m_keysForSignalByUrl.value(url);
    emit(signalDownloadError(key, message));
    qDebug() << "Download error for " << url << ": " << message;
}

void DownloadManager::allDownloadsFinished(){
    emit(signalAllFilmDownloadFinished());
}

void DownloadManager::pause() {
    m_downloader.pause();
}

void DownloadManager::downloadCancelled(QString url) {
    QString key = m_keysForSignalByUrl.value(url);
    emit(signalDownloadCancelled(key));
}

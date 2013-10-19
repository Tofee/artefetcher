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

#include "queuedownloader.h"

#include <downloadManager.h>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

QueueDownloader::QueueDownloader(QObject *parent)
    :m_currentDownload(NULL),m_manager(new QNetworkAccessManager(parent)), m_lastNotifTime(0,0), m_isWorking(false), m_isPaused(false)
{
}

void QueueDownloader::addDownload(QUrl url, QString filename) {
    if (m_pendingDownloads.isEmpty() && ! m_isWorking)
             QTimer::singleShot(0, this, SLOT(startNextDownload()));

    QPair<QUrl, QString> newDownload(url, filename);
    m_pendingDownloads.enqueue(newDownload);

}

int QueueDownloader::queueSize() const {
    return m_pendingDownloads.size();
}

void QueueDownloader::startNextDownload(){
    if (m_isPaused)
        return;

    m_isWorking = true;
    m_downloadTime.restart();

    if (m_pendingDownloads.isEmpty()){
        emit(allDownloadsFinished());
        m_isWorking = false;
        return;
    }
    QPair<QUrl, QString> downloadToStart = m_pendingDownloads.dequeue();

    // Check the destination directory
    QFileInfo destFileInfo(downloadToStart.second);
    if (!destFileInfo.absoluteDir().exists() && ! QDir("/").mkpath(destFileInfo.absolutePath()))
    {
        emit(downloadError(downloadToStart.first.toString(),
                           tr("Cannot create the working directory %1").arg(destFileInfo.absolutePath())));
        return;
    }


    m_outputFile.setFileName(QString(downloadToStart.second).append(TEMP_FILE_PREFIX));

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


// AJOUT FreddyP 14/10/2013 :  m_lastNotifTime.elapsed() est toujours égal à zéro au démarrage du téléchargement
//                             Il faut faire un restart dans ce cas pour l'initialiser correctement.
    m_lastNotifTime.restart();

    connect(m_currentDownload, SIGNAL(readyRead()),                      SLOT(downloadReadyRead()));
    connect(m_currentDownload, SIGNAL(downloadProgress(qint64,qint64)),  SLOT(downloadProgressed(qint64,qint64)));
    connect(m_currentDownload, SIGNAL(finished()),                       SLOT(downloadFinished()));
}

void QueueDownloader::downloadProgressed(qint64 bytesReceived, qint64 totalSize){
    if (m_lastNotifTime.elapsed() < 100 /* ms */)       // Modif FreddyP 14/10/2013 : Tempo à 100 MS au lieu de 800 MS sinon pas de jauge sous windows
        return;
    m_lastNotifTime.restart();
    m_isWorking = true;
    double bytesPerSecond = bytesReceived * 1000.0 / m_downloadTime.elapsed();
    double kbytesPerSecond = bytesPerSecond / 1000.0;
    double remainingTimeInSecond = (totalSize - bytesReceived) / bytesPerSecond;
    emit(downloadProgressed(m_currentDownload->url().toString(), m_previouslyDownloaded + bytesReceived, totalSize+m_previouslyDownloaded, kbytesPerSecond, remainingTimeInSecond));
}

void QueueDownloader::downloadFinished()
{
    m_outputFile.close();

    if (m_currentDownload->error()) {
        if (!m_isPaused && m_currentDownload->errorString() != "Operation canceled") {
            qDebug() << m_currentDownload->errorString();
            emit(downloadError(m_currentDownload->url().toString(), QString("Failed: %1").arg(m_currentDownload->errorString())));
        }
    } else {
        // Remove .part extension
        QFileInfo info(m_outputFile.fileName());
        m_outputFile.rename(info.absolutePath() + QDir::separator() + info.completeBaseName());
        emit(downloadFinished(m_currentDownload->url().toString()));
    }

     m_currentDownload->deleteLater();
     startNextDownload();
}

void QueueDownloader::downloadReadyRead() {
     m_outputFile.write(m_currentDownload->readAll());
}

void QueueDownloader::pause() {
    m_isPaused = !m_isPaused;
    if (m_isPaused)
    {
        QFileInfo info(m_outputFile.fileName());
        QString outputFileWithoutPartExtension(info.absolutePath() + QDir::separator() + info.completeBaseName());
        QPair<QUrl, QString> currentDownload(m_currentDownload->url(), outputFileWithoutPartExtension);
        m_pendingDownloads.push_front(currentDownload);
        m_currentDownload->abort();
        m_outputFile.close();
        emit(paused());
    }
    if (m_isWorking && !m_isPaused)
    {
        startNextDownload();
    }
}

void QueueDownloader::cancelDownloadInProgress() {
    if (m_currentDownload)
    {
        QUrl url = m_currentDownload->url();
        m_currentDownload->abort();
        emit downloadCancelled(url.toString());
    }
}

void QueueDownloader::cancelDownload(QString urlToCancel) {
    if (m_currentDownload && m_currentDownload->url().toString() == urlToCancel)
    {
        cancelDownloadInProgress();
        return;
    }
    QPair<QUrl, QString> pair;
    foreach (pair, m_pendingDownloads)
    {
        if (pair.first.toString() == urlToCancel)
        {
            m_pendingDownloads.removeOne(pair);
            emit downloadCancelled(urlToCancel);
            return;
        }
    }
}

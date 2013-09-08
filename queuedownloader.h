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

#ifndef QUEUEDOWNLOADER_H
#define QUEUEDOWNLOADER_H
#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <QFile>
#include <QQueue>
#include <QPair>
#include <QTime>

#define TEMP_FILE_PREFIX ".part"

class QueueDownloader : public QObject
{
    Q_OBJECT
public:
    explicit QueueDownloader(QObject *parent = 0);

    void addDownload(QUrl url, QString filename) ;

    int queueSize() const;

    void pause();

    void cancelDownloadInProgress();

    void cancelDownload(QString url);

private slots:
    void startNextDownload();

    void downloadProgressed(qint64 bytesReceived, qint64 totalSize);

    void downloadFinished();

    void downloadReadyRead();

signals:
    void downloadProgressed(QString url, qint64 loadedSize, qint64 totalSize, double kbytesPerSecond, double remainingTimeInSecond);
    void downloadFinished(QString url);
    void downloadError(QString url, QString message);
    void downloadCancelled(QString url);
    void allDownloadsFinished();
    void paused();

private:

    QNetworkReply* m_currentDownload;
    QNetworkAccessManager* m_manager;
    QQueue<QPair<QUrl, QString> > m_pendingDonwloads; /// Queue of pair of remote url/local filename
    QFile m_outputFile;
    QTime m_downloadTime;
    QTime m_lastNotifTime;
    bool m_isWorking;
    bool m_isPaused;
    qint64 m_previouslyDownloaded;
};

#endif // QUEUEDOWNLOADER_H

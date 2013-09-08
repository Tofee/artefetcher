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

    void cancelDownload(QString key);

signals:
    void signalAllFilmDownloadFinished();
    void signalDownloadProgressed(QString key,double,double, double);
    void signalDownloadFinished(QString key);
    void signalDownloadCancelled(QString key);
    void signalDownloadError(QString key, QString message);
    void hasBeenPaused();

public slots:
    void pause();
    void cancelDownloadInProgress();

private slots:
    void downloadProgressed(QString url, qint64 loadedSize, qint64 totalSize, double kbytesPerSecond, double remainingTimeInSecond);

    void downloadFinished(QString url);

    void downloadError(QString url, QString message);

    void allDownloadsFinished();

    void downloadCancelled(QString url);

private:
    // Keys sent in emitted signals, indexed by their URL
    // here indexed by the URL of the movie page
    QMap<QString, QString> m_keysForSignalByUrl;
    QueueDownloader m_downloader;
};


#endif // RTMPTHREAD_H

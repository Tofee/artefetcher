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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QNetworkProxy>
#include <QSystemTrayIcon>

#include <preferences.h>
#include <filmdelegate.h>

class QNetworkReply;
class QNetworkAccessManager;
class FilmDelegate;
class QTableWidgetItem;
class DownloadManager;
class FilmDetails;
class QSystemTrayIcon;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:

    /**
     * @brief getStreamIdForColumn
     * @return the stream id matching user preferences
     */
    StreamType getStreamType() const;

    /**
     * @brief getFileName
     * @param title the title of the film
     * @param remoteFilename the file name in the remote server (will be used to check the extension)
     * @param fileSuffixNumber provide an optional file suffix (_<fileSuffixNumber> will be appened between the name and the extension of the file
     * @param episodeName the name of the episode if the film belongs to a video serie
     * @return the filename
     */
    QString getFileName(const QString &title, const QString &remoteFilename, int fileSuffixNumber = 0, QString episodeName = QString()) const;

    void downloadFilm(FilmDetails* film);


private slots:

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void refreshTable();
    void clearAndLoadTable();
    void reloadCurrentRow();
    void updateCurrentDetails();
    void updateRowInTable(const FilmDetails* const film, int rowNumber);

    /* Triggered by the downloader*/
    void allFilmDownloadFinished();
    void downloadProgressed(QString filmUrl, double progression, double kBytesPersecond, double remainingTimeForCurrentFilm);
    void filmDownloaded(QString filmUrl);
    void hasBeenPaused();
    void downloadCancelled(QString filmUrl);
    void downloadError(QString filmUrl, QString errorMsg);

    /* User actions */
    void downloadButtonClicked();
    void webPageButtonClicked();
    void cancelSelectedFilmDownload();
    void playFilm();
    void openFilmDirectory();
    void showAboutWindow();

    void nextPage();
    void previousPage();

    void addFilmManuallyFromUrl();

    void showPreferences();
    void languageChanged();
    void qualityChanged();

    /* Film delegate */
    void errorOccured(QString filmUrl, QString errorMessage);
    void streamIndexLoaded(int resultCount, int currentPage, int pageCount);

private:

    static const QList<MetaType> &listInterestingDetails();

    bool isReadyForDownload(const FilmDetails * const film) const;
    FilmDetails* getCurrentFilm() const;
    bool fileExistForTheFilm(const FilmDetails * const) const;

    void resizeEvent(QResizeEvent * event);
    void closeEvent(QCloseEvent* event);

    QTableWidgetItem *createOrUpdateTitleColumn(int rowNumber);
    void loadStreamComboBox();
    void changeDownloadPartVisibility(bool isVisible);
    void applyProxySettings();
    
private:
    Ui::MainWindow *ui;
    QNetworkAccessManager* manager;
    FilmDelegate* delegate;
    Qt::CheckState m_pressedItemCheckState;
    int m_pressedItemRow;

    QNetworkProxy m_userDefinedProxy;

    DownloadManager* thread;
    QSystemTrayIcon *m_trayIcon;
};

#endif // MAINWINDOW_H

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

class DownloadManager;
class FilmDelegate;
class FilmDetails;
class QNetworkAccessManager;
class QNetworkReply;
class QSystemTrayIcon;
class QTableWidgetItem;

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
     * @param film to get the filename
     * @param provide an optional file suffix (_<fileSuffixNumber> will be appened between the name and the extension of the file
     * @return the filename
     */
    QString getFileName(const FilmDetails * const film) const;

    void downloadFilm(FilmDetails* film);


private slots:

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void refreshTable();
    void clearAndLoadTable();
    void reloadCurrentRow();
    void filmHasBeenUpdated(const FilmDetails * const film);
    void updateCurrentDetails();
    void updateRowInTable(const FilmDetails* const film);

    void updateItemProgressBar();

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
    void streamTypeChanged();

    void nextPage();
    void previousPage();

    void showPreferences();
    void showAboutWindow();

    /* Film delegate */
    void errorOccured(QString filmUrl, QString errorMessage);
    void streamIndexLoaded(int resultCount, int currentPage, int pageCount);

    /* note : this slot is not triggered as a slot by the user click but a timer needs it as a slot */
    void clicOnPreview(bool fromTimer = true);

private:

    static const QList<MetaType> &listInterestingDetails();

    bool isReadyForDownload(const FilmDetails * const film) const;
    FilmDetails* getCurrentFilm() const;
    bool fileExistForTheFilm(const FilmDetails * const) const;
    void updateFilmStreamCombobox(FilmDetails * const);
    void updateButtonsVisibility(const FilmDetails * const);
    void updateBigImageVisibility();

    void resizeEvent(QResizeEvent * event);
    void closeEvent(QCloseEvent* event);

    QTableWidgetItem *createOrUpdateTitleColumn(int rowNumber);
    void loadStreamComboBox();
    void changeDownloadPartVisibility(bool isVisible);
    void applyProxySettings();
    bool eventFilter(QObject *obj, QEvent *event);


    
private:
    Ui::MainWindow *ui;
    QNetworkAccessManager* manager;
    FilmDelegate* delegate;
    Qt::CheckState m_pressedItemCheckState;
    int m_pressedItemRow;
    int m_currentPreview;

    QNetworkProxy m_userDefinedProxy;

    DownloadManager* thread;
    QSystemTrayIcon *m_trayIcon;
    QTimer* m_imageTimer;
};

#endif // MAINWINDOW_H

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

#include <preferences.h>
#include <filmdelegate.h>
#include <QSystemTrayIcon>

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
    //void lockWidgets(bool lock);

    /**
     * @brief getStreamIdForColumn
     * @return the stream id matching user preferences
     */
    StreamType getStreamType() const;

    /**
     * @brief getFileName
     * @param targetDirectory
     * @param title the title of the film
     * @param streamTypeId
     * @return the filename
     */
    QString getFileName(const QString& targetDirectory, const QString &title, const QString &remoteFilename, int fileSuffixNumber = 0);

    void downloadFilm(int currentLine, FilmDetails* film);


private slots:
    void refreshTable();
    void updateCurrentDetails();

    void allFilmDownloadFinished();
    void downloadProgressed(QString filmUrl, double progression, double speed, double remainingTime);
    void filmDownloaded(QString filmUrl);

    void reloadCurrentRow();
    void addFilmManuallyFromUrl();
    void errorOccured(QString filmUrl, QString errorMessage);
    void showPreferences();
    void cellHasBeenClicked(int row, int column);

    void languageChanged();
    void qualityChanged();
    void clearAndLoadTable();

    void downloadButtonClicked();

    void nextPage();
    void previousPage();
    void streamIndexLoaded(int resultCount, int currentPage, int pageCount);

    const QList<MetaType> &listInterestingDetails();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void closeEvent(QCloseEvent* event);

    void createOrUpdateFirstColumn(int rowNumber);
    bool isReadyForDownload(const FilmDetails * const film);
    void loadStreamComboBox();
    void resizeEvent( QResizeEvent * event );
    
private:
    Ui::MainWindow *ui;
    QNetworkAccessManager* manager;
    FilmDelegate* delegate;
    Preferences preferences;
    Qt::CheckState m_pressedItemCheckState;
    int m_pressedItemRow;


    DownloadManager* thread;
    QSystemTrayIcon *m_trayIcon;
};

#endif // MAINWINDOW_H

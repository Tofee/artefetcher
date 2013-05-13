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
    QString getFileName(const QString& targetDirectory, const QString &title, const QString &remoteFilename);

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

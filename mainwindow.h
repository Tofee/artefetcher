#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QNetworkCookie>
#include <QModelIndex>
#include <FilmDetails.h>
#include <QStringList>

#include <preferences.h>
#include <filmdelegate.h>

class QNetworkReply;
class QNetworkAccessManager;
class FilmDelegate;

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
    void updateCookieProfiles();


    void lockWidgets(bool lock);

    /**
     * @brief getStreamIdForColumn
     * @param columnId the column number in ui->tableWidget
     * @return the stream id associated to the column. -1 if none
     */
    StreamType getStreamTypeOfColumn(int columnId) const throw (NotFoundException);

    /**
     * @brief getFileName
     * @param targetDirectory
     * @param title the title of the film
     * @param streamTypeId
     * @return the filename
     */
    QString getFileName(const QString& targetDirectory, const QString &title, StreamType streamType);
private slots:
    void loadCookieProfile();
    void refreshTable();
    void loadPlayList();
    void updateCurrentDetails();
    void downloadAll();
    void allFilmDownloadFinished();
    void downloadProgressed(int filmId, StreamType streamType, double progression, int speed);
    void filmDownloaded(int filmId, StreamType streamType);
    void changeColumnChecking(int column);
    void reloadCurrentRow();
    void addFilmManuallyFromUrl();
    void errorOccured(int filmId, QString errorMessage);
    void showPreferences();
    void cellHasBeenClicked(int row, int column);
    void removeCurrentFilm();
private:
    void closeEvent(QCloseEvent* event);
    
private:
    Ui::MainWindow *ui;
    QNetworkAccessManager* manager;
    FilmDelegate* delegate;
    QStringList cookieProfiles;
    Preferences preferences;
};

#endif // MAINWINDOW_H

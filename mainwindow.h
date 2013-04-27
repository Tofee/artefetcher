#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
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
    QString getFileName(const QString& targetDirectory, const QString &title, StreamType streamType);
private slots:
    void refreshTable();
    void updateCurrentDetails();
    void downloadAll();
    void allFilmDownloadFinished();
    void downloadProgressed(int filmId, StreamType streamType, double progression, double speed);
    void filmDownloaded(int filmId, StreamType streamType);
    void changeColumnChecking(int column);
    void reloadCurrentRow();
    void addFilmManuallyFromUrl();
    void errorOccured(int filmId, QString errorMessage);
    void showPreferences();
    void cellHasBeenClicked(int row, int column);
    void removeCurrentFilm();
    void languageChanged();
    void qualityChanged();

private:
    void closeEvent(QCloseEvent* event);
    
private:
    Ui::MainWindow *ui;
    QNetworkAccessManager* manager;
    FilmDelegate* delegate;
    Preferences preferences;
};

#endif // MAINWINDOW_H

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
class QTableWidgetItem;
class RTMPThread;

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
    QString getFileName(const QString& targetDirectory, const QString &title);
private slots:
    void refreshTable();
    void updateCurrentDetails();
    void downloadFilm(int currentLine, FilmDetails* film);
    void allFilmDownloadFinished();
    void downloadProgressed(int filmId, double progression, double speed, double remainingTime);
    void filmDownloaded(int filmId);

    void reloadCurrentRow();
    void addFilmManuallyFromUrl();
    void errorOccured(int filmId, QString errorMessage);
    void showPreferences();
    void cellHasBeenClicked(int row, int column);

    void languageChanged();
    void qualityChanged();
    void clearAndLoadTable();

    void downloadButtonClicked();

    void nextPage();
    void previousPage();
    void streamIndexLoaded(int resultCount, int currentPage, int pageCount);

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

    // 1) Filter the selected films
    QMap<int, FilmDetails> checkedFilms;
    RTMPThread* thread;
};

#endif // MAINWINDOW_H

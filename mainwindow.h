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
    QString getFileName(const QString& targetDirectory, const QString &title, StreamType streamType);
private slots:
    void refreshTable();
    void updateCurrentDetails();
    void downloadFilm(int currentLine, FilmDetails* film);
    void allFilmDownloadFinished();
    void downloadProgressed(int filmId, StreamType streamType, double progression, double speed);
    void filmDownloaded(int filmId, StreamType streamType);
    void changeColumnChecking(int column);
    void reloadCurrentRow();
    void addFilmManuallyFromUrl();
    void errorOccured(int filmId, QString errorMessage);
    void showPreferences();
    void cellHasBeenClicked(int row, int column);

    void languageChanged();
    void qualityChanged();

    void tableItemPressed(QTableWidgetItem * item);
    void tableItemClicked(QTableWidgetItem * item);
    void checkStateChanged(QTableWidgetItem * item);
    //Qt::CheckState m_pressedItemCheckState;
    //int m_pressedItemRow;

private:
    void closeEvent(QCloseEvent* event);

    void createOrUpdateFirstColumn(int rowNumber);
    
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

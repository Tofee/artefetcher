#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QtGui>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QNetworkCookie>
#include <QSqlError>
#include <QIcon>

#include <preferencedialog.h>
#include <filmdelegate.h>
#include <FilmDetails.h>
#include <rtmpthread.h>
#include <QList>

#define FIREFOX_SQLITE_FILENAME "/cookies.sqlite"
#define FIREFOX_PROFILES_PATH "/.mozilla/firefox/"
#define COLUMN_FOR_PAGE 1
#define COLUMN_FOR_TITLE 0
#define FIRST_CHECKBOX_COLUMN_IN_TABLE 4

// source of images /usr/share/icons/oxygen/16x16/status/
// TODO on peut surement mieux faire que comme ça
class MyCJar : public QNetworkCookieJar
{
public:
    MyCJar(QObject* parent):QNetworkCookieJar(parent) {}

    void setAllCookies(const QList<QNetworkCookie> &cookieList) {
        QNetworkCookieJar::setAllCookies(cookieList);
    }

    QList<QNetworkCookie> allCookies() const {
        return QNetworkCookieJar::allCookies();
    }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    manager(new QNetworkAccessManager(this))
{
    preferences.load();

    ui->setupUi(this);
    updateCookieProfiles();

    loadCookieProfile();

    delegate = new FilmDelegate(manager);

    QStringList header;
    header << tr("Title") << tr("Rating")
           << tr("Views") << tr("Duration");

    for (QList<StreamType>::iterator streamIterator = FilmDelegate::listStreamTypes().begin();
         streamIterator != FilmDelegate::listStreamTypes().end();
         ++streamIterator)
    {
        StreamType& type = *streamIterator;
        header << type.humanCode;
    }

    ui->tableWidget->setColumnCount(header.size());
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->progressBar->setMaximum(100);

    connect(ui->tableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)),
            SLOT(changeColumnChecking(int)));

    connect(delegate, SIGNAL(playListHasBeenUpdated()),
            SLOT(refreshTable()));
    connect(delegate, SIGNAL(errorOccured(int,QString)),
            SLOT(errorOccured(int,QString)));

    connect(ui->loadPlaylistButton, SIGNAL(clicked()),
            SLOT(loadPlayList()));

    connect(ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)),
            SLOT(updateCurrentDetails()));
    connect(ui->downloadVideosButton, SIGNAL(clicked()),
            SLOT(downloadAll()));

    connect(ui->reloadFilmButton, SIGNAL(clicked()),
            SLOT(reloadCurrentRow()));

    connect(ui->manualAddButton, SIGNAL(clicked()),
            SLOT(addFilmManuallyFromUrl()));

    connect(ui->settingsButton, SIGNAL(clicked()),
            SLOT(showPreferences()));

    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)),
            SLOT(cellHasBeenClicked(int, int)));

    connect(ui->allVideosButton, SIGNAL(clicked()),
            delegate, SLOT(loadAllCatalog()));

    // TODO the URL used to remove movie is correct, but works only
    // in a browser, even if the HTTP answer is the same here and in the browser. Weird!
    ui->removeButton->setVisible(false);
    connect(ui->removeButton, SIGNAL(clicked()),
            this, SLOT(removeCurrentFilm()));


    lockWidgets(false);

    loadPlayList();
}



StreamType MainWindow::getStreamTypeOfColumn(int columnId) const throw (NotFoundException)
// TODO catch partout
{
    return FilmDelegate::getStreamTypeByHumanName(ui->tableWidget->horizontalHeaderItem(columnId)->text());
}
void MainWindow::changeColumnChecking(int column)
{
    try {
        getStreamTypeOfColumn(column); // Just to check we are in a checkable column
        QList<QTableWidgetItem*> checked;
        QList<QTableWidgetItem*> unchecked;

        QTableWidgetItem* item;
        for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
        {
            item = ui->tableWidget->item(row, column);
            if (item != NULL && (item->flags()& Qt::ItemIsUserCheckable))
                (item->checkState() == Qt::Checked ? checked : unchecked) << item;
        }

        if (unchecked.size() != 0)
            foreach (item, unchecked)
                item->setCheckState(Qt::Checked);
        else
            foreach (item, checked)
                item->setCheckState(Qt::Unchecked);
    } catch (NotFoundException)
    {
        // This is not a language column. Nothing to do
        return;
    }


}

MainWindow::~MainWindow()
{
    delete ui;
    delete delegate;
}

void MainWindow::updateCookieProfiles()
{
    cookieProfiles.clear();
    QDir d(QDir::homePath().append(FIREFOX_PROFILES_PATH));
    if (! d.exists())
        return;// TODO
    cookieProfiles << d.entryList(QDir::AllDirs|QDir::NoDotAndDotDot);
}

void MainWindow::loadCookieProfile()
{

    QString firefoxProfile(preferences.firefoxProfile());
    if (firefoxProfile.isEmpty())
    {
        firefoxProfile = cookieProfiles.first();
    }
    qDebug() << "Load cookie begin " <<firefoxProfile;

    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
    {
        QSqlDatabase::removeDatabase(firefoxProfile);
    }
    QSqlDatabase base = QSqlDatabase::addDatabase("QSQLITE"); // TODO on a droit qu'à un seul
    base.setConnectOptions("QSQLITE_OPEN_READONLY");
    QString baseName(QDir::homePath());
    baseName.append(FIREFOX_PROFILES_PATH);
    baseName.append(firefoxProfile);
    baseName.append(FIREFOX_SQLITE_FILENAME);
    base.setDatabaseName(baseName);

    QList<QNetworkCookie> result;

    if (!base.open())
    {
        //TODO
        return;
    }

    QString queryText("select host");
    queryText.append(", case substr(host,1,1)='.' when 0 then 'FALSE' else 'TRUE' end");
    queryText.append(", path");
    queryText.append(", case isSecure when 0 then 'FALSE' else 'TRUE' end");
    queryText.append(", expiry");
    queryText.append(", name");
    queryText.append(", value");
    queryText.append(" from moz_cookies");
    queryText.append(" where host like '%arte.tv%';");

    QSqlQuery query;
    if (!query.exec(queryText))
    {
        //TODO
        qDebug() << query.lastError().databaseText();
        qDebug() << query.lastError().driverText();
    }

    while (query.next())
    {
//        QString host(query.value(0).toString());
//        bool unknownBool(query.value(1).toBool());
//        QString path(query.value(2).toString());
//        bool isSecure(query.value(3).toBool());
//        QDateTime expiry(query.value(4).toDateTime());
//        QString name(query.value(5).toString());
//        QString value(query.value(6).toString());

        QString name(query.value(5).toString());
        QString value(query.value(6).toString());
        QNetworkCookie cookie(name.toAscii(), value.toAscii());
        cookie.setDomain(query.value(0).toString());
        result.append(cookie);
    }

    base.close();

    if (manager->cookieJar() != NULL)
    {
        manager->cookieJar()->deleteLater();
    }

    // Override cookies
    MyCJar* cookieJar = new MyCJar(manager);
    cookieJar->setAllCookies(result);
    manager->setCookieJar(cookieJar);

    qDebug() << "Load cookie end";
}

void MainWindow::loadPlayList()
{
    qDebug() << "Load playlist begin";
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    delegate->loadPlayList();
    qDebug() << "Load playlist end";
}

bool isReadyForDownload(const FilmDetails * const film, StreamType streamType)
{
    return !film->m_title.isEmpty()
           // && !film->m_flashPlayerUrl.isEmpty()
            && film->m_streamsByType.contains(streamType);
}

void MainWindow::refreshTable()
{

    QMap<QString, FilmDetails*> details = delegate->films();
    FilmDetails* film;
    int rowNumber = 0;
    foreach (film, details)
    {
        QTableWidgetItem* titleTableItem = ui->tableWidget->item(rowNumber, 0);
        if (titleTableItem == NULL)
        {
            titleTableItem = new QTableWidgetItem();
            ui->tableWidget->setItem(rowNumber, 0, titleTableItem);
        }
        else
        {
            titleTableItem->setText(film->m_title);
        }

        if (film->m_rating)
        {
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(film->m_rating));
            item->setFlags(item->flags()^Qt::ItemIsEditable);
            ui->tableWidget->setItem(rowNumber, 1, item);
        }
        if (film->m_numberOfViews)
        {
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(film->m_numberOfViews));
            item->setFlags(item->flags()^Qt::ItemIsEditable);
            ui->tableWidget->setItem(rowNumber, 2, item);
        }
        if (film->m_durationInMinutes > 0)
        {
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(film->m_durationInMinutes));
            item->setFlags(item->flags()^Qt::ItemIsEditable);
            ui->tableWidget->setItem(rowNumber, 3, item);
        }

        for (int column = FIRST_CHECKBOX_COLUMN_IN_TABLE; column < ui->tableWidget->columnCount(); ++ column)
        {
            try {
                StreamType type =
                        FilmDelegate::getStreamTypeByHumanName(ui->tableWidget->horizontalHeaderItem(column)->text());

                if (isReadyForDownload(film, type)
                        && ui->tableWidget->item(rowNumber, column) == NULL)
                {
                    QTableWidgetItem* checkableWidget = new QTableWidgetItem();
                    checkableWidget->setFlags((Qt::ItemIsUserCheckable|checkableWidget->flags())^Qt::ItemIsEditable);
                    checkableWidget->setCheckState(Qt::Unchecked);
                    ui->tableWidget->setItem(rowNumber, column, checkableWidget);
                }
            } catch (NotFoundException)
            {
                // Nothing to do
            }
        }
        ++rowNumber;
    }

    int previousCount = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(rowNumber);

    if (previousCount <= 0 && rowNumber>0 )
    {
        ui->tableWidget->setCurrentCell(0,0);
    }
    ui->tableWidget->resizeColumnsToContents();
    updateCurrentDetails();
}

void MainWindow::updateCurrentDetails(){
    int rowBegin = ui->tableWidget->currentRow();
     QMap<QString, FilmDetails*> details = delegate->films();
     if (rowBegin >=0 && rowBegin < ui->tableWidget->rowCount())
     {
        FilmDetails*  film = details.values().at(rowBegin);
        ui->summaryLabel->setText(film->m_summary);
        if (!film->m_preview.isNull())
        {
            ui->previewLabel->setPixmap(QPixmap::fromImage(film->m_preview));
            ui->previewLabel->setVisible(true);
        }
        else
        {
            ui->previewLabel->setVisible(false);
        }
        ui->detailsGroupBox->setTitle(film->m_title);
        if (!film->m_countries.isEmpty())
        {
            ui->countryYearDurationlabel->setText(tr("%1, %2, %3min")
                                                  .arg(film->m_countries.join(", "),
                                                       QString::number(film->m_year),
                                                       QString::number(film->m_durationInMinutes)));
        }
        else
        {
            ui->countryYearDurationlabel->setText("");
        }
     }
     else
     {
         ui->summaryLabel->setText(tr("No film in the playlist"));
         ui->previewLabel->setPixmap(QPixmap());
         ui->previewLabel->setVisible(false);
         ui->detailsGroupBox->setTitle("");
         ui->countryYearDurationlabel->setText("");
     }
}

int downloadStream(QString rtmpUrl, QString flashUrl, QProgressBar* bar);

QString MainWindow::getFileName(const QString& targetDirectory, const QString& title, StreamType streamType)
{//TODO la vitesse de chargement ne s'affiche plus
    // TODO les caractères HTML sont à convertir (ex:&#39; => ')
    QString cleanedTitle(title);
    cleanedTitle.replace(QRegExp("[éèëê]"), "e");
    cleanedTitle.replace(QRegExp("[ô]"), "o");
    cleanedTitle.replace(QRegExp("[à]"), "à");
    cleanedTitle.replace(QRegExp("[îï]"), "i");
    cleanedTitle.replace(QRegExp("[û]"), "u");
    cleanedTitle.replace(QRegExp("[ç]"), "c");
    cleanedTitle.replace(QRegExp("[ß]"), "ss");
    cleanedTitle.replace(QRegExp("[ä]"), "ae");
    cleanedTitle.replace(QRegExp("[ö]"), "oe");
    cleanedTitle.replace(QRegExp("[ü]"), "ue");
    cleanedTitle.replace(QRegExp("[/]"), "-");
    cleanedTitle.replace(QRegExp("[^a-zA-Z0-9 _-()]"), " ");
    cleanedTitle = cleanedTitle.simplified();

    QString language(streamType.languageCode);
    language = language.replace(0, 1, language.left(1).toUpper());

    QString baseName(preferences.filenamePattern());
    baseName.replace("%title", cleanedTitle)
            .replace("%language", language)
            .replace("%quality", streamType.qualityCode.toUpper());

    QString filename("%1%2%3");
    filename = filename.arg(targetDirectory,
                            QDir::separator(),
                            baseName);
    return filename;
}


void MainWindow::downloadAll()
{
    QString workingPath(QDir::homePath().append(QDir::separator()).append("arteFetcher"));

    // 1) Filter the selected films
    QMap<int, FilmDetails> checkedFilms;
    QStringList usedNames;

    FilmDetails* film;
    int currentLine =0;
    foreach (film, delegate->films())
    {
        bool oneStreamTypeAdded = false;
        for (int column = FIRST_CHECKBOX_COLUMN_IN_TABLE; column< ui->tableWidget->columnCount(); ++column)
        {
            try
            {
                StreamType streamType = getStreamTypeOfColumn(column);
                if (isReadyForDownload(film, streamType)
                        && ui->tableWidget->item(currentLine, column) != NULL
                        && ui->tableWidget->item(currentLine, column)->checkState()
                            == Qt::Checked)
                {
                    QString titleCellText = ui->tableWidget->item(currentLine, 0)->text();
                    QString futureFileName = getFileName(workingPath, titleCellText, streamType);
                    if (QFile(futureFileName).exists()
                            && QMessageBox::question(this, "File already exists",
                                              tr("A file has already the name of the film: <%1>.\nDo you want to download it anyway?")
                                                     .arg(titleCellText),
                                              QMessageBox::Yes,
                                              QMessageBox::No)
                                == QMessageBox::No)
                    {
                        ui->tableWidget->item(currentLine, column)->setCheckState(Qt::Unchecked);
                        film->m_streamsByType[streamType].use = false;
                    }
                    else if (usedNames.contains(futureFileName)
                             && QMessageBox::warning(this, "Many films with same name",
                                                     tr("Two or more films will have the same name. Please correct the name on your own, editing the title column of the films. Then try again.\nCheck: <%1>")
                                                     .arg(titleCellText)))
                    {
                        return;
                    }
                    else
                    {

                        qDebug() << film->m_streamsByType[streamType].m_rtmpStreamUrl << film->m_flashPlayerUrl;
                        usedNames << futureFileName;
                        oneStreamTypeAdded = true;
                        film->m_streamsByType[streamType].use = true;
                        film->m_streamsByType[streamType].m_targetFileName = futureFileName;
                    }
                }
            }
            catch (NotFoundException e)
            {
                // This should never occur
            }
        }
        if (oneStreamTypeAdded)
        {
            checkedFilms.insert(currentLine, *film);
        }
        ++currentLine;
    }

    // 2) At least one film ?
    if (checkedFilms.isEmpty())
    {
        statusBar()->showMessage(tr("No film is selected or some films are not ready to download. Please wait while loading information and try again later"));
        return;
    }

    // 3) Check the destination directory
    QDir workingDir(workingPath);
    if (!workingDir.exists() && ! workingDir.mkdir(workingPath))
    {
        statusBar()->showMessage(tr("Cannot create the working directory %1").arg(workingPath));
        return;
    }

    // 4) Lock and start download
    lockWidgets(true);

    RTMPThread* thread = new RTMPThread(checkedFilms, this);
    //thread->start();
    connect(thread, SIGNAL(allFilmDownloadFinished()),
            this, SLOT(allFilmDownloadFinished()));
    connect(thread, SIGNAL(downloadProgressed(int,StreamType,double,double)),
            this, SLOT(downloadProgressed(int,StreamType,double,double)));
    connect(thread, SIGNAL(downloadFinished(int,StreamType)),
            this, SLOT(filmDownloaded(int,StreamType)));
}


void MainWindow::allFilmDownloadFinished()
{
    lockWidgets(false);
    statusBar()->showMessage(tr("Download finished."));
}

void MainWindow::lockWidgets(bool lock)
{
    ui->progressBar->setVisible(lock);
    ui->settingsButton->setEnabled(!lock);
    ui->downloadVideosButton->setEnabled(!lock);
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        for (int col = FIRST_CHECKBOX_COLUMN_IN_TABLE; col < ui->tableWidget->columnCount(); ++col)
        {
            QTableWidgetItem* item = ui->tableWidget->item(row, col);
            if (item == NULL)
                continue;
            if (lock)
                item->setFlags(item->flags()^Qt::ItemIsEnabled);
            else
                item->setFlags(item->flags()|Qt::ItemIsEnabled);
        }
    }
    ui->manualAddButton->setEnabled(!lock);
    ui->reloadFilmButton->setEnabled(!lock);
    ui->loadPlaylistButton->setEnabled(!lock);

}

void MainWindow::downloadProgressed(int filmId, StreamType streamType, double progression, double speed)
{
    //if (progression == 0)
    {
        for (int column = FIRST_CHECKBOX_COLUMN_IN_TABLE; column < ui->tableWidget->columnCount(); ++column)
        {
            if (ui->tableWidget->horizontalHeaderItem(column)->text()== streamType.humanCode)
            {
                QTableWidgetItem* currentItem = new QTableWidgetItem();
                currentItem->setIcon(QIcon(":/img/progress.png"));
                currentItem->setFlags(currentItem->flags()^Qt::ItemIsUserCheckable);
                ui->tableWidget->setItem(filmId, column, currentItem);
            }
        }
    }
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setCurrentCell(filmId, 0);
    ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->progressBar->setValue(progression);
    statusBar()->showMessage(
                (speed > 0 ? tr("Download speed: %1 KiB/s (%2)")
                             .arg(speed)
                             .arg(streamType.humanCode)
                          : tr("Download speed: %1 KiB/s (%2) ...")
                             .arg(speed)
                             .arg(streamType.humanCode)));
}

void MainWindow::filmDownloaded(int filmId, StreamType streamType)
{
    for (int column = FIRST_CHECKBOX_COLUMN_IN_TABLE; column < ui->tableWidget->columnCount(); ++column)
    {
        if (ui->tableWidget->horizontalHeaderItem(column)->text()== streamType.humanCode)
        {
            QTableWidgetItem* item = ui->tableWidget->item(filmId, column);
            delete item;
            item = new QTableWidgetItem();
            item->setIcon(QIcon(":/img/finished.png"));
            ui->tableWidget->setItem(filmId, column, item);
            item->setFlags(item->flags()^Qt::ItemIsUserCheckable);

            FilmDetails * d = delegate->films().values().at(filmId);
            d->m_streamsByType[streamType].downloaded = true;

            // Save metadata
            QFile metadataFile(QString(d->m_streamsByType[streamType].m_targetFileName).append(".info"));
            metadataFile.open(QFile::WriteOnly|QFile::Text);
            QTextStream stream (&metadataFile);
            stream<< d->m_title << "\n";
            stream<< d->m_summary;
            stream.flush();
            metadataFile.close();
            QFile picture(QString(d->m_streamsByType[streamType].m_targetFileName).append(".png"));
            picture.open(QFile::WriteOnly);
            QImageWriter writer(&picture, "PNG");
            writer.write(d->m_preview);
            picture.close();
            break;

        }
    }
}

void MainWindow::reloadCurrentRow()
{
    if (ui->tableWidget->rowCount() <=0)
        return;
    int currentRow = ui->tableWidget->currentRow();
    ui->tableWidget->item(currentRow, 0)->setToolTip("");
    ui->tableWidget->item(currentRow, 0)->setIcon(QIcon());
    delegate->reloadFilm(delegate->films().values()[currentRow]);
}


void MainWindow::addFilmManuallyFromUrl()
{
    // TODO add drag&drop
    QString url = QInputDialog::getText(this, tr("Add a new Film from URL"),tr("Enter the URL of your arte film page"));
    delegate->addMovieFromUrl(url);
}


void MainWindow::errorOccured(int filmId, QString errorMessage)
{
    if (ui->tableWidget->rowCount() <= filmId)
        return;
    QTableWidgetItem *cell = ui->tableWidget->item(filmId, 0);
    if (cell == NULL)
    {
        qDebug() << errorMessage;
        return;
    }
    if(cell->icon().isNull())
    {
        cell->setIcon(QIcon(":/img/warning.png"));
        cell->setToolTip(errorMessage);
    }
    else
    {
        if (! cell->toolTip().contains(errorMessage))
            cell->setToolTip(cell->toolTip().append("\n").append(errorMessage));
    }
}

void MainWindow::showPreferences()
{
    QString oldCookieProfile = preferences.firefoxProfile();

    PreferenceDialog dial(this, preferences, cookieProfiles);
    if (dial.exec())
    {
        if (oldCookieProfile != preferences.firefoxProfile())
        {
            loadCookieProfile();
            loadPlayList();
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (ui->progressBar->isVisible())
    {
        int choice = QMessageBox::warning(this,
                                          tr("Close the application?"),
                                          tr("Download is still in progress. Closing the application will stop them immediately and your films file may be incomplete, corrupted or not event downloaded."),
                                          tr("Quit anyway"),
                                          tr("Cancel"));
        if (choice != 0)
        {
            event->ignore();
            return;
        }
    }
    event->accept();
}

void MainWindow::cellHasBeenClicked(int row, int column)
{
    if (ui->progressBar->isVisible())
    {
        return;
    }
    try{
        StreamType streamType = getStreamTypeOfColumn(column);
        FilmDetails * film = delegate->films().values().value(row);
        if (film != NULL)
        {
            QString fileName = getFileName(preferences.destinationDir(), ui->tableWidget->item(row, 0)->text(), streamType);
            if (film->m_streamsByType.contains(streamType)
                    && film->m_streamsByType[streamType].downloaded)
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
        }

    } catch (NotFoundException e)
    {
        return;
    }

}

void MainWindow::removeCurrentFilm()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0 || row >= ui->tableWidget->rowCount())
        return;
    FilmDetails * film = delegate->films().values().value(row);
    if (film == NULL)
        return;
    delegate->removeFilm(film);
}

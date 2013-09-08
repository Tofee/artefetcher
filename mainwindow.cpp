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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QtGui>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QIcon>
#include <QList>
#include <preferencedialog.h>
#include <filmdelegate.h>
#include <FilmDetails.h>
#include <downloadManager.h>

#define COLUMN_FOR_TITLE 1
#define COLUMN_FOR_DURATION 2
#define COLUMN_FOR_PREVIEW 0

#define TABLE_PREVIEW_MAX_WIDTH 100
#define TABLE_PREVIEW_MAX_HEIGHT 100
#define TABLE_COLUMN_MARGIN 10

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    manager(new QNetworkAccessManager(this)),
        thread (new DownloadManager(this)),
        m_trayIcon(new QSystemTrayIcon(QIcon(":/img/arte-tv.png"), this))
{
    preferences.load();

    ui->setupUi(this);
    this->setWindowTitle("ArteFetcher v0.2.3");
    this->resize(preferences.preferredWindowSize());
    m_trayIcon->show();

    delegate = new FilmDelegate(manager, preferences);

    QStringList header;
    header
           << "" // Preview
           << tr("Title")
           << tr("Duration")
    ;

    ui->tableWidget->setColumnCount(header.size());
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);


    {
        QFontMetrics metric(ui->tableWidget->font());
        int totalColumnWidths(QApplication::style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 8 /* Initial size, at least the vscroll bar width*/);
        int nextColumn = (metric.boundingRect("00:00:00").width() + TABLE_COLUMN_MARGIN);
        totalColumnWidths+=nextColumn;
        ui->tableWidget->horizontalHeader()->resizeSection(COLUMN_FOR_DURATION, nextColumn);

        totalColumnWidths += (nextColumn = TABLE_PREVIEW_MAX_WIDTH + TABLE_COLUMN_MARGIN);
        ui->tableWidget->horizontalHeader()->resizeSection(COLUMN_FOR_PREVIEW,  nextColumn);

        totalColumnWidths += (nextColumn = metric.boundingRect("Sample of fitting title").width());
        ui->tableWidget->horizontalHeader()->resizeSection(COLUMN_FOR_TITLE,    nextColumn);

        ui->tableWidget->setIconSize(QSize(TABLE_PREVIEW_MAX_WIDTH, TABLE_PREVIEW_MAX_HEIGHT));
        ui->tableWidget->setMinimumSize(totalColumnWidths, 0);
    }

    ui->progressBar->setMaximum(100);

    ui->languageComboBox->addItems(FilmDelegate::listLanguages());
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findText(preferences.selectedLanguage()));
    ui->qualityComboBox->addItems(FilmDelegate::listQualities());
    ui->qualityComboBox->setCurrentIndex(ui->qualityComboBox->findText(preferences.selectedQuality()));

    ui->previewLabel->setMaximumSize(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT);
    ui->previewLabel->setMinimumSize(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT);
    ui->previewLabel->setPixmap(QPixmap(":/img/Arte.jpg").scaled(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT, Qt::KeepAspectRatio));

    ui->detailsGroupBox->setStyleSheet("QGroupBox { font-size: 16px; font-weight: bold; }");

    changeDownloadPartVisibility(false);

    ui->dateEdit->setVisible(false);
    ui->dateEdit->setDate(QDate::currentDate());

    loadStreamComboBox();
    if (!preferences.pendingDownloads().isEmpty())
    {
        ui->streamComboBox->setCurrentIndex(ui->streamComboBox->findText(tr("Downloads")));
    }

    connect(delegate, SIGNAL(playListHasBeenUpdated()),
            SLOT(refreshTable()));
    connect(delegate, SIGNAL(errorOccured(QString,QString)),
            SLOT(errorOccured(QString,QString)));

    connect(ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)),
            SLOT(updateCurrentDetails()));

    connect(ui->reloadFilmButton, SIGNAL(clicked()),
            SLOT(reloadCurrentRow()));

    connect(ui->manualAddButton, SIGNAL(clicked()),
            SLOT(addFilmManuallyFromUrl()));

    connect(ui->settingsButton, SIGNAL(clicked()),
            SLOT(showPreferences()));

    connect(ui->tableWidget, SIGNAL(cellClicked(int,int)),
            SLOT(cellHasBeenClicked(int, int)));

    connect(ui->streamComboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(clearAndLoadTable()));

    connect(ui->dateEdit, SIGNAL(dateChanged(QDate)),
            SLOT(clearAndLoadTable()));

    connect(ui->languageComboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(languageChanged()));
    connect(ui->qualityComboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(qualityChanged()));

    connect(thread, SIGNAL(signalAllFilmDownloadFinished()),
            SLOT(allFilmDownloadFinished()));
    connect(thread, SIGNAL(signalDownloadProgressed(QString,double,double, double)),
            SLOT(downloadProgressed(QString,double,double, double)));
    connect(thread, SIGNAL(signalDownloadFinished(QString)),
            SLOT(filmDownloaded(QString)));
    connect(thread, SIGNAL(signalDownloadCancelled(QString)),
            SLOT(downloadCancelled(QString)));
    connect(thread, SIGNAL(hasBeenPaused()), SLOT(hasBeenPaused()));
    connect(ui->cancelProgressingDownloadButton, SIGNAL(clicked()),
            thread, SLOT(cancelDownloadInProgress()));
    connect(thread, SIGNAL(signalDownloadError(QString,QString)),
            SLOT(downloadError(QString,QString)));

    connect(ui->downloadButton, SIGNAL(clicked()), SLOT(downloadButtonClicked()));
    connect(ui->cancelSelectedFilmButton, SIGNAL(clicked()),
            SLOT(cancelSelectedFilmDownload()));

    connect(delegate, SIGNAL(streamIndexLoaded(int,int,int)),
            SLOT(streamIndexLoaded(int,int,int)));

    connect(ui->rightPageButton, SIGNAL(clicked()),
            SLOT(nextPage()));

    connect(ui->leftPageButton, SIGNAL(clicked()),
            SLOT(previousPage()));
    connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                 this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    connect(ui->webPageButton, SIGNAL(clicked()), SLOT(webPageButtonClicked()));

    connect(ui->pauseButton, SIGNAL(clicked()), thread, SLOT(pause()));



    clearAndLoadTable();

}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        this->setVisible(!this->isVisible());
        break;
    case QSystemTrayIcon::MiddleClick:
        //showMessage();
        break;
    default:
        ;
    }
}

void MainWindow::loadStreamComboBox() {
    int previousIndex = ui->streamComboBox->currentIndex();
    ui->streamComboBox->clear();
    ui->streamComboBox->addItem(tr("Arte selection"), "http://www.arte.tv/guide/"+ preferences.selectedLanguage() + "/plus7/selection.json");
    ui->streamComboBox->addItem(tr("Most recent"), "http://www.arte.tv/guide/"+ preferences.selectedLanguage() + "/plus7/plus_recentes.json");
    ui->streamComboBox->addItem(tr("Most seen"), "http://www.arte.tv/guide/"+ preferences.selectedLanguage() + "/plus7/plus_vues.json");
    ui->streamComboBox->addItem(tr("Last chance"), "http://www.arte.tv/guide/"+ preferences.selectedLanguage() + "/plus7/derniere_chance.json");
    ui->streamComboBox->addItem(tr("All"), "http://www.arte.tv/guide/"+ preferences.selectedLanguage() + "/plus7.json");
    //ui->streamComboBox->addItem(tr("Test"), "http://www.arte.tv/papi/tvguide/epg/live/F/L3/1.json");
    ui->streamComboBox->addItem(tr("By date"), DATE_STREAM_PREFIX);
    ui->streamComboBox->addItem(tr("Downloads"), DOWNLOAD_STREAM);
    if (previousIndex >=0 )
        ui->streamComboBox->setCurrentIndex(previousIndex);
}

void MainWindow::streamIndexLoaded(int resultCount, int currentPage, int pageCount){
    ui->pageLabel->setText(tr("Page %1/%2\nTotal results: %3").arg(currentPage).arg(pageCount).arg(resultCount));
    ui->leftPageButton->setEnabled(currentPage > 1);
    ui->rightPageButton->setEnabled(currentPage < pageCount);
    ui->tableWidget->clearContents();
    ui->tableWidget->setCurrentCell(-1, 0, QItemSelectionModel::ClearAndSelect);
}

StreamType MainWindow::getStreamType() const
{
    return FilmDelegate::getStreamTypeByLanguageAndQuality(preferences.m_selectedLanguage, preferences.m_selectedQuality);
}


MainWindow::~MainWindow()
{
    preferences.setPreferredWindowSize(size());
    delete delegate;
    preferences.save();
    delete ui;
}

bool MainWindow::isReadyForDownload(const FilmDetails * const film)
{
    return film && !film->m_title.isEmpty()
            && !film->m_streamUrl.isEmpty() &&
            (film->m_downloadStatus == NONE || film->m_downloadStatus == CANCELLED || film->m_downloadStatus == ERROR);
}

void MainWindow::languageChanged(){
    preferences.m_selectedLanguage = ui->languageComboBox->currentText();
    loadStreamComboBox();
}

void MainWindow::qualityChanged()
{
    preferences.m_selectedQuality = ui->qualityComboBox->currentText();
    clearAndLoadTable();
}

void MainWindow::clearAndLoadTable()
{
    QString url = ui->streamComboBox->itemData(ui->streamComboBox->currentIndex()).toString();
    bool dateCurrentlyShown = false;
    if (url == DATE_STREAM_PREFIX)
    {
        dateCurrentlyShown = true;
        url.append(preferences.selectedLanguage()).append(":");
        url.append(ui->dateEdit->date().toString("yyyyMMdd"));
    }
    ui->dateEdit->setVisible(dateCurrentlyShown);
    delegate->loadPlayList(url);
}

void MainWindow::nextPage()
{
    delegate->loadNextPage();
}

void MainWindow::previousPage()
{
    delegate->loadPreviousPage();
}


bool isTeaserFromOriginalMovie(const FilmDetails& film)
{
    return !film.m_metadata.value(Preview_Or_ArteP7).isEmpty()
                                     && "ARTE+7" != film.m_metadata.value(Preview_Or_ArteP7);
}

QTableWidgetItem* MainWindow::createOrUpdateTitleColumn(int rowNumber)
{
    QTableWidgetItem* titleTableItem = ui->tableWidget->item(rowNumber, COLUMN_FOR_TITLE);
    if (titleTableItem== NULL)
    {
        titleTableItem = new QTableWidgetItem();
        ui->tableWidget->setItem(rowNumber, COLUMN_FOR_TITLE, titleTableItem);
    }

    QFontMetrics metric(ui->tableWidget->font());

    ui->tableWidget->verticalHeader()->resizeSection(rowNumber, 5*(metric.lineSpacing()));

    FilmDetails* film = delegate->visibleFilms().at(rowNumber);
    if (film == NULL) {
        return titleTableItem;
    }

    titleTableItem->setToolTip("");
    titleTableItem->setIcon(QIcon());
    if (! film->m_errors.empty())
    {
        titleTableItem->setIcon(QIcon(":/img/warning.png"));
        titleTableItem->setToolTip(film->m_errors.join("\n"));
    } else if (film->m_downloadStatus == ERROR)
    {
        titleTableItem->setToolTip(tr("Previous download failed."));
    }

    if (film->m_metadata.contains(Episode_name))
    {
        QString newTooltip = tr("Episode: %1").arg(film->m_metadata.value(Episode_name));
        QString previousTooltip = titleTableItem->toolTip();
        if (!previousTooltip.isEmpty())
        {
            newTooltip.append("\n\n").append(previousTooltip);
        }
        titleTableItem->setToolTip(newTooltip);
    }

    if (isTeaserFromOriginalMovie(*film)) {
        titleTableItem->setIcon(QIcon(":/img/locked.png"));
    }
    switch (film->m_downloadStatus){
    case ERROR:
        titleTableItem->setIcon(QIcon(":/img/error.png"));
        break;
    case CANCELLED:
        titleTableItem->setIcon(QIcon(":/img/cancelled.png"));
        break;
    case DOWNLOADING:
        titleTableItem->setIcon(QIcon(":/img/progress.png"));
        break;
    case REQUESTED:
        titleTableItem->setIcon(QIcon(":/img/waiting.png"));
        break;
    case DOWNLOADED:
        titleTableItem->setIcon(QIcon(":/img/finished.png"));
        break;
    case NONE:
    default:
        break;
    }

    titleTableItem->setText(film->m_title);

    return titleTableItem;

}

void MainWindow::refreshTable()
{

    QList<FilmDetails*> details = delegate->visibleFilms();

    ui->tableWidget->setRowCount(details.size());

    FilmDetails* film;
    int rowNumber = 0;
    foreach (film, details)
    {
        createOrUpdateTitleColumn(rowNumber);
        if (film->m_durationInMinutes > 0)
        {
            QTime duration(QTime().addSecs(film->m_durationInMinutes * 60));
            QTableWidgetItem* item = new QTableWidgetItem(duration.toString());
            item->setFlags(item->flags()^Qt::ItemIsEditable);
            ui->tableWidget->setItem(rowNumber, COLUMN_FOR_DURATION, item);
        }

        QTableWidgetItem* previewItem = ui->tableWidget->item(rowNumber, COLUMN_FOR_PREVIEW);
        if (previewItem == NULL)
        {
            previewItem = new QTableWidgetItem();
            ui->tableWidget->setItem(rowNumber, COLUMN_FOR_PREVIEW, previewItem);
        }
        if (film->m_preview.isNull())
            previewItem->setIcon(QIcon(QPixmap(":/img/Arte.jpg").scaled(TABLE_PREVIEW_MAX_WIDTH, TABLE_PREVIEW_MAX_HEIGHT, Qt::KeepAspectRatio)));
        else
            previewItem->setIcon(QIcon(QPixmap::fromImage(film->m_preview)));


        bool isEpisode=film->m_metadata.contains(Episode_name);

            if (ui->tableWidget->item(rowNumber, COLUMN_FOR_PREVIEW)) {
                ui->tableWidget->item(rowNumber, COLUMN_FOR_PREVIEW)->setBackgroundColor(isEpisode ? Qt::lightGray : Qt::white);
                ui->tableWidget->item(rowNumber, COLUMN_FOR_PREVIEW)->setToolTip(tr("Episode: %1").arg(film->m_metadata.value(Episode_name)));
            }

            if (ui->tableWidget->item(rowNumber, COLUMN_FOR_TITLE)) {
                ui->tableWidget->item(rowNumber, COLUMN_FOR_TITLE)->setBackgroundColor(isEpisode ? Qt::lightGray : Qt::white);
            }

            if (ui->tableWidget->item(rowNumber, COLUMN_FOR_DURATION)) {
                ui->tableWidget->item(rowNumber, COLUMN_FOR_DURATION)->setBackgroundColor(isEpisode ? Qt::lightGray : Qt::white);
                ui->tableWidget->item(rowNumber, COLUMN_FOR_DURATION)->setToolTip(tr("Episode: %1").arg(film->m_metadata.value(Episode_name)));
            }

        ++rowNumber;
    }

    if (ui->tableWidget->currentRow() < 0 && details.size() > 0 )
    {
        ui->tableWidget->setCurrentCell(0,0);
    }

    updateCurrentDetails();

}

const QList<MetaType>& MainWindow::listInterestingDetails() {
    static QList<MetaType> shownMetadata;
    if (shownMetadata.isEmpty())
    {
        shownMetadata // << Available_until
                << Description << First_broadcast_long << Type << Views << Rank << Episode_name;
    }
    return shownMetadata;
}

FilmDetails* MainWindow::getCurrentFilm() const {
    int rowBegin = ui->tableWidget->currentRow();

    QList<FilmDetails*> details = delegate->visibleFilms();
    if (rowBegin < 0 || rowBegin >= details.size())
        return NULL;

    return details.at(rowBegin);
}


void MainWindow::updateCurrentDetails(){
     FilmDetails* film = getCurrentFilm();
     ui->downloadButton->setEnabled(isReadyForDownload(film));
     ui->cancelSelectedFilmButton->setEnabled(film != NULL &&
             (film->m_downloadStatus == DOWNLOADING || film->m_downloadStatus == REQUESTED));
     if (film == NULL)
     {
         ui->summaryLabel->setText(tr("No film in the playlist"));
         ui->previewLabel->setPixmap(QPixmap());
         ui->previewLabel->setVisible(false);
         ui->detailsGroupBox->setTitle("");
         ui->countryYearDurationlabel->setText("");
         return;
     }
    ui->detailsGroupBox->setTitle(film->m_title);

    QString prefix;

    foreach(MetaType key, listInterestingDetails()){
        if (film->m_metadata.contains(key) && film->m_metadata.value(key) != "0")
                prefix.append(tr("<b> %0 : </b>%1<br/>").arg(FilmDetails::enum2Str(key)).arg(film->m_metadata.value(key)));
    }

    prefix.append("<br/>");
    ui->summaryLabel->setText(prefix.append(film->m_summary));
    if (!film->m_preview.isNull())
    {
        ui->previewLabel->setPixmap(QPixmap::fromImage(film->m_preview));
        ui->previewLabel->setEnabled(true);
    }
    else
    {
        ui->previewLabel->setPixmap(QPixmap(":/img/Arte.jpg").scaled(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT, Qt::KeepAspectRatio));
        ui->previewLabel->setDisabled(true);
    }

    {
        QString coutryYearDurationText;
        if (isTeaserFromOriginalMovie(*film))
            coutryYearDurationText.append(tr("The related video is a teaser of the original movie.\n"));
        if (!film->m_metadata.value(First_broadcast_long).isEmpty())
        {
            coutryYearDurationText.append(tr("%1 %2 ")
                                          .arg(FilmDetails::enum2Str(First_broadcast_long))
                                          .arg(film->m_metadata.value(First_broadcast_long)));
        }

        coutryYearDurationText.append(tr("(%1 min)").arg(QString::number(film->m_durationInMinutes)));

        if (! film->m_metadata.value(Available_until).isEmpty())
        {
            coutryYearDurationText.append(tr("\n%1").arg(film->m_metadata.value(Available_until)));
        }

        ui->countryYearDurationlabel->setText(coutryYearDurationText);
    }
    ui->extractIconLabel->setVisible(isTeaserFromOriginalMovie(*film));

     switch(film->m_downloadStatus)
     {
     case NONE:
         ui->downloadButton->setToolTip(tr("Download"));
     case DOWNLOADING:
         ui->downloadButton->setToolTip(tr("Downloading..."));
         break;
     case REQUESTED:
         ui->downloadButton->setToolTip(tr("Waiting..."));
         break;
     case DOWNLOADED:
         ui->downloadButton->setToolTip(tr("Downloaded"));
         break;
     case CANCELLED:
         ui->downloadButton->setToolTip(tr("Cancelled"));
         break;
     case ERROR:
         ui->downloadButton->setToolTip(tr("Download error"));
     }

}


QString cleanFilenameForFileSystem(const QString filename) {
    QString cleanedFilename(filename);
    // TODO les caractères HTML sont à convertir (ex:&#39; => ')

    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[éèëê]")), "e");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ÉÈËÊ]")), "E");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ô]")), "o");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Ô]")), "O");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[âáà]")), "a");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ÂÁÀ]")), "A");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[îï]")), "i");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ÎÏ]")), "I");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[û]")), "u");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Û]")), "U");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ç]")), "c");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Ç]")), "C");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ß]")), "ss");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ä]")), "ae");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Ä]")), "AE");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ö]")), "oe");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Ö]")), "OE");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[ü]")), "ue");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[Ü]")), "UE");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[/]")), "-");
    cleanedFilename.replace(QRegExp(QString::fromLocal8Bit("[^a-zA-Z0-9 _-()]")), " ");
    return cleanedFilename.simplified();
}

QString MainWindow::getFileName(const QString& targetDirectory, const QString& title, const QString& remoteFilename, int fileSuffixNumber, QString episodeName)
{
    QString extension = "flv";
    if (remoteFilename != "")
    {
        QFileInfo remoteFile(remoteFilename);
        if (remoteFile.suffix().size() == 3 || remoteFile.suffix().size() == 4)
            extension = remoteFile.suffix();
    }

    QString cleanedTitle;

    QString language(getStreamType().languageCode);
    language = language.replace(0, 1, language.left(1).toUpper());

    if (preferences.useDedicatedDirectoryForSeries() && !episodeName.isEmpty())
    {
        QRegExp episodeNumberRegExp("\\([0-9 \\-/]+\\)");

        QString serieName = QString(title).replace(episodeNumberRegExp, "");

        /* Get the episode suffix, like "(12/15)" to put it as prefix of the film title */
        QString episodeNumberText;
        if (episodeNumberRegExp.indexIn(title) >= 0)
        {
            episodeNumberText = episodeNumberRegExp.cap().append(" ");
        }

        QString filename = preferences.filenamePattern();;
        filename.replace("%title", episodeNumberText.append(episodeName))
                .replace("%language", language)
                .replace("%quality", getStreamType().qualityCode.toUpper());

        cleanedTitle = cleanFilenameForFileSystem(serieName).append(QDir::separator()).append(cleanFilenameForFileSystem(filename));
    }
    else
    {
        cleanedTitle = preferences.filenamePattern();
        cleanedTitle.replace("%title", title)
                .replace("%language", language)
                .replace("%quality", getStreamType().qualityCode.toUpper());
        cleanedTitle = cleanFilenameForFileSystem(cleanedTitle);
    }

    QString countSuffix(fileSuffixNumber > 0 ? "_" + fileSuffixNumber : "");

    QString filename("%1%2%3%4.%5");
    filename = filename.arg(targetDirectory,
                            QDir::separator(),
                            cleanedTitle,
                            countSuffix,
                            extension);
    return filename;
}

void MainWindow::downloadFilm(int currentLine, FilmDetails* film){
    QString workingPath(preferences.destinationDir());

    if (isReadyForDownload(film))
    {
        if (ui->tableWidget->item(currentLine, COLUMN_FOR_TITLE) == NULL)
        {
            qDebug() << "No title for the current row";
            return;
        }
        QString titleCellText = ui->tableWidget->item(currentLine, COLUMN_FOR_TITLE)->text();
        QString futureFileName = getFileName(workingPath, titleCellText, film->m_streamUrl, 0, film->m_metadata.value(Episode_name));

        int fileSuffixNumber = 1;
        foreach(QString otherFilmUrl, delegate->downloadList())
        {
            FilmDetails* otherFilm = delegate->findFilmByUrl(otherFilmUrl);
            if (otherFilm)
            {
                if (otherFilm->m_targetFileName == futureFileName)
                {
                    // TODO faut aussi stocker ce futureFileName dans le fichier de conf de l'appli, sinon au redémarrage, si on reprend le téléchargement dans un ordre différent, les vidéos seront mélangées.
                    futureFileName = getFileName(workingPath, titleCellText, film->m_streamUrl, fileSuffixNumber, film->m_metadata.value(Episode_name));
                }
            }
        }

        if (QFile(futureFileName).exists()
                && QMessageBox::question(this, tr("File already exists"),
                                  tr("A file with the same name already exists:\n%1\nDo you want to continue and replace it?")
                                         .arg(futureFileName),
                                  QMessageBox::Yes,
                                  QMessageBox::No)
                    == QMessageBox::No)
        {
            // TODO on ne gère pas l'écrasement du film, il faut supprimer l'ancien film si on répond yes !!
            // Il faut revoir chacun des cas... on peut être dans plusieurs cas à la fois
            film->m_downloadStatus = CANCELLED;
        }
        else if (QFile(QString(futureFileName).append(TEMP_FILE_PREFIX)).exists()
                 && QMessageBox::question(this, tr("Incomplete download found"),
                                          tr("The download of the movie <%1> has already been started. Do you want to continue the download?")
                                            .arg(titleCellText),
                                          QMessageBox::Yes,
                                          QMessageBox::No) == QMessageBox::No)
        {
            film->m_downloadStatus = CANCELLED;
        }
        else
        {
            film->m_downloadStatus = REQUESTED;
            film->m_targetFileName = futureFileName;

            delegate->addUrlToDownloadList(film->m_infoUrl); // TODO c'est trop trop moche de faire ça. Design à revoir
            thread->addFilmToDownloadQueue(film->m_infoUrl, *film);
        }
    }

    refreshTable();
}

void MainWindow::changeDownloadPartVisibility(bool isVisible)
{
    ui->progressBar->setVisible(isVisible);
    ui->pauseButton->setVisible(isVisible);
    ui->downloadTitleLabel->setVisible(isVisible);
    ui->downloadSeparatorLine->setVisible(isVisible);
    ui->cancelProgressingDownloadButton->setVisible(isVisible);
}

void MainWindow::cancelSelectedFilmDownload()
{
    FilmDetails* film = getCurrentFilm();
    if (film == NULL || film->m_downloadStatus == CANCELLED
            || film->m_downloadStatus == DOWNLOADED
            || film->m_downloadStatus == NONE
            || film->m_downloadStatus == ERROR)
        return;
    thread->cancelDownload(film->m_infoUrl);
}

void MainWindow::allFilmDownloadFinished()
{
    changeDownloadPartVisibility(false);
    ui->downloadLabel->setText("");
    statusBar()->showMessage(tr("Download finished."));
}

void MainWindow::downloadProgressed(QString filmUrl, double progression, double speed, double remainingTime)
{
    changeDownloadPartVisibility(true);
    ui->progressBar->setValue(progression);


    FilmDetails* film = delegate->findFilmByUrl(filmUrl);
    QString filmFileName("...");
    if (film)
    {
        filmFileName = film->m_targetFileName;
        film->m_downloadStatus = DOWNLOADING;
    }

    QString remainingTimeString;
    QTime remainingTimeTime(0,0,0);

    remainingTimeString = remainingTimeTime.addSecs(remainingTime).toString();
    ui->downloadLabel->setText(tr("Downloading %1\nSpeed %2 kB/s  -  Remaining: %3")
                               .arg(filmFileName)
                               .arg(speed)
                               .arg(remainingTimeString));

    statusBar()->showMessage((tr("%1 item(s) in queue").arg(thread->queueSize())));
    int filmId = delegate->getLineForUrl(filmUrl);

    if (filmId >= 0)
    {
        createOrUpdateTitleColumn(filmId);
        if (filmId == ui->tableWidget->currentRow())
            updateCurrentDetails();
    }
}
void MainWindow::downloadCancelled(QString filmUrl)
{
    FilmDetails* film = delegate->findFilmByUrl(filmUrl);
    if (film)
    {
        film->m_downloadStatus = CANCELLED;
        refreshTable();
    }
}

void MainWindow::downloadError(QString filmUrl, QString errorMsg){
    FilmDetails* film = delegate->findFilmByUrl(filmUrl);
    if (film)
    {
        film->m_errors.append(tr("Download error: %1").arg(errorMsg));
        film->m_downloadStatus = ERROR;
        refreshTable();
    }
}

void MainWindow::hasBeenPaused() {
    ui->downloadLabel->setText(tr("Paused"));
}

void MainWindow::filmDownloaded(QString filmUrl)
{

    FilmDetails * film = delegate->findFilmByUrl(filmUrl);
    if (film)
    {
        // All of that should be done in the delegate!!
        film->m_downloadStatus = DOWNLOADED;

        QFileInfo filmFile(film->m_targetFileName);

        // Save metadata
        QFile metadataFile(filmFile.absolutePath() + QDir::separator() + filmFile.completeBaseName() + ".info");
        metadataFile.open(QFile::WriteOnly|QFile::Text);
        QTextStream stream (&metadataFile);
        stream<< film->m_title << "\n";

        foreach (MetaType key, listInterestingDetails())
        {
            QString value = film->m_metadata.value(key);
            stream << FilmDetails::enum2Str(key) << ": " << value << "\n";
        }
        stream<< film->m_summary;

        stream.flush();
        metadataFile.close();

        // Save preview picture
        QFile picture(filmFile.absolutePath() + QDir::separator() + filmFile.completeBaseName() + ".png");
        picture.open(QFile::WriteOnly);
        QImageWriter writer(&picture, "PNG");
        writer.write(film->m_preview);
        picture.close();
    }

    int filmId = delegate->getLineForUrl(filmUrl);

    if (filmId >= 0)
    {
        refreshTable();
    }

    if (filmId == ui->tableWidget->currentRow())
    {
        updateCurrentDetails();
    }
}

void MainWindow::reloadCurrentRow()
{
    if (ui->tableWidget->rowCount() <=0)
        return;
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0)
        return;
    QTableWidgetItem* titleWidgetItem = ui->tableWidget->item(currentRow, COLUMN_FOR_TITLE);
    if (titleWidgetItem != NULL)
    {
        titleWidgetItem->setToolTip("");
        titleWidgetItem->setIcon(QIcon());
    }
    delegate->reloadFilm(delegate->visibleFilms()[currentRow]);
}


void MainWindow::addFilmManuallyFromUrl()
{
    QString url = QInputDialog::getText(this, tr("Add a new Film from URL"),tr("Enter the URL of your arte film page"));
    delegate->addMovieFromUrl(url);
}


void MainWindow::errorOccured(QString filmUrl, QString errorMessage)
{
    FilmDetails * film = delegate->findFilmByUrl(filmUrl);
    if (!film)
        return;
    film->m_errors.append(errorMessage);
    qDebug() << errorMessage;

    refreshTable();
}

void MainWindow::showPreferences()
{
    PreferenceDialog dial(this, preferences);
    dial.exec();
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

void MainWindow::cellHasBeenClicked(int /*row*/, int column)
{
    if (ui->progressBar->isVisible() || column != COLUMN_FOR_TITLE)
    {
        return;
    }
    try{
        /*FilmDetails * film = delegate->visibleFilms()[row];
        QTableWidgetItem* titleItem = ui->tableWidget->item(row, COLUMN_FOR_TITLE);
        if (film != NULL && titleItem != NULL)
        {
            // TODO ça ne marche pas comme ça, il faut stocker le fichier du film qu'on télécharge
            QString fileName = getFileName(preferences.destinationDir(), titleItem->text(), film->m_streamUrl);
            if (film->m_downloadStatus == DOWNLOADED)
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
        }*/

    } catch (NotFoundException&)
    {
        return;
    }

}


void MainWindow::downloadButtonClicked()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0)
        return;

    FilmDetails *details = delegate->visibleFilms()[row];
    downloadFilm(row, details);
}

void MainWindow::webPageButtonClicked()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0)
        return;

    FilmDetails * film = delegate->visibleFilms()[row];
    if (film != NULL)
    {
        QDesktopServices::openUrl(QUrl(film->m_infoUrl));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    ui->previewLabel->setVisible(width() > 950);
    event->accept();
}


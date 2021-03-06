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
#include <QIcon>
#include <QInputDialog>
#include <QList>
#include <QMessageBox>
#include <QtGui>

#include "catalogs/artemaincatalog.h"
#include "catalogs/artedatecatalog.h"
#include "catalogs/artelivecatalog.h"
#include "catalogs/incompletedownloads.h"
#include "view/preferencedialog.h"
#include "filmdelegate.h"
#include "film/filmdetails.h"
#include "downloadManager.h"
#include "view/aboutdialog.h"
#include "tools.h"

#define COLUMN_FOR_TITLE 1
#define COLUMN_FOR_DURATION 2
#define COLUMN_FOR_PREVIEW 0

#define TABLE_PREVIEW_MAX_WIDTH 100
#define TABLE_PREVIEW_MAX_HEIGHT 100
#define TABLE_COLUMN_MARGIN 10

#define PROGRESS_PEN_WIDTH 16
#define PROGRESS_Y_POS_ON_IMAGE MAX_IMAGE_HEIGHT - PROGRESS_PEN_WIDTH / 2

#define DOWNLOAD_STREAM "::download::"
#define DEFAULT_FILM_ICON ":/img/unknown"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_manager(new QNetworkAccessManager(this)),
    m_currentPreview(0),
    thread (new DownloadManager(this)),
    m_trayIcon(new QSystemTrayIcon(QIcon(":/img/icon"), this))
{
    Preferences::getInstance();
    applyProxySettings();

    ui->setupUi(this);

    this->setWindowTitle("Arte Fetcher v." + QApplication::applicationVersion());
    this->resize(Preferences::getInstance()->preferredWindowSize());
    m_trayIcon->show();



    delegate = new FilmDelegate(m_manager);
    delegate->addCatalog(new ArteMainCatalog(this));
    delegate->addCatalog(new ArteDateCatalog(this));
    delegate->addCatalog(new ArteLiveCatalog(this));

    ICatalog* incompleteCatalog = new IncompleteDownloads(this);
    delegate->addCatalog(incompleteCatalog);

    loadStreamComboBox();
    int index = ui->streamComboBox->findText(Preferences::getInstance()->catalogAtStartup());
    ui->streamComboBox->setCurrentIndex(index > 0 ? index : 0);

    if (!Preferences::getInstance()->pendingDownloads().isEmpty())
    {
        ui->streamComboBox->setCurrentIndex(ui->streamComboBox->findText(incompleteCatalog->listSupportedCatalogNames().first()));
    }

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

    ui->progressBar->setMaximum(100); // Required for Qt5

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

    ui->previewLabel->setMaximumSize(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT);
    ui->previewLabel->setMinimumSize(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT);
    ui->previewLabel->setPixmap(QPixmap(DEFAULT_FILM_ICON).scaled(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT, Qt::KeepAspectRatio));

    ui->detailsGroupBox->setStyleSheet("QGroupBox { font-size: 16px; font-weight: bold; }");

    ui->filmStreamComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    changeDownloadPartVisibility(false);

    ui->dateEdit->setVisible(false);
    ui->dateEdit->setDate(QDate::currentDate());

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->start(100 /*ms*/);
    connect(timer, SIGNAL(timeout()), SLOT(updateItemProgressBar()));

    m_imageTimer = new QTimer(this);
    m_imageTimer->setSingleShot(true);
    m_imageTimer->start(3000 /*ms*/);
    connect(m_imageTimer, SIGNAL(timeout()), SLOT(clicOnPreview()));


    connect(delegate, SIGNAL(playListHasBeenUpdated()),
            SLOT(refreshTable()));
    connect(delegate, SIGNAL(errorOccured(QString,QString)),
            SLOT(errorOccured(QString,QString)));

    connect(delegate, SIGNAL(streamIndexLoaded(int,int,int)),
            SLOT(streamIndexLoaded(int,int,int)));

    connect(delegate, SIGNAL(filmHasBeenUpdated(FilmDetails*const)),
            SLOT(filmHasBeenUpdated(FilmDetails*const)));

    connect(ui->tableWidget, SIGNAL(currentCellChanged(int,int,int,int)),
            SLOT(updateCurrentDetails()));

    connect(ui->reloadFilmButton, SIGNAL(clicked()),
            SLOT(reloadCurrentRow()));

    connect(ui->settingsButton, SIGNAL(clicked()),
            SLOT(showPreferences()));

    connect(ui->streamComboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(clearAndLoadTable()));

    connect(ui->playButton, SIGNAL(clicked()),
            SLOT(playFilm()));
    connect(ui->openDirectoryButton, SIGNAL(clicked()),
            SLOT(openFilmDirectory()));
    connect(ui->aboutButton, SIGNAL(clicked()),
            SLOT(showAboutWindow()));

    connect(ui->dateEdit, SIGNAL(dateChanged(QDate)),
            SLOT(clearAndLoadTable()));
    connect(ui->searchButton, SIGNAL(clicked()),
            SLOT(clearAndLoadTable()));

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

    connect(ui->rightPageButton, SIGNAL(clicked()),
            SLOT(nextPage()));

    connect(ui->leftPageButton, SIGNAL(clicked()),
            SLOT(previousPage()));
    connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                 this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    connect(ui->webPageButton, SIGNAL(clicked()), SLOT(webPageButtonClicked()));

    connect(ui->pauseButton, SIGNAL(clicked()), thread, SLOT(pause()));

    ui->previewLabel->installEventFilter(this);

    registerApplication();
    clearAndLoadTable();
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        this->setVisible(!this->isVisible());
        break;
    default:
        ;
    }
}

void MainWindow::loadStreamComboBox() {
    ui->streamComboBox->clear();
    ui->streamComboBox->addItems(Preferences::getInstance()->favoriteCatalogs().empty() ? delegate->listCatalogNames() : Preferences::getInstance()->favoriteCatalogs());
    ui->streamComboBox->addItem(tr("Downloads"), DOWNLOAD_STREAM);
}

void MainWindow::streamIndexLoaded(int resultCount, int currentPage, int pageCount){
    ui->pageLabel->setText(tr("Page %1/%2\nTotal results: %3").arg(currentPage).arg(pageCount).arg(resultCount));
    ui->leftPageButton->setEnabled(currentPage > 1);
    ui->rightPageButton->setEnabled(currentPage < pageCount);
    ui->tableWidget->clearContents();
    ui->tableWidget->setCurrentCell(-1, 0, QItemSelectionModel::ClearAndSelect);
}




MainWindow::~MainWindow()
{
    Preferences::getInstance()->setPreferredWindowSize(size());
    delete delegate;
    Preferences::getInstance()->save();
    Preferences::getInstance()->killInstance();
    delete ui;
}

bool MainWindow::isReadyForDownload(const FilmDetails * const film) const
{
    return film && !film->m_title.isEmpty()
            && !film->m_allStreams.isEmpty() &&
            (film->m_downloadStatus == DL_NONE || film->m_downloadStatus == DL_CANCELLED || film->m_downloadStatus == DL_ERROR);
}

bool filmWillBeDownloaded(const FilmDetails* const film)
{
    return !(film->m_downloadStatus == DL_NONE || film->m_downloadStatus == DL_CANCELLED || film->m_downloadStatus == DL_ERROR);
}

void MainWindow::updateItemProgressBar(){
    if (!ui->playlistProgressBar->isVisible()){
        ui->playlistProgressBar->setMaximum(0);
    }
    const int currentMax = ui->playlistProgressBar->maximum();
    const int newCount(m_manager->findChildren<QNetworkReply*>().size());
    const int newMax(newCount > currentMax ? newCount : currentMax);

    ui->playlistProgressBar->setMaximum(newMax);
    ui->playlistProgressBar->setValue(newMax-newCount);
    ui->playlistProgressBar->setVisible(newCount > 0);
}

void MainWindow::clearAndLoadTable()
{
    bool dateCurrentlyShown = delegate->isDateCatalog(ui->streamComboBox->currentText());
    bool searchEditShown = false;

    ui->searchEdit->setVisible(searchEditShown);
    ui->searchButton->setVisible(searchEditShown);
    ui->dateEdit->setVisible(dateCurrentlyShown);

    ui->tableWidget->clearContents();
    ui->pageLabel->setText(tr("Loading..."));

    delegate->loadPlayList(ui->streamComboBox->currentText(), ui->dateEdit->date());
}

void MainWindow::nextPage()
{
    delegate->loadPage(ui->streamComboBox->currentText(), 1);
}

void MainWindow::previousPage()
{
    delegate->loadPage(ui->streamComboBox->currentText(), -1);
}


bool isTeaserFromOriginalMovie(const FilmDetails& film)
{
    return !film.m_metadata.value(Preview_Or_ArteP7).isEmpty()
                                     && "ARTE+7" != film.m_metadata.value(Preview_Or_ArteP7) && "ARTE Concert" != film.m_metadata.value(Preview_Or_ArteP7);
}

bool hasFilmAnEpisodeName(const FilmDetails* const film){
    return !film->m_metadata.value(Episode_name).isEmpty();
}

bool isFilmAnEpisode(const FilmDetails* const film){
    return (film->m_episodeNumber > 0) || hasFilmAnEpisodeName(film);
}

void appendWithNewLine(QString& stringToChange, const QString & addition)
{
    if (!stringToChange.isEmpty())
    {
        stringToChange.append("\n");
    }
    stringToChange.append(addition);
}

QString buildTooltip(const FilmDetails* const film){
    QString tooltip;

    if (hasFilmAnEpisodeName(film))
    {
        tooltip = QObject::tr("Episode: %1").arg(film->m_metadata.value(Episode_name));
    }

    if (! film->m_errors.empty())
    {
        appendWithNewLine(tooltip, film->m_errors.join("\n"));
    }

    switch (film->m_downloadStatus)
    {
    case DL_REQUESTED:
        appendWithNewLine(tooltip, QObject::tr("Download pending"));
        break;
    case DL_DOWNLOADING:
        appendWithNewLine(tooltip, QObject::tr("Download in progress: %1%2").arg(film->m_downloadProgress).arg("%"));
        break;
    case DL_DOWNLOADED:
        appendWithNewLine(tooltip, QObject::tr("Download finished"));
        break;
    case DL_CANCELLED:
        appendWithNewLine(tooltip, QObject::tr("Download has been cancelled."));
        break;
    case DL_ERROR:
        appendWithNewLine(tooltip, QObject::tr("Previous download failed."));
        break;
    default:
    case DL_NONE:
        break;
    }

    return tooltip;
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

    titleTableItem->setIcon(QIcon());
    if (! film->m_errors.empty())
    {
        titleTableItem->setIcon(QIcon(":/img/warning"));
    }

    if (isTeaserFromOriginalMovie(*film)) {
        titleTableItem->setIcon(QIcon(":/img/locked"));
    }
    switch (film->m_downloadStatus){
    case DL_ERROR:
        titleTableItem->setIcon(QIcon(":/img/error"));
        break;
    case DL_CANCELLED:
        titleTableItem->setIcon(QIcon(":/img/cancelled"));
        break;
    case DL_DOWNLOADING:
        titleTableItem->setIcon(QIcon(":/img/progress"));
        break;
    case DL_REQUESTED:
        titleTableItem->setIcon(QIcon(":/img/waiting"));
        break;
    case DL_DOWNLOADED:
        titleTableItem->setIcon(QIcon(":/img/finished"));
        break;
    case DL_NONE:
    default:
        break;
    }

    titleTableItem->setText(film->m_title);
    titleTableItem->setFlags(titleTableItem->flags()^Qt::ItemIsEditable);

    return titleTableItem;

}

void MainWindow::refreshTable()
{
    QList<FilmDetails*> details = delegate->visibleFilms();

    ui->tableWidget->setRowCount(details.size());

    FilmDetails* film;
    foreach (film, details)
    {
        updateRowInTable(film);
    }

    if (ui->tableWidget->currentRow() < 0 && details.size() > 0 )
    {
        ui->tableWidget->setCurrentCell(0,0);
    }

    updateCurrentDetails();

}

void MainWindow::updateRowInTable(const FilmDetails* const film){
    if (!film)
        return;

    QList<int> rows = delegate->getLineForUrl(delegate->getFilmUniqueKey(film));

    int rowNumber;
    foreach (rowNumber, rows)
    {
        createOrUpdateTitleColumn(rowNumber);
        QString durationString;
        if (film->m_durationInMinutes > 0)
        {

            QTime duration(0,0);
            duration = duration.addSecs(film->m_durationInMinutes * 60);
            durationString = duration.toString();
        }
        QTableWidgetItem* durationItem = new QTableWidgetItem(durationString);
        durationItem->setFlags(durationItem->flags()^Qt::ItemIsEditable);
        ui->tableWidget->setItem(rowNumber, COLUMN_FOR_DURATION, durationItem);

        QTableWidgetItem* previewItem = ui->tableWidget->item(rowNumber, COLUMN_FOR_PREVIEW);
        if (previewItem == NULL)
        {
            previewItem = new QTableWidgetItem();
            previewItem->setFlags(previewItem->flags()^Qt::ItemIsEditable);
            ui->tableWidget->setItem(rowNumber, COLUMN_FOR_PREVIEW, previewItem);
        }
        if (film->m_preview.isEmpty())
            previewItem->setIcon(QIcon(QPixmap(DEFAULT_FILM_ICON).scaled(TABLE_PREVIEW_MAX_WIDTH, TABLE_PREVIEW_MAX_HEIGHT, Qt::KeepAspectRatio)));
        else {
            QImage image = film->m_preview.values().first();
            if (film->m_downloadProgress > 0) {
                QPainter painter(&image);

                painter.setPen(QPen(Qt::green, PROGRESS_PEN_WIDTH));
                painter.drawLine(0,PROGRESS_Y_POS_ON_IMAGE,film->m_downloadProgress*MAX_IMAGE_WIDTH/100,PROGRESS_Y_POS_ON_IMAGE);
            }
            previewItem->setIcon(QIcon(QPixmap::fromImage(image)));

        }

        bool isEpisode = isFilmAnEpisode(film);

        QString tooltip(buildTooltip(film));

        if (ui->tableWidget->item(rowNumber, COLUMN_FOR_PREVIEW)) {
            ui->tableWidget->item(rowNumber, COLUMN_FOR_PREVIEW)->setBackgroundColor(isEpisode ? Qt::lightGray : Qt::white);
            ui->tableWidget->item(rowNumber, COLUMN_FOR_PREVIEW)->setToolTip(tooltip);
        }

        if (ui->tableWidget->item(rowNumber, COLUMN_FOR_TITLE)) {
            ui->tableWidget->item(rowNumber, COLUMN_FOR_TITLE)->setBackgroundColor(isEpisode ? Qt::lightGray : Qt::white);
            ui->tableWidget->item(rowNumber, COLUMN_FOR_TITLE)->setToolTip(tooltip);
        }

        if (ui->tableWidget->item(rowNumber, COLUMN_FOR_DURATION)) {
            ui->tableWidget->item(rowNumber, COLUMN_FOR_DURATION)->setBackgroundColor(isEpisode ? Qt::lightGray : Qt::white);
            ui->tableWidget->item(rowNumber, COLUMN_FOR_DURATION)->setToolTip(tooltip);
        }

    }
}



const QList<MetaType>& MainWindow::listInterestingDetails() {
    static QList<MetaType> shownMetadata;
    if (shownMetadata.isEmpty())
    {
        shownMetadata // << Available_until
                << Description // << RAW_First_Broadcast
                << Type << Views << Episode_name << Production_year << Shooting_date << Director;
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

bool MainWindow::fileExistForTheFilm(const FilmDetails * const film) const {
    if (film == NULL)
        return false;
    QString filename = film->m_targetFileName.isEmpty() ? getFileName(film)
                                                        : film->m_targetFileName;
    return QFile(filename).exists() || QFile(filename.append(".part")).exists();
}

void MainWindow::updateCurrentDetails() {
     FilmDetails* film = getCurrentFilm();
     if (film == NULL)
         return;

    ui->detailsGroupBox->setTitle(film->m_title);

    QString prefix;

    if (!film->m_producingCountries.empty()){
        prefix.append(tr("<b>Countries: </b>"));
        prefix.append(film->m_producingCountries.join(", "));
        prefix.append("<br/>");
    }
    foreach(MetaType key, listInterestingDetails()){
        if (film->m_metadata.contains(key) && film->m_metadata.value(key) != "0")
                prefix.append(tr("<b> %0 : </b>%1<br/>").arg(FilmDetails::enum2Str(key)).arg(film->m_metadata.value(key)));
    }

    prefix.append("<br/>");

    ui->summaryLabel->setText(prefix.append(film->m_summary));

    {
        QString coutryYearDurationText;
        if (isTeaserFromOriginalMovie(*film))
            coutryYearDurationText.append(tr("The related video is a teaser of the original movie.\n"));

        if (film->m_durationInMinutes > 0) {
            coutryYearDurationText.append(tr("(%1 min)\n").arg(QString::number(film->m_durationInMinutes)));
        }

        if (!film->m_metadata.value(RAW_First_Broadcast).isEmpty())
        {
            QString firstBroadcastString =  film->m_metadata.value(RAW_First_Broadcast);
            coutryYearDurationText.append(tr("%1 %2 ")
                                          .arg(FilmDetails::enum2Str(RAW_First_Broadcast))
                                          .arg(firstBroadcastString));
        }

        if (! film->m_metadata.value(RAW_Available_until).isEmpty())
        {
            QString availableUntil =film->m_metadata.value(RAW_Available_until);
            coutryYearDurationText.append(tr("\nAvailable until %1").arg(availableUntil));
        }

        ui->countryYearDurationlabel->setText(coutryYearDurationText);
    }
    ui->extractIconLabel->setVisible(isTeaserFromOriginalMovie(*film));

    clicOnPreview(false);

    updateFilmStreamCombobox(film);

    // Ceci doit être fait après updateFilmStreamCombobox car updateFilmStreamCombobox calcule le type de vidéo à télécharger donc le nom du fichier
    updateButtonsVisibility(film);

}

void MainWindow::updateFilmStreamCombobox(FilmDetails * const film) {
    // Gestion assez complexe de filmStream combobox
    disconnect(ui->filmStreamComboBox, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(streamTypeChanged()));
    ui->filmStreamComboBox->clear();
    if (film->m_allStreams.keys().size()){
        ui->filmStreamComboBox->addItems(film->m_allStreams.keys());
    }

    if (ui->filmStreamComboBox->isVisible()){
        int streamTypeIndexToSelect = -1;
        if (! film->m_choosenStreamType.isEmpty()){
            streamTypeIndexToSelect = ui->filmStreamComboBox->findText(film->m_choosenStreamType);
        } else {
            int indexInFavorites = 0;
            while (streamTypeIndexToSelect < 0 && indexInFavorites < Preferences::getInstance()->favoriteStreamTypes().size()) {
                streamTypeIndexToSelect = ui->filmStreamComboBox->findText(Preferences::getInstance()->favoriteStreamTypes().at(indexInFavorites));
                indexInFavorites++;
            }
        }
        // If nothing has been found, that means the stream name is unknown. Then we fallback to the first item
        if (streamTypeIndexToSelect == -1 && ui->filmStreamComboBox->count()){
            streamTypeIndexToSelect = 0;
        }
        if (streamTypeIndexToSelect >= 0){
            film->m_choosenStreamType = ui->filmStreamComboBox->itemText(streamTypeIndexToSelect);
            ui->filmStreamComboBox->setCurrentIndex(streamTypeIndexToSelect);

            connect(ui->filmStreamComboBox, SIGNAL(currentIndexChanged(QString)),
                    SLOT(streamTypeChanged()));
        }
    }
}
void MainWindow::updateButtonsVisibility(const FilmDetails * const film){
    bool hasDownloadStarted = film && (film->m_downloadStatus == DL_DOWNLOADING || film->m_downloadStatus == DL_DOWNLOADED || film->m_downloadStatus == DL_ERROR || fileExistForTheFilm(film));

    ui->openDirectoryButton->setVisible(hasDownloadStarted);
    ui->playButton->setVisible(hasDownloadStarted);
    ui->downloadButton->setVisible(isReadyForDownload(film));
    ui->cancelSelectedFilmButton->setVisible(film != NULL &&
            (film->m_downloadStatus == DL_DOWNLOADING || film->m_downloadStatus == DL_REQUESTED));
    ui->filmStreamComboBox->setVisible(ui->filmStreamComboBox->count());
    ui->filmStreamComboBox->setEnabled(!filmWillBeDownloaded(film));
}

void MainWindow::downloadFilm(FilmDetails* film){
    if (!isReadyForDownload(film))
        return;

    if (film->m_choosenStreamType.isEmpty()){
        film->m_choosenStreamType = ui->filmStreamComboBox->currentText();
    }

    QString remoteUrl = film->m_allStreams[ui->filmStreamComboBox->currentText()];
    // keep existing file name, in case the download has been requested in a previous execution
    // if this name is empty, build a filename.
    QString futureFileName = film->m_targetFileName;
    if (futureFileName.isEmpty())
    {
        futureFileName = getFileName(film);
    }

    // Check the file (not .part) does not exist.
    if (QFile(futureFileName).exists()
            && QMessageBox::question(this, tr("File already exists"),
                              tr("A file with the same name already exists:\n%1\nDo you want to continue and replace it?")
                                     .arg(futureFileName),
                              QMessageBox::Yes,
                              QMessageBox::No)
                == QMessageBox::No)
    {
        film->m_downloadStatus = DL_CANCELLED;
    }
    else
    {
        film->m_downloadStatus = DL_REQUESTED;
        film->m_targetFileName = futureFileName;

        delegate->addUrlToDownloadList(delegate->getFilmUniqueKey(film)); // TODO c'est trop trop moche de faire ça. Design à revoir
        thread->addFilmToDownloadQueue(delegate->getFilmUniqueKey(film), remoteUrl, futureFileName);
    }

    updateRowInTable(film);
    updateCurrentDetails();
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
    if (film == NULL || film->m_downloadStatus == DL_CANCELLED
            || film->m_downloadStatus == DL_DOWNLOADED
            || film->m_downloadStatus == DL_NONE
            || film->m_downloadStatus == DL_ERROR)
        return;
    thread->cancelDownload(delegate->getFilmUniqueKey(film));
}

void MainWindow::playFilm() {
    FilmDetails* film = getCurrentFilm();
    if (film == NULL)
        return;

    QString filePath = film->m_targetFileName;
    if (filePath.isEmpty())
        filePath = getFileName(film);
    if (! QFile(filePath).exists())
    {
        filePath = filePath.append(".part");
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

void MainWindow::openFilmDirectory() {
    FilmDetails* film = getCurrentFilm();
    if (film == NULL || film->m_downloadStatus == DL_ERROR)
        return;

    QFileInfo filmFile(film->m_targetFileName);
    QDesktopServices::openUrl(QUrl::fromLocalFile(filmFile.absolutePath()));
}

void MainWindow::streamTypeChanged() {
    FilmDetails* film = getCurrentFilm();
    // Si film->m_choosenStreamType.isEmpty() ça veut dire qu'on n'a encore jamais appliqué l'algorithme
    // permettant de prendre le soustitrage favori. Il ne faut donc pas prendre en compte la combobox
    if (film == NULL || film->m_choosenStreamType.isEmpty() || ui->filmStreamComboBox->currentText().isEmpty()){
        return;
    }
    film->m_choosenStreamType = ui->filmStreamComboBox->currentText();
    updateButtonsVisibility(film);
}

void MainWindow::showAboutWindow() {
    AboutDialog popup(this);
    popup.exec();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->previewLabel && event->type() == QEvent::MouseButtonPress)
    {
        clicOnPreview();
        return true;
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::clicOnPreview(bool fromTimer) {
    if (fromTimer)
    {
        m_currentPreview = m_currentPreview + 1;
    }

    FilmDetails* film = getCurrentFilm();
    if (film == NULL)
        return;

    updateBigImageVisibility();
    if (!film->m_preview.isEmpty())
    {
        ui->previewLabel->setPixmap(QPixmap::fromImage(film->m_preview.values().value(m_currentPreview % film->m_preview.size())));
    }
    else
    {
        ui->previewLabel->setPixmap(QPixmap(DEFAULT_FILM_ICON).scaled(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT, Qt::KeepAspectRatio));
    }

    if (fromTimer)
    {
        m_imageTimer->start();
    }
}

void MainWindow::allFilmDownloadFinished()
{
    changeDownloadPartVisibility(false);
    ui->downloadLabel->setText("");
    statusBar()->showMessage(tr("Download finished."));
}

void MainWindow::downloadProgressed(QString filmKey, double progression, double kBytesPersecond, double remainingTimeForCurrentFilm)
{
    changeDownloadPartVisibility(true);

    FilmDetails* film = delegate->findFilmByKey(filmKey);
    if (!film)
        return;

    QString filmFileName("...");

    filmFileName = film->m_targetFileName;
    film->m_downloadStatus = DL_DOWNLOADING;
    film->m_downloadProgress = progression;

    double totalProgress = delegate->computeTotalDownloadProgress();
    double totalFilmsDuration = delegate->computeTotalDownloadRequestedDuration();
    ui->progressBar->setValue(totalProgress);

    double filmLeftSize = (100. - progression) * film->m_durationInMinutes;

    if (remainingTimeForCurrentFilm != 0 && filmLeftSize != 0)
    {
        double speed = filmLeftSize  / remainingTimeForCurrentFilm;

        double tailleRestanteTotale = (100. - totalProgress) * totalFilmsDuration;
        double tempsTotal = tailleRestanteTotale / speed;

        QTime remainingTimeTime(0,0,0);
        QString remainingTimeString(remainingTimeTime.addSecs(tempsTotal).toString());
        ui->downloadLabel->setText(tr("Downloading %1\nSpeed %2 kB/s  -  Remaining: %3")
                                   .arg(filmFileName)
                                   .arg(QString::number(kBytesPersecond,'f', 1))
                                   .arg(remainingTimeString));
    }

    statusBar()->showMessage((tr("%1 item(s) in queue").arg(thread->queueSize())));

    updateRowInTable(film);
    if (getCurrentFilm() == film){
        updateButtonsVisibility(film);
    }
}

void MainWindow::downloadCancelled(QString filmKey)
{
    FilmDetails* film = delegate->findFilmByKey(filmKey);
    if (film)
    {
        film->m_downloadStatus = DL_CANCELLED;
        updateRowInTable(film);
        if (getCurrentFilm() == film){
            updateButtonsVisibility(film);
        }
    }
}

void MainWindow::downloadError(QString filmKey, QString errorMsg){
    FilmDetails* film = delegate->findFilmByKey(filmKey);
    if (film)
    {
        film->m_errors.append(tr("Download error: %1").arg(errorMsg));
        film->m_downloadStatus = DL_ERROR;
        updateRowInTable(film);
        updateCurrentDetails();
    }
}

void MainWindow::hasBeenPaused() {
    ui->downloadLabel->setText(tr("Paused"));
}

void MainWindow::filmDownloaded(QString filmKey)
{
    FilmDetails * film = delegate->findFilmByKey(filmKey);
    if (film == NULL)
        return;
    // All of that should be done in the delegate!!
    film->m_downloadStatus = DL_DOWNLOADED;

    QFileInfo filmFile(film->m_targetFileName);

    // Save metadata
    if (Preferences::getInstance()->saveMetaInInfoFile()) {
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
    }

    // Save preview picture
    if (Preferences::getInstance()->saveImagePreview()) {
        int count(1);
        foreach(QImage image, film->m_preview.values()) {
            QFile picture(QString("%1%2%3_%4.png").arg(filmFile.absolutePath(), QDir::separator(), filmFile.completeBaseName(), QString::number(count)));
            picture.open(QFile::WriteOnly);
            QImageWriter writer(&picture, "PNG");
            writer.write(image);
            picture.close();
            ++count;
        }
    }

    updateRowInTable(film);
    updateCurrentDetails();
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

void MainWindow::errorOccured(QString filmKey, QString errorMessage)
{
    FilmDetails * film = delegate->findFilmByKey(filmKey);
    if (!film)
        return;
    film->m_errors.append(errorMessage);
    qDebug() << errorMessage << " for " << filmKey;

    updateRowInTable(film);
    updateCurrentDetails();
}

void MainWindow::showPreferences()
{
    PreferenceDialog dial(this, delegate->listCatalogNames());

    if (QDialog::Accepted == dial.exec())
    {
        applyProxySettings();
        clearAndLoadTable();
        loadStreamComboBox();
    }
}

void MainWindow::applyProxySettings() {
    if (!Preferences::getInstance()->proxyEnabled())
    {
        QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    }
    else
    {
        m_userDefinedProxy.setType(QNetworkProxy::HttpProxy);
        m_userDefinedProxy.setHostName(Preferences::getInstance()->proxyHttpUrl());
        m_userDefinedProxy.setPort(Preferences::getInstance()->proxyHttpPort());
        QNetworkProxy::setApplicationProxy(m_userDefinedProxy);
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

void MainWindow::downloadButtonClicked()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0)
        return;

    FilmDetails *details = delegate->visibleFilms()[row];
    downloadFilm(details);
}

void MainWindow::webPageButtonClicked()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0)
        return;

    FilmDetails * film = delegate->visibleFilms()[row];
    if (film != NULL)
    {
        QDesktopServices::openUrl(QUrl(film->relatedWebPage()));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    updateBigImageVisibility();
    event->accept();
}

void MainWindow::updateBigImageVisibility(){
    ui->previewLabel->setVisible(width() > 950);
}

void MainWindow::filmHasBeenUpdated(const FilmDetails * const film){
    if (film == NULL)
        return;
    if (film == getCurrentFilm()){
        updateCurrentDetails();
    }
    updateRowInTable(film);
}

void MainWindow::registerApplication(){
    if (Preferences::getInstance()->registrationAgreement()){
        QNetworkAccessManager * manager = new QNetworkAccessManager(this);
        QString url = "http://goo.gl/VShoxJ"; // This URL should be change for every version
        // web browser is used to make the difference between first run and 20th run.

#ifdef Q_OS_WIN32
        QString osName = "Windows NT 5.1";
#elif defined Q_OS_MAC
        QString osName = "Macintosh; Intel Mac OS X 10_9_2";
#elif defined Q_OS_LINUX
        QString osName = "X11; Linux x86_64";
#else // Unknown
        QString osName = "iPad; CPU OS 5_1 like Mac OS X";
#endif


        QNetworkRequest req = QNetworkRequest(QUrl(url));
        req.setRawHeader("Referer", QString("http://v%1").arg(QApplication::applicationVersion()).toStdString().c_str());

        if (!Preferences::getInstance()->firstRegistrationDone()){
            QString userAgent = QString("Mozilla/5.0 (%1; rv:30.0) Gecko/20100101 Firefox/30.0").arg(osName);
            qDebug() << "First registration, useragent:" << userAgent;
            req.setRawHeader("User-Agent", userAgent.toStdString().c_str());
            QNetworkReply* registrationReply = manager->get(QNetworkRequest(req));
            connect(registrationReply, SIGNAL(finished()), SLOT(firstRegistrationResult()));

        }
        if (!Preferences::getInstance()->secondRegistrationDone()
                && Preferences::getInstance()->startAppCount() >= USAGES_NEEDED_FOR_SECOND_REGISTRATION){
            QString userAgent = QString("Mozilla/5.0 (%1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/36.0.1985.67 Safari/537.36").arg(osName);
            qDebug() << "Second registration, useragent:" << userAgent;
            req.setRawHeader("User-Agent" , userAgent.toStdString().c_str());
            QNetworkReply* registrationReply = manager->get(QNetworkRequest(req));
            connect(registrationReply, SIGNAL(finished()), SLOT(secondRegistrationResult()));
        }
    }
}

void MainWindow::firstRegistrationResult(){
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
    if (reply->error() == QNetworkReply::NoError){
        Preferences::getInstance()->setFirstRegistrationDone();
    } else {
        qDebug() << "Registration failed: " << reply->error() << reply->errorString();
    }
}

void MainWindow::secondRegistrationResult(){
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>(sender());
    if (reply->error() == QNetworkReply::NoError){
        Preferences::getInstance()->setSecondRegistrationDone();
    } else {
        qDebug() << "Registration failed: " << reply->error() << reply->errorString();
    }
}

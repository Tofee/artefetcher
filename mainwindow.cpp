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
#include <rtmpthread.h>


#define COLUMN_FOR_PAGE 1
#define COLUMN_FOR_TITLE 0
#define FIRST_CHECKBOX_COLUMN_IN_TABLE 0

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    manager(new QNetworkAccessManager(this)),
        thread (new RTMPThread(checkedFilms, this))
{
    preferences.load();

    ui->setupUi(this);

    delegate = new FilmDelegate(manager);

    QStringList header;
    header << tr("Title") << tr("Rating")
           << tr("Views") << tr("Duration");

    ui->tableWidget->setColumnCount(header.size());
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->progressBar->setMaximum(100);

    ui->languageComboBox->addItems(FilmDelegate::listLanguages());
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findText(preferences.selectedLanguage()));
    ui->qualityComboBox->addItems(FilmDelegate::listQualities());
    ui->qualityComboBox->setCurrentIndex(ui->qualityComboBox->findText(preferences.selectedQuality())); // TODO tooltip pour les qualités, c'est pas du tout intuitif

    ui->streamComboBox->addItem("Popular", "http://www.arte.tv/guide/fr/plus7/popular.json");
    ui->streamComboBox->addItem("Arte likes", "http://www.arte.tv/guide/fr/plus7/recommended.json");
    ui->streamComboBox->addItem("Last chance", "http://www.arte.tv/guide/fr/plus7/last_chance.json");
    ui->streamComboBox->addItem("All", "http://www.arte.tv/guide/fr/plus7/all.json");

    connect(ui->tableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)),
            SLOT(changeColumnChecking(int)));

    connect(delegate, SIGNAL(playListHasBeenUpdated()),
            SLOT(refreshTable()));
    connect(delegate, SIGNAL(errorOccured(int,QString)),
            SLOT(errorOccured(int,QString)));

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
            this, SLOT(clearAndLoadTable()));

    connect(ui->languageComboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(languageChanged()));
    connect(ui->qualityComboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(qualityChanged()));


    ui->progressBar->setVisible(false);


    connect(thread, SIGNAL(allFilmDownloadFinished()),
            this, SLOT(allFilmDownloadFinished()));
    connect(thread, SIGNAL(downloadProgressed(int,StreamType,double,double)),
            this, SLOT(downloadProgressed(int,StreamType,double,double)));
    connect(thread, SIGNAL(downloadFinished(int,StreamType)),
            this, SLOT(filmDownloaded(int,StreamType)));

    connect(ui->downloadButton, SIGNAL(clicked()), SLOT(downloadButtonClicked()));

    connect(delegate, SIGNAL(streamIndexLoaded(int,int,int)),
            SLOT(streamIndexLoaded(int,int,int)));

    connect(ui->rightPageButton, SIGNAL(clicked()),
            this, SLOT(nextPage()));

    connect(ui->leftPageButton, SIGNAL(clicked()),
            this, SLOT(previousPage()));

    clearAndLoadTable();
}

void MainWindow::streamIndexLoaded(int resultCount, int currentPage, int pageCount){
    ui->pageLabel->setText(QString("%1/%2").arg(currentPage).arg(pageCount));
    ui->leftPageButton->setEnabled(currentPage <= 1);
    ui->rightPageButton->setEnabled(currentPage < pageCount);

}

StreamType MainWindow::getStreamType() const
{
    return FilmDelegate::getStreamTypeByLanguageAndQuality(preferences.m_selectedLanguage, preferences.m_selectedQuality);
}
void MainWindow::changeColumnChecking(int column)
{
    try {
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
    preferences.save();
    delete ui;
    delete delegate;
}

bool MainWindow::isReadyForDownload(const FilmDetails * const film)
{
    return !film->m_title.isEmpty()
            && film->m_streamsByType.contains(getStreamType()) && !film->m_isDownloading;
}

void MainWindow::languageChanged(){
    preferences.m_selectedLanguage = ui->languageComboBox->currentText();
}

void MainWindow::qualityChanged()
{
    preferences.m_selectedQuality = ui->qualityComboBox->currentText();
}

void MainWindow::clearAndLoadTable()
{
    QString url = ui->streamComboBox->itemData(ui->streamComboBox->currentIndex()).toString();
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

void MainWindow::createOrUpdateFirstColumn(int rowNumber)
{
    FilmDetails* film = delegate->films().values().at(rowNumber);
    QTableWidgetItem* titleTableItem = ui->tableWidget->item(rowNumber, FIRST_CHECKBOX_COLUMN_IN_TABLE);
    if (titleTableItem != NULL)
    {
        delete titleTableItem;
    }
    titleTableItem = new QTableWidgetItem();

    bool isDownloadRequested = film->m_streamsByType.value(getStreamType()).use;

    if (film->m_isDownloading){
        titleTableItem->setIcon(QIcon(":/img/progress.png"));
    } else if (isDownloadRequested) {
        titleTableItem->setIcon(QIcon(":/img/clock.png"));
    }

    titleTableItem->setText(film->m_title);

    ui->tableWidget->setItem(rowNumber, FIRST_CHECKBOX_COLUMN_IN_TABLE, titleTableItem);
}

void MainWindow::refreshTable()
{

    QMap<QString, FilmDetails*> details = delegate->films();
    FilmDetails* film;
    int rowNumber = 0;
    foreach (film, details)
    {
        createOrUpdateFirstColumn(rowNumber);

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
     if (details.values().size()<=rowBegin)
         return;
     FilmDetails*  film = details.values().at(rowBegin);
     if (rowBegin >=0 && rowBegin < ui->tableWidget->rowCount())
     {
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

     bool isDownloadButtonClickable= false;
     if (film->m_isDownloading)
         ui->downloadButton->setText("Downloading...");
     else if (film->m_streamsByType[getStreamType()].use){
         ui->downloadButton->setText("Waiting...");
     }
     else
     {
         ui->downloadButton->setText("Download");
         isDownloadButtonClickable = isReadyForDownload(film);
     }
     ui->downloadButton->setEnabled(isDownloadButtonClickable);
}


QString MainWindow::getFileName(const QString& targetDirectory, const QString& title)
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

    QString language(getStreamType().languageCode);
    language = language.replace(0, 1, language.left(1).toUpper());

    QString baseName(preferences.filenamePattern());
    baseName.replace("%title", cleanedTitle)
            .replace("%language", language)
            .replace("%quality", getStreamType().qualityCode.toUpper());

    QString filename("%1%2%3");
    filename = filename.arg(targetDirectory,
                            QDir::separator(),
                            baseName);
    return filename;
}

void MainWindow::downloadFilm(int currentLine, FilmDetails* film){
        qDebug() << "BEGIN MainWindow::downloadFilm(): Add " << film->title() << " to download";
    QString workingPath(QDir::homePath().append(QDir::separator()).append("arteFetcher"));

    StreamType streamType = getStreamType();

    if (isReadyForDownload(film))
    {
        QString titleCellText = ui->tableWidget->item(currentLine, COLUMN_FOR_TITLE)->text();
        QString futureFileName = getFileName(workingPath, titleCellText);
        if (QFile(futureFileName).exists()
                && QMessageBox::question(this, "File already exists",
                                  tr("A file has already the name of the film: <%1>.\nDo you want to download it anyway?")
                                         .arg(titleCellText),
                                  QMessageBox::Yes,
                                  QMessageBox::No)
                    == QMessageBox::No)
        {
            ui->tableWidget->item(currentLine, FIRST_CHECKBOX_COLUMN_IN_TABLE)->setCheckState(Qt::Unchecked);
            film->m_streamsByType[streamType].use = false;
            return;
        }
        else
        {
            qDebug() << film->m_streamsByType[streamType].m_rtmpStreamUrl;

            film->m_streamsByType[streamType].use = true;
            film->m_streamsByType[streamType].m_targetFileName = futureFileName;
        }
    }

    // 3) Check the destination directory
    QDir workingDir(workingPath);
    if (!workingDir.exists() && ! workingDir.mkdir(workingPath))
    {
        statusBar()->showMessage(tr("Cannot create the working directory %1").arg(workingPath));
        return;
    }

    checkedFilms.insert(currentLine, *film);
    qDebug() << "MainWindow::downloadFilm(): Add " << film->title() << " to download";
    thread->addFilmToDownloadQueue(currentLine, *film);
    updateCurrentDetails();
}

void MainWindow::allFilmDownloadFinished()
{
//    lockWidgets(false);
    ui->progressBar->setVisible(false);
    statusBar()->showMessage(tr("Download finished."));
}

void MainWindow::downloadProgressed(int filmId, StreamType streamType, double progression, double speed)
{
    ui->progressBar->setVisible(true);

    delegate->films().values().at(filmId)->m_isDownloading = true;
    createOrUpdateFirstColumn(filmId);

    ui->progressBar->setValue(progression);
    statusBar()->showMessage(
                (tr("Downloading %0 speed: %1 KiB/s (%2)     -    %3 item(s) in queue")
                 .arg(delegate->films().values().at(filmId)->title())
                             .arg(speed)
                 .arg(streamType.humanCode).arg(thread->queueSize())));
    if (filmId == ui->tableWidget->currentRow())
        updateCurrentDetails();
}

void MainWindow::filmDownloaded(int filmId, StreamType streamType)
{

    QTableWidgetItem* item = ui->tableWidget->item(filmId, FIRST_CHECKBOX_COLUMN_IN_TABLE);
    //delete item;
    //item = new QTableWidgetItem();
    item->setIcon(QIcon(":/img/finished.png"));
    ui->tableWidget->setItem(filmId, FIRST_CHECKBOX_COLUMN_IN_TABLE, item);
    item->setFlags(item->flags()^Qt::ItemIsUserCheckable);

    delegate->films().values().at(filmId)->m_isDownloading = false;
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

void MainWindow::cellHasBeenClicked(int row, int column)
{
    if (ui->progressBar->isVisible() || column != FIRST_CHECKBOX_COLUMN_IN_TABLE)
    {
        return;
    }
    try{
        StreamType streamType = getStreamType();
        FilmDetails * film = delegate->films().values().value(row);
        if (film != NULL)
        {
            QString fileName = getFileName(preferences.destinationDir(), ui->tableWidget->item(row, 0)->text());
            if (film->m_streamsByType.contains(streamType)
                    && film->m_streamsByType[streamType].downloaded)
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
        }

    } catch (NotFoundException e)
    {
        return;
    }

}


void MainWindow::downloadButtonClicked()
{
    int row = ui->tableWidget->currentRow();

    FilmDetails *details = delegate->films().values().value(row);
    downloadFilm(row, details);
}

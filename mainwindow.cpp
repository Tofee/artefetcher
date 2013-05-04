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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    manager(new QNetworkAccessManager(this)),
        thread (new DownloadManager(this))
{
    preferences.load();

    ui->setupUi(this);

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

    QFontMetrics metric(ui->tableWidget->font());

    ui->tableWidget->horizontalHeader()->resizeSection(COLUMN_FOR_DURATION, metric.boundingRect("999 min.").width());
    ui->tableWidget->horizontalHeader()->resizeSection(COLUMN_FOR_PREVIEW, 110);
    ui->tableWidget->horizontalHeader()->resizeSection(COLUMN_FOR_TITLE, metric.boundingRect("Sample of a fitting title").width());
    ui->tableWidget->setIconSize(QSize(100,100));

    ui->progressBar->setMaximum(100);

    ui->languageComboBox->addItems(FilmDelegate::listLanguages());
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findText(preferences.selectedLanguage()));
    ui->qualityComboBox->addItems(FilmDelegate::listQualities());
    ui->qualityComboBox->setCurrentIndex(ui->qualityComboBox->findText(preferences.selectedQuality()));

    ui->streamComboBox->addItem("Sélection Arte", "http://www.arte.tv/guide/fr/plus7/selection.json");
    ui->streamComboBox->addItem("Plus récentes", "http://www.arte.tv/guide/fr/plus7/plus_recentes.json");
    ui->streamComboBox->addItem("Plus vues", "http://www.arte.tv/guide/fr/plus7/plus_vues.json");
    ui->streamComboBox->addItem("Dernière chance", "http://www.arte.tv/guide/fr/plus7/derniere_chance.json");
    ui->streamComboBox->addItem("Tout", "http://www.arte.tv/guide/fr/plus7.json");
    ui->streamComboBox->addItem(tr("Downloads"), "about:downloads");

    ui->previewLabel->setMaximumSize(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT);
    ui->previewLabel->setMinimumSize(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT);
    ui->previewLabel->setPixmap(QPixmap(":/img/Arte.jpg").scaled(MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT, Qt::KeepAspectRatio));


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
            this, SLOT(clearAndLoadTable()));

    connect(ui->languageComboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(languageChanged()));
    connect(ui->qualityComboBox, SIGNAL(currentIndexChanged(int)),
            SLOT(qualityChanged()));


    ui->progressBar->setVisible(false);


    connect(thread, SIGNAL(signalAllFilmDownloadFinished()),
            this, SLOT(allFilmDownloadFinished()));
    connect(thread, SIGNAL(signalDownloadProgressed(QString,double,double, double)),
            this, SLOT(downloadProgressed(QString,double,double, double)));
    connect(thread, SIGNAL(signalDownloadFinished(QString)),
            this, SLOT(filmDownloaded(QString)));

    connect(ui->downloadButton, SIGNAL(clicked()), SLOT(downloadButtonClicked()));

    connect(delegate, SIGNAL(streamIndexLoaded(int,int,int)),
            SLOT(streamIndexLoaded(int,int,int)));

    connect(ui->rightPageButton, SIGNAL(clicked()),
            this, SLOT(nextPage()));

    connect(ui->leftPageButton, SIGNAL(clicked()),
            this, SLOT(previousPage()));

    clearAndLoadTable();
}

void MainWindow::streamIndexLoaded(int /*resultCount*/, int currentPage, int pageCount){
    ui->pageLabel->setText(QString("%1/%2").arg(currentPage).arg(pageCount));
    ui->leftPageButton->setEnabled(currentPage > 1);
    ui->rightPageButton->setEnabled(currentPage < pageCount);
    ui->tableWidget->clearContents();
}

StreamType MainWindow::getStreamType() const
{
    return FilmDelegate::getStreamTypeByLanguageAndQuality(preferences.m_selectedLanguage, preferences.m_selectedQuality);
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
            && !film->m_streamUrl.isEmpty() && !film->m_isDownloading;
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
    FilmDetails* film = delegate->visibleFilms().at(rowNumber);
    QTableWidgetItem* titleTableItem = ui->tableWidget->item(rowNumber, COLUMN_FOR_TITLE);
    if (titleTableItem== NULL)
    {
        titleTableItem = new QTableWidgetItem();
        ui->tableWidget->setItem(rowNumber, COLUMN_FOR_TITLE, titleTableItem);
    }

    QFontMetrics metric(ui->tableWidget->font());

    ui->tableWidget->verticalHeader()->resizeSection(rowNumber, 5*(metric.lineSpacing()));

    if (film->m_isDownloading){
        titleTableItem->setIcon(QIcon(":/img/progress.png"));
    } else if (film->m_hasBeenRequested) {
        if (film->m_isDownloaded)
            titleTableItem->setIcon(QIcon(":/img/finished.png"));
        else
            titleTableItem->setIcon(QIcon(":/img/clock.png"));
    }

    titleTableItem->setText(film->m_title);

}

void MainWindow::refreshTable()
{

    QList<FilmDetails*> details = delegate->visibleFilms();

    int previousCount = ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(details.size());

    FilmDetails* film;
    int rowNumber = 0;
    foreach (film, details)
    {
        createOrUpdateFirstColumn(rowNumber);
        if (film->m_durationInMinutes > 0)
        {
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(film->m_durationInMinutes));
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
            previewItem->setIcon(QIcon(QPixmap(":/img/Arte.jpg").scaled(100, 100, Qt::KeepAspectRatio)));
        else
            previewItem->setIcon(QIcon(QPixmap::fromImage(film->m_preview)));

        ++rowNumber;
    }

    if (previousCount <= 0 && details.size() > 0 )
    {
        ui->tableWidget->setCurrentCell(0,0);
    }

    updateCurrentDetails();

}

const QList<MetaType>& MainWindow::interestingDetails() {
    static QList<MetaType> shownMetadata;
    if (shownMetadata.isEmpty())
    {
        shownMetadata << Available_until << Description << First_broadcast << Type << Views << Rank;
    }
    return shownMetadata;
}

void MainWindow::updateCurrentDetails(){

    int rowBegin = ui->tableWidget->currentRow();

     QList<FilmDetails*> details = delegate->visibleFilms();
     if (rowBegin < 0 || rowBegin >= details.size())
         return;
     FilmDetails* film = details.at(rowBegin);
     if (rowBegin >=0 && rowBegin < ui->tableWidget->rowCount())
     {
        QString prefix;

        foreach(MetaType key, interestingDetails()){
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
        ui->detailsGroupBox->setTitle(film->m_title);
        {
            ui->countryYearDurationlabel->setText(tr("Broadcasted %2 (%3min)")
                                                  .arg(film->m_metadata.value(First_broadcast),
                                                       QString::number(film->m_durationInMinutes)));
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
     else if (film->m_hasBeenRequested){
         if (film->m_isDownloaded)
             ui->downloadButton->setText("Downloaded");
         else
            ui->downloadButton->setText("Waiting...");
     }
     else
     {
         ui->downloadButton->setText("Download");
         isDownloadButtonClickable = isReadyForDownload(film);
     }
     ui->downloadButton->setEnabled(isDownloadButtonClickable);
}


QString MainWindow::getFileName(const QString& targetDirectory, const QString& title, const QString& remoteFilename)
{
    QString extension = "flv";
    if (remoteFilename != "")
    {
        QFileInfo remoteFile(remoteFilename);
        if (remoteFile.suffix().size() == 3 || remoteFile.suffix().size() == 4)
            extension = remoteFile.suffix();
    }

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

    QString filename("%1%2%3.%4");
    filename = filename.arg(targetDirectory,
                            QDir::separator(),
                            baseName, extension);
    return filename;
}

void MainWindow::downloadFilm(int currentLine, FilmDetails* film){
    QString workingPath(QDir::homePath().append(QDir::separator()).append("arteFetcher"));

    if (isReadyForDownload(film))
    {
        if (ui->tableWidget->item(currentLine, COLUMN_FOR_TITLE) == NULL)
        {
            qDebug() << "No title for the current row";
            return;
        }
        QString titleCellText = ui->tableWidget->item(currentLine, COLUMN_FOR_TITLE)->text();
        QString futureFileName = getFileName(workingPath, titleCellText, film->m_streamUrl);
        if (QFile(futureFileName).exists()
                && QMessageBox::question(this, tr("File already exists"),
                                  tr("A file has already the name of the film: <%1>.\nYou can either continue the download (already fetched data will be kept) or cancel.\nDo you want to continue the download?")
                                         .arg(titleCellText),
                                  QMessageBox::Yes,
                                  QMessageBox::No)
                    == QMessageBox::No)
        {
            film->m_hasBeenRequested = false;
        }
        else
        {
            film->m_hasBeenRequested = true;
            film->m_targetFileName = futureFileName;
            // 3) Check the destination directory
            QDir workingDir(workingPath);
            if (!workingDir.exists() && ! workingDir.mkdir(workingPath))
            {
                statusBar()->showMessage(tr("Cannot create the working directory %1").arg(workingPath));
                return;
            }

            delegate->addUrlToDownloadList(film->m_infoUrl); // TODO c'est trop trop moche de faire ça. Design à revoir
            thread->addFilmToDownloadQueue(film->m_infoUrl, *film);
        }
    }

    refreshTable();
}

void MainWindow::allFilmDownloadFinished()
{
    ui->progressBar->setVisible(false);
    ui->downloadLabel->setText("");
    statusBar()->showMessage(tr("Download finished."));
}

void MainWindow::downloadProgressed(QString filmUrl, double progression, double speed, double remainingTime)
{
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(progression);


    FilmDetails* film = delegate->findFilmByUrl(filmUrl);
    QString filmFileName("...");
    if (film)
    {
        filmFileName = film->m_targetFileName;
        film->m_isDownloading = true;
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
        createOrUpdateFirstColumn(filmId);
        if (filmId == ui->tableWidget->currentRow())
            updateCurrentDetails();
    }
}

void MainWindow::filmDownloaded(QString filmUrl)
{

    FilmDetails * film = delegate->findFilmByUrl(filmUrl);
    if (film)
    {
        // All of that should be done in the delegate!!
        film->m_isDownloaded = true;
        film->m_isDownloading = false;

        // Save metadata
        QFile metadataFile(QString(film->m_targetFileName).append(".info"));
        metadataFile.open(QFile::WriteOnly|QFile::Text);
        QTextStream stream (&metadataFile);
        stream<< film->m_title << "\n";

        foreach (MetaType key,interestingDetails())
        {
            QString value = film->m_metadata.value(key);
            stream << FilmDetails::enum2Str(key) << ": " << value << "\n";
        }
        stream<< film->m_summary;

        stream.flush();
        metadataFile.close();

        // Save preview picture
        QFile picture(QString(film->m_targetFileName).append(".png"));
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
    int filmId = delegate->getLineForUrl(filmUrl);
    if (ui->tableWidget->rowCount() <= filmId)
        return;
    QTableWidgetItem *cell = ui->tableWidget->item(filmId, COLUMN_FOR_TITLE);
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
    if (ui->progressBar->isVisible() || column != COLUMN_FOR_TITLE)
    {
        return;
    }
    try{
        FilmDetails * film = delegate->visibleFilms()[row];
        QTableWidgetItem* titleItem = ui->tableWidget->item(row, COLUMN_FOR_TITLE);
        if (film != NULL && titleItem != NULL)
        {
            QString fileName = getFileName(preferences.destinationDir(), titleItem->text(), film->m_streamUrl);
            if (film->m_isDownloaded)
                QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
        }

    } catch (NotFoundException&)
    {
        return;
    }

}


void MainWindow::downloadButtonClicked()
{
    int row = ui->tableWidget->currentRow();

    FilmDetails *details = delegate->visibleFilms()[row];
    downloadFilm(row, details);
}

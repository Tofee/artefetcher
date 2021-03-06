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

#include "preferencedialog.h"
#include "ui_preferencedialog.h"
#include "../catalogs/incompletedownloads.h"
#include <QtGui>
#include <QFileDialog>

#include <filmdelegate.h>


PreferenceDialog::PreferenceDialog(QWidget *parent, QStringList catalogs) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog)
{
    ui->setupUi(this);

    ui->languageComboBox->addItems(StreamType::listLanguages());
    ui->languageComboBox->setCurrentIndex(ui->languageComboBox->findText(Preferences::getInstance()->applicationLanguage()));

    ui->qualityComboBox->addItems(StreamType::listQualities());
    ui->qualityComboBox->setCurrentIndex(ui->qualityComboBox->findText(Preferences::getInstance()->selectedQuality()));

    ui->destinationDirectoryLineEdit->setText(Preferences::getInstance()->m_destinationDir);
    ui->filenamePatternLineEdit->setText(Preferences::getInstance()->m_filenamePattern);

    ui->seriesDirectoryCheckBox->setChecked(Preferences::getInstance()->m_dedicatedDirectoryForSeries);

    ui->metaInfoCheckBox->setChecked(Preferences::getInstance()->m_saveMetaInInfoFile);
    ui->imagePreviewCheckBox->setChecked(Preferences::getInstance()->m_saveImagePreview);

    ui->proxyCheckBox->setChecked(Preferences::getInstance()->m_proxyEnabled);
    ui->proxyHttpUrlLineEdit->setText(Preferences::getInstance()->m_proxyHttpUrl);
    ui->proxyHttpPortSpinBox->setValue(Preferences::getInstance()->m_proxyHttpPort);
    QString countPerPageString = QString::number(Preferences::getInstance()->m_resultCountPerPage);
    ui->resultCountComboBox->setCurrentIndex(ui->resultCountComboBox->findText(countPerPageString) < 0 ? ui->resultCountComboBox->count() - 1: ui->resultCountComboBox->findText(countPerPageString));

    ui->favoriteStreamListWidget->addItems(Preferences::getInstance()->favoriteStreamTypes());

    ui->registrationCheckBox->setToolTip(Preferences::registrationAgreementText());
    ui->registrationCheckBox->setChecked(Preferences::getInstance()->registrationAgreement());

    foreach (QString item, catalogs){
        QListWidgetItem* widgetItem = new QListWidgetItem(item);

        bool isLocked = item == IncompleteDownloads(this).listSupportedCatalogNames().first();
        bool checked = Preferences::getInstance()->favoriteCatalogs().empty() || Preferences::getInstance()->favoriteCatalogs().contains(item);
        widgetItem->setCheckState(isLocked || checked ? Qt::Checked : Qt::Unchecked);
        widgetItem->setFlags(isLocked ? (widgetItem->flags() & ~Qt::ItemIsUserCheckable) : (widgetItem->flags() | Qt::ItemIsUserCheckable)); // TODO les téléchargements inachevés ne sont pas décochables
        ui->subscriptionListWidget->addItem(widgetItem);
    }

    updateProxyConfigVisibility();

    connect(ui->browsePushButton, SIGNAL(clicked()),
            this, SLOT(browse()));

    connect(ui->destinationDirectoryLineEdit, SIGNAL(textChanged(QString)),
            SLOT(checkIsAcceptable()));
    connect(ui->filenamePatternLineEdit, SIGNAL(textChanged(QString)),
            SLOT(checkIsAcceptable()));

    connect(ui->proxyCheckBox, SIGNAL(clicked()),
            SLOT(updateProxyConfigVisibility()));

    connect(ui->upPushButton, SIGNAL(clicked()),
            SLOT(upStreamType()));
    connect(ui->downPushButton, SIGNAL(clicked()),
            SLOT(downStreamType()));

    connect(ui->subscriptionListWidget, SIGNAL(itemChanged(QListWidgetItem*)),
            SLOT(reloadCatalogsAtStartup()));

    reloadCatalogsAtStartup();

    int index = ui->startupCatalogComboBox->findText(Preferences::getInstance()->catalogAtStartup());
    ui->startupCatalogComboBox->setCurrentIndex(index > 0 ? index : 0);
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
}

void PreferenceDialog::accept()
{
    Preferences::getInstance()->m_destinationDir = ui->destinationDirectoryLineEdit->text();
    Preferences::getInstance()->m_filenamePattern = ui->filenamePatternLineEdit->text();

    Preferences::getInstance()->m_dedicatedDirectoryForSeries = ui->seriesDirectoryCheckBox->isChecked();

    Preferences::getInstance()->m_favoriteStreamTypes.clear();
    for (int i = 0; i < ui->favoriteStreamListWidget->count(); i++){
        Preferences::getInstance()->m_favoriteStreamTypes << ui->favoriteStreamListWidget->item(i)->text();
    }

    Preferences::getInstance()->m_saveMetaInInfoFile = ui->metaInfoCheckBox->isChecked();
    Preferences::getInstance()->m_saveImagePreview = ui->imagePreviewCheckBox->isChecked();

    Preferences::getInstance()->m_resultCountPerPage = ui->resultCountComboBox->currentText().toInt();

    Preferences::getInstance()->m_proxyEnabled = ui->proxyCheckBox->isChecked();
    Preferences::getInstance()->m_proxyHttpUrl = ui->proxyHttpUrlLineEdit->text();
    Preferences::getInstance()->m_proxyHttpPort = ui->proxyHttpPortSpinBox->value();

    Preferences::getInstance()->m_selectedQuality = ui->qualityComboBox->currentText();

    Preferences::getInstance()->m_applicationLanguage = ui->languageComboBox->currentText();

    Preferences::getInstance()->m_registrationAgreement = ui->registrationCheckBox->isChecked();

    //
    QStringList favoriteCatalogs;
    for (int i = 0; i < ui->subscriptionListWidget->count(); ++i){
        QListWidgetItem* item = ui->subscriptionListWidget->item(i);
        if (item->checkState() == Qt::Checked){
            favoriteCatalogs << item->text();
        }
    }
    Preferences::getInstance()->m_favoriteCatalogs.clear();
    if (favoriteCatalogs.size() != ui->subscriptionListWidget->count()){
        Preferences::getInstance()->m_favoriteCatalogs.append(favoriteCatalogs);
    }

    Preferences::getInstance()->m_catalogAtStartup = ui->startupCatalogComboBox->currentText();

    QDialog::accept();
}

void PreferenceDialog::reloadCatalogsAtStartup(){
    QString previousSelection = ui->startupCatalogComboBox->currentText();

    QStringList checkedItems;
    QStringList allItems;
    ui->startupCatalogComboBox->clear();
    for (int i = 0; i < ui->subscriptionListWidget->count(); ++i){
        QListWidgetItem* item = ui->subscriptionListWidget->item(i);
        if (item->checkState() == Qt::Checked){
            checkedItems.append(item->text());
        }
        allItems.append(item->text());
    }
    ui->startupCatalogComboBox->addItems(checkedItems.empty() ? allItems : checkedItems);

    int index = ui->startupCatalogComboBox->findText(previousSelection);
    ui->startupCatalogComboBox->setCurrentIndex(index > 0 ? index : 0);
}

void PreferenceDialog::browse()
{
    QString newPath = QFileDialog::getExistingDirectory(this, tr("Target directory"), Preferences::getInstance()->destinationDir());
    if (! newPath.isEmpty())
        ui->destinationDirectoryLineEdit->setText(newPath);
}

void PreferenceDialog::checkIsAcceptable()
{
    QString errorMessage;
    if (! QDir(ui->destinationDirectoryLineEdit->text()).exists())
    {
        errorMessage = tr("Target directory does not exist");
    }

    if (ui->filenamePatternLineEdit->text().isEmpty() || !ui->filenamePatternLineEdit->text().contains("%title"))
    {
        errorMessage = tr("Filename pattern is not correct");
    }
    ui->errorLabel->setText(errorMessage);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(errorMessage.isEmpty());
}

void PreferenceDialog::updateProxyConfigVisibility() {
    bool toBeVisible = ui->proxyCheckBox->isChecked();

    ui->proxyHttpUrlLabel->setVisible(toBeVisible);
    ui->proxyHttpUrlLineEdit->setVisible(toBeVisible);
    ui->proxyHttpPortLabel->setVisible(toBeVisible);
    ui->proxyHttpPortSpinBox->setVisible(toBeVisible);
}


void PreferenceDialog::upStreamType(){
    int currentIndex = ui->favoriteStreamListWidget->currentRow();
    if (currentIndex > 0){
        QListWidgetItem* item = ui->favoriteStreamListWidget->takeItem(currentIndex);
        ui->favoriteStreamListWidget->insertItem(currentIndex - 1, item);
        ui->favoriteStreamListWidget->setCurrentRow(currentIndex - 1);
    }
}

void PreferenceDialog::downStreamType(){
    int currentIndex = ui->favoriteStreamListWidget->currentRow();
    if (currentIndex < ui->favoriteStreamListWidget->count() - 1){
        QListWidgetItem* item = ui->favoriteStreamListWidget->takeItem(currentIndex);
        ui->favoriteStreamListWidget->insertItem(currentIndex + 1, item);
        ui->favoriteStreamListWidget->setCurrentRow(currentIndex + 1);
    }
}

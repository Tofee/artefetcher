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
#include <QtGui>
#include <QFileDialog>

PreferenceDialog::PreferenceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog)
{
    ui->setupUi(this);

    ui->destinationDirectoryLineEdit->setText(Preferences::getInstance()->m_destinationDir);
    ui->filenamePatternLineEdit->setText(Preferences::getInstance()->m_filenamePattern);

    ui->seriesDirectoryCheckBox->setChecked(Preferences::getInstance()->m_dedicatedDirectoryForSeries);

    ui->metaInfoCheckBox->setChecked(Preferences::getInstance()->m_saveMetaInInfoFile);
    ui->imagePreviewCheckBox->setChecked(Preferences::getInstance()->m_saveImagePreview);

    connect(ui->browsePushButton, SIGNAL(clicked()),
            this, SLOT(browse()));

    connect(ui->destinationDirectoryLineEdit, SIGNAL(textChanged(QString)),
            SLOT(checkIsAcceptable()));
    connect(ui->filenamePatternLineEdit, SIGNAL(textChanged(QString)),
            SLOT(checkIsAcceptable()));
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

    Preferences::getInstance()->m_saveMetaInInfoFile = ui->metaInfoCheckBox->isChecked();
    Preferences::getInstance()->m_saveImagePreview = ui->imagePreviewCheckBox->isChecked();


    QDialog::accept();
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

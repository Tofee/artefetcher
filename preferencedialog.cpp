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

PreferenceDialog::PreferenceDialog(QWidget *parent,
                                   Preferences& preferences) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog),
    m_preferences(preferences)
{
    ui->setupUi(this);

    ui->destinationDirectoryLineEdit->setText(m_preferences.m_destinationDir);
    ui->filenamePatternLineEdit->setText(m_preferences.m_filenamePattern);

    connect(ui->browsePushButton, SIGNAL(clicked()),
            this, SLOT(browse()));
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
}


void PreferenceDialog::accept()
{
    // TODO must : regarder si les caractères du pattern collent avec le systeme de fichier
    m_preferences.m_destinationDir = ui->destinationDirectoryLineEdit->text();
    m_preferences.m_filenamePattern = ui->filenamePatternLineEdit->text();
    QDialog::accept();
}

void PreferenceDialog::browse()
{
    m_preferences.m_destinationDir = QFileDialog::getExistingDirectory(this, tr("Target directory"), m_preferences.destinationDir());
}

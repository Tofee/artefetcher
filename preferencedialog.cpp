#include "preferencedialog.h"
#include "ui_preferencedialog.h"
#include <QtGui>

PreferenceDialog::PreferenceDialog(QWidget *parent,
                                   Preferences& preferences,
                                   const QStringList& cookieProfiles) :
    QDialog(parent),
    ui(new Ui::PreferenceDialog),
    m_preferences(preferences)
{
    ui->setupUi(this);

    ui->firefoxProfileComboBox->addItems(cookieProfiles);
    int profileIndex = ui->firefoxProfileComboBox->findText(m_preferences.firefoxProfile());
    if (profileIndex >= 0)
    {
        ui->firefoxProfileComboBox->setCurrentIndex(profileIndex);
    }

    QStringList availableStreams;
    for (QList<StreamType>::const_iterator it= FilmDelegate::listStreamTypes().constBegin();
                 it != FilmDelegate::listStreamTypes().constEnd(); ++it)
    {
        availableStreams << (*it).humanCode;
    }

    QString stream;
    foreach (stream, availableStreams)
    {
        QListWidgetItem * it = new QListWidgetItem;
        it->setText(stream);
        it->setFlags(it->flags()|Qt::ItemIsUserCheckable);
        if (preferences.selectedStreams().size() == 0 || preferences.selectedStreams().contains(stream))
        {
            it->setCheckState(Qt::Checked);
        }
        else
        {
            it->setCheckState(Qt::Unchecked);
        }
        ui->wishedStreamListWidget->addItem(it);
    }

    ui->destinationDirectoryLineEdit->setText(m_preferences.destinationDir());

    ui->filenamePatternLineEdit->setText(m_preferences.filenamePattern());


    connect(ui->browsePushButton, SIGNAL(clicked()),
            this, SLOT(browse()));
}

PreferenceDialog::~PreferenceDialog()
{
    delete ui;
}


void PreferenceDialog::accept()
{
    m_preferences.m_selectedStreams.clear();
    QList<int> result;
    for (int row = 0; row < ui->wishedStreamListWidget->count(); ++row)
    {
        QListWidgetItem* streamItem = ui->wishedStreamListWidget->item(row);
        if (streamItem->checkState() == Qt::Checked)
        {
            m_preferences.m_selectedStreams << streamItem->text();
            result << row;
        }
    }

    // TODO must : regarder si les caractères du pattern collent avec le systeme de fichier
    m_preferences.m_destinationDir = ui->destinationDirectoryLineEdit->text();
    m_preferences.m_filenamePattern = ui->filenamePatternLineEdit->text();
    m_preferences.m_firefoxProfile = ui->firefoxProfileComboBox->currentText();
    m_preferences.save();
    QDialog::accept();
}

void PreferenceDialog::browse()
{
    m_preferences.m_destinationDir = QFileDialog::getExistingDirectory(this, tr("Target directory"), m_preferences.destinationDir());
}

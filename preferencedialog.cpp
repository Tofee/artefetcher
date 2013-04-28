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

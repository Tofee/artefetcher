#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QFile>
#include <QTextStream>

#define PROJECT_URL "https://sourceforge.net/projects/artefetcher/"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    resize(500, 600);
    ui->imageLabel->setPixmap(QPixmap(":/img/arteFetcher").scaled(200, 156));
    ui->textLabel->setText("<html>Arte Fetcher v. "+QApplication::applicationVersion()+"<br/>"
                           + "Arte Fetcher is developped by Emmanuel Quincerot, under GPL-v3 license<br/>"
                           + "Website: <a href='" + PROJECT_URL + "'>"
                           + PROJECT_URL + "</a><br/><br/>"
                           + "This application is using the Qt framework ("+ QT_VERSION_STR +")</html>");

    QFile file(":/doc/Readme");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->textBrowser->setText(tr("Release notes are not available"));
             return;
    }
    QTextStream stream(&file);
    ui->textBrowser->setText(stream.readAll());
    file.close();
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

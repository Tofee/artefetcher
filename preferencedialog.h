#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include <QDialog>
#include <preferences.h>

namespace Ui {
class PreferenceDialog;
}

class PreferenceDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PreferenceDialog(QWidget *parent,
                              Preferences& preferences);
    ~PreferenceDialog();
public slots:
    void accept();
private slots:
    void browse();
private:
    Ui::PreferenceDialog *ui;
    Preferences& m_preferences;
};

#endif // PREFERENCEDIALOG_H

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
    explicit PreferenceDialog(QWidget *parent);
    ~PreferenceDialog();
public slots:
    void accept();
private slots:
    void browse();
    void checkIsAcceptable();
    void updateProxyConfigVisibility();

    void upStreamType();
    void downStreamType();

private:
    Ui::PreferenceDialog *ui;
};

#endif // PREFERENCEDIALOG_H

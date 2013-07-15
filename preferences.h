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

#ifndef PREFERENCES_H
#define PREFERENCES_H
#include <QStringList>
#include <QString>
#include <QSettings>
#include <QSet>
#include <QSize>

class IPreferences
{
public:
    friend class PreferenceDialog;
    friend class MainWindow;

    IPreferences(){}

    /**
     * @brief selectedStreams
     * @return if empty, it means all
     */
    const QString &selectedLanguage() const
    {
        return m_selectedLanguage;
    }
    const QString &selectedQuality() const
    {
        return m_selectedQuality;
    }

    const QString &filenamePattern() const
    {
        return m_filenamePattern;
    }
    const QString &destinationDir() const
    {
        return m_destinationDir;
    }
    const QSet<QString> &pendingDownloads() const
    {
        return m_pendingDownloads;
    }
    void setPendingDonwloads(QSet<QString> downloadUrls){
        m_pendingDownloads = downloadUrls;
    }

    const QSize &preferredWindowSize() const
    {
        return m_preferredWindowSize;
    }

    void setPreferredWindowSize(QSize newSize)
    {
        m_preferredWindowSize = newSize;
    }

    virtual void load() = 0;
    virtual void save() = 0;

protected:
    QString m_selectedLanguage;
    QString m_selectedQuality;
    QString m_filenamePattern;
    QString m_destinationDir;
    QSize   m_preferredWindowSize;
    QSet<QString> m_pendingDownloads;
};


class Preferences: public IPreferences
{
public:
    Preferences();
    void load();
    void save();
private:
    QSettings settings;
};

#endif // PREFERENCES_H

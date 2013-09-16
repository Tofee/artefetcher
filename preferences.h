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
    virtual ~IPreferences(){}

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
    void setPendingDownloads(QSet<QString> downloadUrls){
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

    bool useDedicatedDirectoryForSeries() const {
        return m_dedicatedDirectoryForSeries;
    }

    void setUseDedicatedDirectoryForSeries(bool useInTheFuture) {
        m_dedicatedDirectoryForSeries = useInTheFuture;
    }

    bool saveImagePreview() const {
        return m_saveImagePreview;
    }

    void setSaveImagePreview(bool newValue) {
        m_saveImagePreview = newValue;
    }

    bool saveMetaInInfoFile() const {
        return m_saveMetaInInfoFile;
    }

    void setSaveMetaInInfoFile(bool newValue) {
        m_saveMetaInInfoFile = newValue;
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
    bool    m_dedicatedDirectoryForSeries;
    bool    m_saveImagePreview;
    bool    m_saveMetaInInfoFile;
};


class Preferences: public IPreferences
{
private:
    Preferences();
public:
    void load();
    void save();

    static Preferences* getInstance() {
        if (_singleton == NULL)
        {
            _singleton = new Preferences();
        }
        return _singleton;
    }

    static void killInstance() {
        if (_singleton != NULL)
        {
            delete _singleton;
            _singleton = NULL;
        }
    }

private:
    QSettings settings;

private:
    static Preferences* _singleton;

};

#endif // PREFERENCES_H

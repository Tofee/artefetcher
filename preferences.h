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


#define PREFERENCES_FILMMAP_ARTEID "arteId"
#define PREFERENCES_FILMMAP_TITLE "title"
#define PREFERENCES_FILMMAP_FILMURL "url"
#define PREFERENCES_FILMMAP_DESC "desc"
#define PREFERENCES_FILMMAP_VIDEOQUALITY "vQual"
#define PREFERENCES_FILMMAP_VIDEOURL "vUrl"
#define PREFERENCES_FILMMAP_IMAGE "image"
#define PREFERENCES_FILMMAP_DURATION "duration"
#define PREFERENCES_FILMMAP_EPISODE_NAME "episodeName"
#define PREFERENCES_FILMMAP_EPISODE_NUMBER "episode#"

#define USAGES_NEEDED_FOR_SECOND_REGISTRATION 20

class IPreferences
{
public:
    friend class PreferenceDialog;
//    friend class MainWindow;

    IPreferences()
        : m_dedicatedDirectoryForSeries(false), m_saveImagePreview(true), m_saveMetaInInfoFile(true), m_resultCountPerPage(10), m_proxyEnabled(false), m_proxyHttpPort(3128)
    {}
    virtual ~IPreferences(){}

    const QString &applicationLanguage()        const { return m_applicationLanguage; }
    const QString &selectedQuality()            const { return m_selectedQuality; }
    const QString &filenamePattern()            const { return m_filenamePattern; }
    const QString &destinationDir()             const { return m_destinationDir; }

    const QList<QVariant> &pendingDownloads()   const { return m_pendingDownloads; }
    const QSize &preferredWindowSize()          const { return m_preferredWindowSize; }
    bool useDedicatedDirectoryForSeries()       const { return m_dedicatedDirectoryForSeries; }
    bool saveImagePreview()                     const { return m_saveImagePreview; }
    bool saveMetaInInfoFile()                   const { return m_saveMetaInInfoFile; }
    int resultCountPerPage()                    const { return m_resultCountPerPage ? m_resultCountPerPage : 100000; }
    const QStringList favoriteStreamTypes()     const { return m_favoriteStreamTypes; }

    bool proxyEnabled()                         const { return m_proxyEnabled; }
    QString proxyHttpUrl()                      const { return m_proxyHttpUrl; }
    bool proxyHttpPort()                        const { return m_proxyHttpPort; }

    bool registrationAgreement()                const { return m_registrationAgreement; }
    bool firstRegistrationDone()                const { return m_firstRegistration; }
    bool secondRegistrationDone()               const { return m_secondRegistration; }
    int startAppCount()                         const { return m_startAppCount; }

    void setPendingDownloads(QList<QVariant> downloadUrls)      { m_pendingDownloads = downloadUrls; }
    void setPreferredWindowSize(QSize newSize)                  { m_preferredWindowSize = newSize; }
    void setFirstRegistrationDone()                             { m_firstRegistration = true; }
    void setSecondRegistrationDone()                            { m_secondRegistration = true; }

    void addStreamName(QString streamName)                      { m_favoriteStreamTypes.append(streamName); }

    virtual void load() = 0;
    virtual void save() = 0;

    static QString registrationAgreementText(){
        return QObject::tr("<html>Artefetcher would like to have your agreement to register you as an ArteFetcher user.<br/><br/>"
          "This will be helpful to know how much this application is used and thus give further motivation to the developers to provide improvements.<br/><br/>"
          "This will be done through a simple download of this page http://artefetcher.sourceforge.net through a google short URL.<br/>"
          "<b>The IP address and the OS type are the only private data that will be sent. ArteFetcher developpers team will not have access to the IP address, but only google will.</b><br/>"
          "This registration will occur at the first run and after %1 arteFetcher starts.</html>").arg(USAGES_NEEDED_FOR_SECOND_REGISTRATION);
    }

protected:
    QString m_applicationLanguage;
    QString m_selectedQuality;
    QString m_filenamePattern;
    QString m_destinationDir;
    QSize   m_preferredWindowSize;
    QList<QVariant> m_pendingDownloads;
    bool    m_dedicatedDirectoryForSeries;
    bool    m_saveImagePreview;
    bool    m_saveMetaInInfoFile;
    // 0 if all results shown in the page
    int     m_resultCountPerPage;
    /* Stream types in a favorite order */
    QStringList m_favoriteStreamTypes;

    /* Proxy : */
    bool    m_proxyEnabled;
    QString m_proxyHttpUrl;
    short   m_proxyHttpPort;

    bool    m_registrationAgreement;// User agree to register
    int     m_startAppCount;// Number of times
    bool    m_firstRegistration;//the first registration has been done
    bool    m_secondRegistration;//the second registration has been done
};


class Preferences: public IPreferences
{
private:
    Preferences();
    void load();
public:
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

/**
 * @brief compareVersions Compares two versions
 * @param v1 first version
 * @param v2 second version
 * @return depends on v1 and v2 :
 *  if v1 == v2 returns 0
 *  if v1 < v2 returns < 0
 *  if v1 > v2 returns > 0
 */
int compareVersions(QString v1, QString v2);

QStringList listVideoStreamTypes();

#endif // PREFERENCES_H

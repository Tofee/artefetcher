#ifndef PREFERENCES_H
#define PREFERENCES_H
#include <QStringList>
#include <QString>
#include <QSettings>
#include <QSet>

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

    virtual void load() = 0;
    virtual void save() = 0;

protected:
    QString m_selectedLanguage;
    QString m_selectedQuality;
    QString m_filenamePattern;
    QString m_destinationDir;
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

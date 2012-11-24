#ifndef PREFERENCES_H
#define PREFERENCES_H
#include <QStringList>
#include <QString>
#include <QSettings>

class IPreferences
{
public:
    friend class PreferenceDialog;

    IPreferences(){}

    /**
     * @brief selectedStreams
     * @return if empty, it means all
     */
    const QStringList &selectedStreams() const
    {
        return m_selectedStreams;
    }

    /**
     * @brief firefoxProfile
     * @return if empty string, it means any string
     */
    const QString &firefoxProfile() const
    {
        return m_firefoxProfile;
    }
    const QString &filenamePattern() const
    {
        return m_filenamePattern;
    }
    const QString &destinationDir() const
    {
        return m_destinationDir;
    }

    virtual void load() = 0;
    virtual void save() = 0;

protected:
    QStringList m_selectedStreams;
    QString m_firefoxProfile;
    QString m_filenamePattern;
    QString m_destinationDir;
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

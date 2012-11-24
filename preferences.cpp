#include "preferences.h"
#include <QDir>

Preferences::Preferences()
{
    load();
}

void Preferences::load()
{
    QString defaultWorkingPath(QDir::homePath().append(QDir::separator()).append("arteFetcher"));

    m_selectedStreams = settings.value("selectedStreams").toStringList();
    m_firefoxProfile = settings.value("firefoxProfile").toString();
    // TODO "[]" are forbidden in fat32
    m_filenamePattern = settings.value("filenamePattern", "[%language %quality] %title.flv").toString();
    m_destinationDir = settings.value("destinationDir", defaultWorkingPath).toString();
}

void Preferences::save()
{
    settings.setValue("selectedStreams", m_selectedStreams);
    settings.setValue("firefoxProfile", m_firefoxProfile);
    settings.setValue("filenamePattern", m_filenamePattern);
    settings.setValue("destinationDir", m_destinationDir);
    settings.sync();
}

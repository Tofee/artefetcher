#include "preferences.h"
#include <QDir>
#include <filmdelegate.h>
Preferences::Preferences()
{
    load();
}

void Preferences::load()
{
    QString defaultWorkingPath(QDir::homePath().append(QDir::separator()).append("arteFetcher"));

    m_selectedLanguage = settings.value("stream_language", FilmDelegate::listLanguages().first()).toString();
    m_selectedQuality = settings.value("stream_quality", FilmDelegate::listQualities().first()).toString();
    // TODO "[]" are forbidden in fat32
    m_filenamePattern = settings.value("filename_pattern", "[%language %quality] %title").toString();
    m_destinationDir = settings.value("destination_directory", defaultWorkingPath).toString();
    m_pendingDownloads = QSet<QString>::fromList(settings.value("pending_downloads", QStringList()).toStringList());
}

void Preferences::save()
{
    settings.setValue("stream_language", m_selectedLanguage);
    settings.setValue("stream_quality", m_selectedQuality);
    settings.setValue("filename_pattern", m_filenamePattern);
    settings.setValue("destination_directory", m_destinationDir);
    settings.setValue("pending_downloads", QStringList(m_pendingDownloads.toList()));
    settings.sync();
}

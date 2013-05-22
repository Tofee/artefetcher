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

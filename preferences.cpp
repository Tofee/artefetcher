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

#define DEF_OPT_STR_LANGUAGE     "stream_language"
#define DEF_OPT_STR_QUALITY      "stream_quality"
#define DEF_OPT_FILENAME_PATTERN "filename_pattern"
#define DEF_OPT_DST_DIR          "destination_directory"
#define DEF_OPT_PENDING_DOWNLADS "pending_downloads"
#define DEF_OPT_PREF_WINDOW_SIZE "preferred_window_size"
#define DEF_OPT_DEDICATED_DIR_SERIES "dedicated_dir_series"

Preferences::Preferences()
{
    load();
}

void Preferences::load()
{
    QString defaultWorkingPath(QDir::homePath().append(QDir::separator()).append("arteFetcher"));

    m_selectedLanguage = settings.value(DEF_OPT_STR_LANGUAGE, FilmDelegate::listLanguages().first()).toString();
    m_selectedQuality = settings.value(DEF_OPT_STR_QUALITY, FilmDelegate::listQualities().first()).toString();
    // TODO "[]" are forbidden in fat32
    m_filenamePattern = settings.value(DEF_OPT_FILENAME_PATTERN, "[%language %quality] %title").toString();
    m_destinationDir = settings.value(DEF_OPT_DST_DIR, defaultWorkingPath).toString();
    m_pendingDownloads = QSet<QString>::fromList(settings.value(DEF_OPT_PENDING_DOWNLADS, QStringList()).toStringList());
    m_preferredWindowSize = settings.value(DEF_OPT_PREF_WINDOW_SIZE, QSize(960,600)).toSize();
    m_dedicatedDirectoryForSeries = settings.value(DEF_OPT_DEDICATED_DIR_SERIES, true).toBool();
}

void Preferences::save()
{
    settings.setValue(DEF_OPT_STR_LANGUAGE, m_selectedLanguage);
    settings.setValue(DEF_OPT_STR_QUALITY, m_selectedQuality);
    settings.setValue(DEF_OPT_FILENAME_PATTERN, m_filenamePattern);
    settings.setValue(DEF_OPT_DST_DIR, m_destinationDir);
    settings.setValue(DEF_OPT_PENDING_DOWNLADS, QStringList(m_pendingDownloads.toList()));
    settings.setValue(DEF_OPT_PREF_WINDOW_SIZE, m_preferredWindowSize);
    settings.setValue(DEF_OPT_DEDICATED_DIR_SERIES, m_dedicatedDirectoryForSeries);
    settings.sync();
}

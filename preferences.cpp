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
#include <QMessageBox>
#include <filmdelegate.h>

#define DEF_OPT_STR_LANGUAGE          "stream_language"
#define DEF_OPT_STR_QUALITY           "stream_quality"
#define DEF_OPT_FILENAME_PATTERN      "filename_pattern"
#define DEF_OPT_FAVORITE_STREAM_TYPE  "favorite_stream_type"
#define DEF_OPT_DST_DIR               "destination_directory"
#define DEF_OPT_PENDING_DOWNLADS      "pending_downloads"
#define DEF_OPT_PREF_WINDOW_SIZE      "preferred_window_size"
#define DEF_OPT_DEDICATED_DIR_SERIES  "dedicated_dir_series"
#define DEF_OPT_SAVE_IMAGE_PREVIEW    "save_image_preview"
#define DEF_OPT_SAVE_META_INFO        "save_meta_info"
#define DEF_OPT_RESULT_COUNT_P_PAGE   "result_count_per_page"
#define DEF_OPT_PROXY_HTTP_ENABLED    "proxy_http_enabled"
#define DEF_OPT_PROXY_HTTP_URL        "proxy_http_url"
#define DEF_OPT_PROXY_HTTP_PORT       "proxy_http_port"
#define DEF_LAST_VERSION_USED         "last_version_used"

#define RESULT_PER_PAGE 10

Preferences *Preferences::_singleton = NULL;

Preferences::Preferences()
{
    load();
}

QStringList listVideoStreamTypes() {
    // TODO that could be better
    QStringList list;
    list.append("VOSTF");
    list.append("VF");
    list.append("Version allemande");
    list.append("ST sourds/mal");
    list.append("VOF");

    list.append("Dt. Version");
    list.append("Frz. Version");
    list.append(QString::fromUtf8("Hörfilm"));
    list.append("OmU");
    list.append(QString::fromUtf8("UT Hörgeschädigte"));
    return list;
}

void Preferences::load()
{
    QString defaultWorkingPath(QDir::homePath().append(QDir::separator()).append("arteFetcher"));

    m_applicationLanguage = settings.value(DEF_OPT_STR_LANGUAGE, StreamType::listLanguages().first()).toString();
    m_selectedQuality = settings.value(DEF_OPT_STR_QUALITY, StreamType::listQualities().first()).toString();
    m_filenamePattern = settings.value(DEF_OPT_FILENAME_PATTERN, "[%language %quality] %title").toString();
    m_favoriteStreamTypes = settings.value(DEF_OPT_FAVORITE_STREAM_TYPE, listVideoStreamTypes()).toStringList();
    // If a favorite is missing, we have to add it at the end of the list
    foreach (QString favorite, listVideoStreamTypes()) {
        if (!m_favoriteStreamTypes.contains(favorite)) {
            m_favoriteStreamTypes << favorite;
        }
    }
    m_destinationDir = settings.value(DEF_OPT_DST_DIR, defaultWorkingPath).toString();
    m_pendingDownloads = settings.value(DEF_OPT_PENDING_DOWNLADS, QStringList()).toStringList();
    m_preferredWindowSize = settings.value(DEF_OPT_PREF_WINDOW_SIZE, QSize(960,600)).toSize();
    m_dedicatedDirectoryForSeries = settings.value(DEF_OPT_DEDICATED_DIR_SERIES, true).toBool();
    m_saveImagePreview = settings.value(DEF_OPT_SAVE_IMAGE_PREVIEW, true).toBool();
    m_saveMetaInInfoFile = settings.value(DEF_OPT_SAVE_META_INFO, true).toBool();
    m_resultCountPerPage = settings.value(DEF_OPT_RESULT_COUNT_P_PAGE, RESULT_PER_PAGE).toInt();

    m_proxyEnabled = settings.value(DEF_OPT_PROXY_HTTP_ENABLED, false).toBool();
    m_proxyHttpUrl = settings.value(DEF_OPT_PROXY_HTTP_URL).toString();
    m_proxyHttpPort = settings.value(DEF_OPT_PROXY_HTTP_PORT, 3128).toInt();

    if (settings.value(DEF_LAST_VERSION_USED, "0.5.1").toString() == "0.5.1"){
        // TODO migration des langues pourries dans les préférences
//http://stackoverflow.com/questions/6832596/how-to-compare-software-version-number-using-js-only-number
//        const QString newFileNamePattern = "[%language] %title";
//        QMessageBox::information(NULL, QObject::tr("New version"),
//                                 QObject::tr("Your filename pattern was: %0.\nQuality is no longer supported.\nThus the pattern has been replaced with %1.\nYou can change it in the preference dialog.")
//                                 .arg(m_filenamePattern)
//                                 .arg(newFileNamePattern));
//        m_filenamePattern = newFileNamePattern;
        // Note : previous pending downloads will start from the beginning rather than continuing.
    }
}

void Preferences::save()
{
    settings.setValue(DEF_OPT_STR_LANGUAGE, m_applicationLanguage);
    settings.setValue(DEF_OPT_STR_QUALITY, m_selectedQuality);
    settings.setValue(DEF_OPT_FILENAME_PATTERN, m_filenamePattern);
    settings.setValue(DEF_OPT_DST_DIR, m_destinationDir);
    settings.setValue(DEF_OPT_PENDING_DOWNLADS, QStringList(m_pendingDownloads));
    settings.setValue(DEF_OPT_PREF_WINDOW_SIZE, m_preferredWindowSize);
    settings.setValue(DEF_OPT_DEDICATED_DIR_SERIES, m_dedicatedDirectoryForSeries);
    settings.setValue(DEF_OPT_SAVE_IMAGE_PREVIEW, m_saveImagePreview);
    settings.setValue(DEF_OPT_SAVE_META_INFO, m_saveMetaInInfoFile);
    settings.setValue(DEF_OPT_RESULT_COUNT_P_PAGE, m_resultCountPerPage);
    settings.setValue(DEF_OPT_FAVORITE_STREAM_TYPE, m_favoriteStreamTypes);
    settings.setValue(DEF_OPT_PROXY_HTTP_ENABLED, m_proxyEnabled);
    settings.setValue(DEF_OPT_PROXY_HTTP_URL, m_proxyHttpUrl);
    settings.setValue(DEF_OPT_PROXY_HTTP_PORT, m_proxyHttpPort);
    settings.setValue(DEF_LAST_VERSION_USED, QApplication::applicationVersion());
    settings.sync();
}

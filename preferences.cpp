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
#define DEF_OPT_REGISTERING_AGREEMENT "registering_agreement"
#define DEF_LAST_VERSION_USED         "last_version_used"
#define DEF_START_APP_COUNT           "start_app_count"
#define DEF_FIRST_REGISTRATION_DONE   "first_registration"
#define DEF_SECOND_REGISTRATION_DONE  "second_registration"
#define DEF_FAVORITE_CATALOGS         "favorite_catalogs"
#define DEF_CATALOG_AT_STARTUP        "startup_catalog"

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

int compareVersions(QString v1, QString v2){
    QRegExp regexp("[\\-.]");
    QStringList v1List = v1.split(regexp);
    QStringList v2List = v2.split(regexp);
    for(int i = 0; i < std::max(v1List.size(), v2List.size()); ++i){
        bool ok1, ok2;
        if (v1List.size() == i) return -1;
        if (v2List.size() == i) return 1;
        int v1Item = v1List.at(i).toInt(&ok1);
        int v2Item = v2List.at(i).toInt(&ok2);
        if (ok1 && !ok2) return 1;
        if (!ok1 && ok2) return -1;
        if (!ok1 && !ok2) continue;// String are not compared.
        // Now v1Item and v2Item are both integer, comparison is easy
        if ((v2Item - v1Item) != 0) return v1Item - v2Item;
    }
    return 0;
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
    m_pendingDownloads = settings.value(DEF_OPT_PENDING_DOWNLADS).toList();
    m_preferredWindowSize = settings.value(DEF_OPT_PREF_WINDOW_SIZE, QSize(960,600)).toSize();
    m_dedicatedDirectoryForSeries = settings.value(DEF_OPT_DEDICATED_DIR_SERIES, true).toBool();
    m_saveImagePreview = settings.value(DEF_OPT_SAVE_IMAGE_PREVIEW, true).toBool();
    m_saveMetaInInfoFile = settings.value(DEF_OPT_SAVE_META_INFO, true).toBool();
    m_resultCountPerPage = settings.value(DEF_OPT_RESULT_COUNT_P_PAGE, RESULT_PER_PAGE).toInt();

    m_proxyEnabled = settings.value(DEF_OPT_PROXY_HTTP_ENABLED, false).toBool();
    m_proxyHttpUrl = settings.value(DEF_OPT_PROXY_HTTP_URL).toString();
    m_proxyHttpPort = settings.value(DEF_OPT_PROXY_HTTP_PORT, 3128).toInt();
    m_favoriteCatalogs = settings.value(DEF_FAVORITE_CATALOGS, QStringList() /*when empty, it means no filter*/).toStringList();
    m_catalogAtStartup = settings.value(DEF_CATALOG_AT_STARTUP, QString() /*when empty, it means first catalog*/).toString();

    QString lastVersionUsed = settings.value(DEF_LAST_VERSION_USED, "0.5.4").toString();

    // Migration
    if (compareVersions(lastVersionUsed, "0.5.4") <= 0){
        // Migrate preferences because of a bug with 0.5.4 version :
        // the favorite stream types countains bad encoded characters
        m_favoriteStreamTypes = listVideoStreamTypes();
    }

    // All about registration
    if (settings.value(DEF_OPT_REGISTERING_AGREEMENT).isNull()){
        m_registrationAgreement = (QMessageBox::question(NULL, QObject::tr("Registration request"),
                              registrationAgreementText().append(QObject::tr("<br/><br/>You can change this setting anytime in preferences.<br/><br/>Do you agree?")),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes);
    } else {
        m_registrationAgreement = settings.value(DEF_OPT_REGISTERING_AGREEMENT).toBool();
    }
    m_startAppCount = settings.value(DEF_START_APP_COUNT, 0).toInt();
    m_startAppCount++;
    m_firstRegistration = settings.value(DEF_FIRST_REGISTRATION_DONE, "false").toBool();
    m_secondRegistration = settings.value(DEF_SECOND_REGISTRATION_DONE, "false").toBool();

    if (compareVersions(QApplication::applicationVersion(), lastVersionUsed) > 0) // 0.6.0 to 0.6.1 is minor version
    {
        m_firstRegistration = false;
//        m_secondRegistration = false;
//        m_startAppCount = 1;
    }
}

void Preferences::save()
{
    settings.setValue(DEF_OPT_STR_LANGUAGE, m_applicationLanguage);
    settings.setValue(DEF_OPT_STR_QUALITY, m_selectedQuality);
    settings.setValue(DEF_OPT_FILENAME_PATTERN, m_filenamePattern);
    settings.setValue(DEF_OPT_DST_DIR, m_destinationDir);
    settings.setValue(DEF_OPT_PENDING_DOWNLADS, m_pendingDownloads);
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
    settings.setValue(DEF_OPT_REGISTERING_AGREEMENT, m_registrationAgreement);
    settings.setValue(DEF_START_APP_COUNT, m_startAppCount);
    settings.setValue(DEF_FIRST_REGISTRATION_DONE, m_firstRegistration);
    settings.setValue(DEF_SECOND_REGISTRATION_DONE, m_secondRegistration);
    settings.setValue(DEF_FAVORITE_CATALOGS, m_favoriteCatalogs);
    settings.setValue(DEF_CATALOG_AT_STARTUP, m_catalogAtStartup);
    settings.sync();
}

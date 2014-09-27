#include "testpreferences.h"

#include <QObject>

#include "preferences.h"

void TestPreferences::testCompareVersions() {
    Q_ASSERT(compareVersions("0.6.2", "0.5.4") > 0);
    Q_ASSERT(compareVersions("0.6.2", "0.6.2") == 0);
    Q_ASSERT(compareVersions("0.6.2", "0.6.9") < 0);
    Q_ASSERT(compareVersions("0.6.2", "0.5") > 0);
    Q_ASSERT(compareVersions("0.5", "0.5.4") < 0);
    Q_ASSERT(compareVersions("0.5.1-SNAPSHOT", "0.5.1-SCHTROUMPF") == 0);
    Q_ASSERT(compareVersions("0.5.1", "0.5.2-SCHTROUMPF") < 0);
    Q_ASSERT(compareVersions("0.5.1-SNAPSHOT", "0.5.2") < 0);
}


#ifndef TESTPREFERENCES_H
#define TESTPREFERENCES_H
#include <QObject>


class TestPreferences: public QObject {
  Q_OBJECT
public slots:
    void testCompareVersions();
};


#endif // TESTPREFERENCES_H

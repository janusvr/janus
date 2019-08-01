#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include <QObject>
#include <QtCore>
#include <QtNetwork>

#include "mathutil.h"

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT

public:
    explicit CookieJar(QObject *parent = 0);
    ~CookieJar();

    static void Initialize();

    void ReadFromDisk();
    void SaveToDisk();

    void PrintAllCookies();

    static int max_cookies_per_domain;
    static int max_cookie_value_size;
    static QPointer <CookieJar> cookie_jar;
};

#endif // COOKIEJAR_H

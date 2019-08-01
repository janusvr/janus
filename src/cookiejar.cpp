#include "cookiejar.h"

QPointer <CookieJar> CookieJar::cookie_jar;
int CookieJar::max_cookies_per_domain = 100;
int CookieJar::max_cookie_value_size = 1024;

CookieJar::CookieJar(QObject *parent) :
    QNetworkCookieJar(parent)
{
    ReadFromDisk();
}

CookieJar::~CookieJar()
{    
}

void CookieJar::Initialize()
{
    CookieJar::cookie_jar = new CookieJar();
}

void CookieJar::ReadFromDisk()
{
    QString filename = MathUtil::GetAppDataPath() + "cookies.json";
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "CookieJar::ReadFromDisk(): File" << filename << "can't be read from";
        return;
    }

    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);

    file.close();

    if (error.error != QJsonParseError::NoError) {
        qDebug() << "CookieJar::ReadFromDisk(): File" << filename << "contains JSON error" << error.error << error.errorString();
        return;
    }

    QJsonObject root = document.object();
    QJsonObject::const_iterator it;
    for (it=root.begin(); it != root.end(); ++it) {

        QJsonObject pathCookieHash = it.value().toObject();
        QList<QNetworkCookie> cookiesForPath;

        QString domainStr = it.key();
        if (!domainStr.isEmpty() && domainStr.at(0) == '.') {
            domainStr.remove(0,1);
        }
        domainStr = QString("http://") + domainStr;

        QJsonObject::const_iterator it2;
        for (it2=pathCookieHash.begin(); it2 != pathCookieHash.end(); ++it2) {
            QNetworkCookie cookie(it2.key().toLatin1(), it2.value().toString().toLatin1());
            cookie.setDomain(it.key());
            cookie.setPath("/");
            cookiesForPath << cookie;
        }

        //const bool result = setCookiesFromUrl(cookiesForPath, domainStr);        
        setCookiesFromUrl(cookiesForPath, domainStr);
//            qDebug() << "";
//            qDebug() << "CookieJar::ReadFromDisk(): result" << result << domainStr << cookie.name() << cookie.value();
//            qDebug() << "";

    }
//    qDebug() << "CookieJar::ReadFromDisk(): Read" << allCookies().size() << "cookies.";
}

void CookieJar::SaveToDisk()
{
//    qDebug() << "CookieJar::SaveToDisk(): Saving" << allCookies().size() << "cookies...";
    QJsonObject saveData;
    QList <QNetworkCookie> cookies = allCookies();
    foreach (QNetworkCookie cookie, cookies) {
        QJsonObject cookiesForDomain;
        if (saveData.contains(cookie.domain())) {
            cookiesForDomain = saveData[cookie.domain()].toObject();
        }
        cookiesForDomain.insert(QString(cookie.name()), QString(cookie.value()));
        saveData.insert(cookie.domain(), cookiesForDomain);
    }

    QJsonDocument document;
    document.setObject(saveData);

    const QString filename = MathUtil::GetAppDataPath() + "cookies.json";
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "CookieJar::SaveToDisk(): File" << filename << "can't be appended";
        return;
    }

    file.write(document.toJson());
    file.close();
}

void CookieJar::PrintAllCookies()
{
    qDebug() << "CookieJar::PrintAllCookies()" << allCookies().size();
    qDebug() << allCookies();
    foreach (QNetworkCookie cookie, allCookies()) {
        qDebug() << "cookie's domain" << cookie.domain();
    }
}

#include "webasset.h"

QHash <QString, AuthData> WebAsset::auth_list;

QPointer <QNetworkAccessManager> WebAsset::manager;
QPointer <QNetworkDiskCache> WebAsset::cache;
bool WebAsset::use_cache = true;

WebAsset::WebAsset() :
    progress(0.0f),
    started(false),
    loaded(false),
    processing(false),
    processed(false),
    finished(false),
    status_code(0),
    error_logged(false),
    redirected(false)
{
}

WebAsset::~WebAsset()
{
}

void WebAsset::Initialize()
{
    if (manager.isNull()) {
        manager = new QNetworkAccessManager();
    }

    if (cache.isNull()) {
        cache = new QNetworkDiskCache();
        cache->setCacheDirectory(MathUtil::GetCachePath());
        qint64 cache_size = 1000000000;
        cache->setMaximumCacheSize(cache_size);
        manager->setCache(cache);
    }

    manager->setCookieJar(CookieJar::cookie_jar);
    if (CookieJar::cookie_jar) {
        CookieJar::cookie_jar->setParent(NULL);
    }
}

void WebAsset::SetUseCache(const bool b)
{
    use_cache = b;
}

bool WebAsset::GetUseCache()
{
    return use_cache;
}

QPointer <QNetworkAccessManager> WebAsset::GetNetworkManager()
{
    return manager;
}

void WebAsset::InitializeState()
{
    redirected = false;
    started = true;
    loaded = false;
    processing = false;
    processed = false;
    error_logged = false;
    status_code = 0;
    error_str.clear();
    ba.clear();
}

void WebAsset::Load(const QUrl & u)
{
//    qDebug() << "WebAsset::Load" << u;    
    reply = DoHTTPGet(u);
    reply->setParent(NULL);

    if (reply) {
        connect(reply, SIGNAL(readyRead()), this, SLOT(ReadyRead()));
        connect(reply, SIGNAL(finished()), this, SLOT(ProcessThread()));
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(slotSslErrors(QList<QSslError>)));
    }
}

void WebAsset::Unload()
{
    if (reply) {
        disconnect(reply, 0, 0, 0);
    }

    started = false;
    loaded = false;
    processing = false;
    processed = false;
    finished = false;    
    error_logged = false;
    error_str.clear();
    status_code = 0;

    ba.clear();
}

void WebAsset::SetStarted(const bool b)
{
    started = b;
}

bool WebAsset::GetStarted() const
{
    return started;
}

bool WebAsset::GetRunning() const
{
    return (reply ? !reply->isRunning() : false);
}

void WebAsset::SetLoaded(const bool b)
{
    loaded = b;
}

bool WebAsset::GetLoaded() const
{
    return loaded;
}

void WebAsset::SetProcessing(const bool b)
{
    processing = b;
}

bool WebAsset::GetProcessing() const
{
    return processing;
}

void WebAsset::SetProcessed(const bool b)
{
    processed = b;
}

bool WebAsset::GetProcessed() const
{
    return processed;
}

void WebAsset::SetFinished(const bool b)
{
    finished = b;
}

bool WebAsset::GetFinished() const
{
    return finished;
}

void WebAsset::SetErrorLogged(const bool b)
{
    error_logged = b;
}

bool WebAsset::GetErrorLogged() const
{
    return error_logged;
}

QString WebAsset::GetErrorString() const
{
    return error_str;
}

void WebAsset::SetData(const QByteArray & b)
{
    ba = b;
}

const QByteArray & WebAsset::GetData() const
{
    return ba;
}

void WebAsset::ClearData()
{
    ba.clear();
}

float WebAsset::GetProgress() const
{    
    return (GetError() || finished) ? 1.0f : progress;
}

bool WebAsset::GetError() const
{
    return (status_code >= 400) || (status_code == -1);
}

void WebAsset::SetStatusCode(const int s)
{
    status_code = s;
}

int WebAsset::GetStatusCode() const
{
    return status_code;
}

void WebAsset::SetURL(const QUrl u)
{
//    qDebug() << "WebAsset::SetURL" << u;
    url = u;
}

QUrl WebAsset::GetURL() const
{
    return url;
}

void WebAsset::ProcessThread()
{
//    qDebug() << "WebAsset::ProcessThread()";
    if (reply && status_code != -1) {
        status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QUrl redirect_url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (redirect_url.isEmpty()) {
            if (reply->error() == QNetworkReply::NoError && reply->isOpen()) {
                ba += reply->readAll();
                // Detect .gz compression based on magic number in first 2 bytes
                const bool gzipped = (ba.length() >= 2 && (unsigned char)(ba.at(0)) == 0x1f && (unsigned char)(ba.at(1)) == 0x8b);
                if (gzipped) {
                    ba = MathUtil::Decompress(ba); //NOTE: if not run in thread, this function can block
                    // Remove .gz extension from URL
                    if (url.fileName().right(3).toLower() == ".gz") {
                        QString url_string = url.toString();
                        url.setUrl(url_string.left(url_string.lastIndexOf(".gz")));
                    }
                }
            }
        }
        else {
            redirect_url.setFragment(url.fragment());
            redirect_url = url.resolved(redirect_url);
//            qDebug() << "WebAsset::ProcessThread() - REDIRECTION detected" << redirect_url << status_code << url.fragment();
            reply->close();
            Load(redirect_url);
            redirected = true;
            return;
        }
        reply->close();
    }

    loaded = true;
    progress = 1.0f;
    disconnect(reply, 0, 0, 0);    
    emit Finished();
}

void WebAsset::downloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
    if (bytesTotal > 0) {
        progress = float(double(bytesReceived)/double(bytesTotal));
    }
    else {
        progress = 0.0f;
    }
}

void WebAsset::slotError(QNetworkReply::NetworkError)
{
//    qDebug() << "WebAsset::slotError()" << url.toString();
    error_str = QString("Error: loading \"") + url.toString() + "\"";
    MathUtil::ErrorLog(error_str);
    loaded = true;    
    progress = 1.0f;
    status_code = -1;
}

void WebAsset::slotSslErrors(QList <QSslError>)
{
//    qDebug() << "WebAsset::slotSslError()" << url.toString();
    error_str = QString("Error: loading \"") + url.toString() + "\"";
    MathUtil::ErrorLog(error_str);
    loaded = true;    
    progress = 1.0f;
    status_code = -1;
}

QPointer <QNetworkReply> WebAsset::DoHTTPGet(const QUrl & u)
{
    if (manager) {
        InitializeState();
        url = u;

        const QString s = url.fileName().toLower();
        const bool suppress_cache = (s.right(5) == ".html" || s.right(4) == ".htm" || s.right(4) == ".php" || s.right(3) == ".js" || s.right(4) == ".txt" || s.right(4) == ".mtl");

        QNetworkRequest request;
        request.setUrl(url);
        request.setRawHeader("User-Agent", "FireBox 1.0");                

        //set the user/pass for authentication, if we have it
        const QString host = url.host();
        if (auth_list.contains(host)) {
            //qDebug() << "WebAsset::DoNetworkRequest() - host" << host;
            request.setRawHeader("Authorization", "Basic " + (auth_list[host].user+":"+auth_list[host].password).toLatin1().toBase64());
        }

        //59.9 - Remove referer header, this was redundant (always the URL being requested)
//        if (s.length() > 0) {
//            request.setRawHeader("Referer", s.toLatin1());
//        }

        request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
        if (!use_cache || suppress_cache) {
            request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
        }
        else {
            request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
        }

        return manager->get(request);
    }

    return QPointer <QNetworkReply> ();
}

void WebAsset::DoHTTPPost(const QUrl & u, const QByteArray & b)
{
//    qDebug() << "WebAsset::DoHTTPPost() sending: " << b << b.size();
    if (manager) {
        InitializeState();
        url = u;

        // here connect signals etc.
        QNetworkRequest request(url);
        request.setUrl(url);
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

        reply = manager->post(request, b);

        if (reply) {
            connect(reply, SIGNAL(finished()), this, SLOT(ProcessThread()));
            connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
            connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(slotSslErrors(QList<QSslError>)));
        }
    }
}

void WebAsset::DoHTTPPut(const QUrl & u, const QByteArray & b)
{
//    qDebug() << "WebAsset::DoHTTPPut() sending: " << b << b.size();
    if (manager) {
        InitializeState();
        url = u;

        // here connect signals etc.
        QNetworkRequest request(url);
        request.setUrl(url);
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

        reply = manager->put(request, b);

        if (reply) {
            connect(reply, SIGNAL(finished()), this, SLOT(ProcessThread()));
            connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
            connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(slotSslErrors(QList<QSslError>)));
        }
    }
}

void WebAsset::DoHTTPDelete(const QUrl & u)
{
//    qDebug() << "WebAsset::DoHTTPDelete() sending: " << b << b.size();
    if (manager) {
        InitializeState();
        url = u;

        // here connect signals etc.
        QNetworkRequest request(url);
        request.setUrl(url);
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

        reply = manager->deleteResource(request);

        if (reply) {
            connect(reply, SIGNAL(finished()), this, SLOT(ProcessThread()));
            connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
            connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(slotSslErrors(QList<QSslError>)));
        }
    }
}

void WebAsset::DoHTTPPost(const QUrl & u, const QHash <QString, QString> & data)
{
    if (manager) {
        InitializeState();
        url = u;

        multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

        QHash <QString, QString>::const_iterator i;
        for (i=data.begin(); i!=data.end(); ++i) {
            QHttpPart keyPart;
            keyPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + i.key() + "\""));
            keyPart.setBody(i.value().toUtf8());

            multiPart->append(keyPart);
        }

        // here connect signals etc.
        QNetworkRequest request(url);
        request.setUrl(url);

        reply = manager->post(request, multiPart);

        if (reply) {
            multiPart->setParent(reply); // delete the multiPart with the reply

            connect(reply, SIGNAL(finished()), this, SLOT(ProcessThread()));
            connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
            connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
            connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(slotSslErrors(QList<QSslError>)));
        }
    }
}

void WebAsset::LoadAuthData()
{
    QString filename = MathUtil::GetAppDataPath() + "authentication.txt";
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream ifs(&file);
        while (!ifs.atEnd()) {
            QStringList eachline = ifs.readLine().simplified().split(" ");
            if (eachline.length() == 3) { //not a comment or blank line
                AuthData d;
                d.user = eachline[1];
                d.password = eachline[2];
                WebAsset::auth_list[eachline[0]] = d;
            }

        }
        file.close();
    }
    else {
        qDebug() << "Game::LoadAuthData()" << filename << "could not be found, creating";
        SaveAuthData();
    }
}

void WebAsset::SaveAuthData()
{
    QString filename = MathUtil::GetAppDataPath() + "authentication.txt";
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Game::SaveAuthData(): File " << filename << " can't be saved";
        return;
    }

    QTextStream ofs(&file);
    QHash <QString, AuthData>::iterator i;
    for (i=auth_list.begin(); i!=auth_list.end(); ++i) {
        ofs << i.key() << " " << i.value().user << " " << i.value().password << "\n";
    }
    file.close();
    qDebug() << "Game::SaveAuthData() - Saved" << auth_list.size() << "user credentials.";
}

void WebAsset::ReadyRead()
{
    if (reply) {
        ba += reply->readAll();
    }
//    qDebug() << "WebAsset::ReadyRead()" << ba.size() << url;
}

bool WebAsset::GetRedirected() const
{
    return redirected;
}

#ifndef WEBASSET_H
#define WEBASSET_H

#include <QtNetwork>

#include "mathutil.h"
#include "cookiejar.h"
#include "rendererinterface.h"

struct AuthData {
    QString user;
    QString password;
};

class WebAsset : public QObject
{
    Q_OBJECT

public:

    WebAsset();
    ~WebAsset();   

    void Load(const QUrl & u);
    void Unload();

    bool GetRedirected() const;

    bool GetRunning() const;

    void SetStarted(const bool b);
    bool GetStarted() const;

    void SetLoaded(const bool b);
    bool GetLoaded() const;

    void SetProcessing(const bool b);
    bool GetProcessing() const;

    void SetProcessed(const bool b);
    bool GetProcessed() const;

    void SetFinished(const bool b);
    bool GetFinished() const;

    void SetErrorLogged(const bool b);
    bool GetErrorLogged() const;

    void SetData(const QByteArray & b);
    const QByteArray & GetData() const;
    void ClearData();

    float GetProgress() const;

    bool GetError() const;
    void SetStatusCode(const int s);
    int GetStatusCode() const;
    QString GetErrorString() const;

    void SetURL(const QUrl u);
    QUrl GetURL() const;

    QPointer <QNetworkReply> DoHTTPGet(const QUrl & u);
    void DoHTTPPost(const QUrl & u, const QByteArray & b);
    void DoHTTPPost(const QUrl & u, const QHash <QString, QString> & data);
    void DoHTTPPut(const QUrl & u, const QByteArray & b);
    void DoHTTPDelete(const QUrl & u);

    static void Initialize();
    static void SetUseCache(const bool b);
    static bool GetUseCache();
    static QPointer <QNetworkAccessManager> GetNetworkManager();

    static void LoadAuthData();
    static void SaveAuthData();   

    static QHash <QString, AuthData> auth_list;

signals:

    void Finished(); //needed for JS XMLHttpRequest

public slots:

    void ReadyRead();
    void ProcessThread();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void slotError(QNetworkReply::NetworkError e);
    void slotSslErrors(QList <QSslError> e);   

private: 

    void InitializeState();

    QUrl url;
    QPointer <QNetworkReply> reply;    
    QPointer <QHttpMultiPart> multiPart;
    float progress;
    QByteArray ba;
    bool started;
    bool loaded;
    bool processing;
    bool processed;
    bool finished;
    int status_code;
    bool error_logged;
    QString error_str;
    bool redirected;

    static QPointer <QNetworkAccessManager> manager;
    static QPointer <QNetworkDiskCache> cache;
    static bool use_cache;

};

#endif // WEBASSET_H

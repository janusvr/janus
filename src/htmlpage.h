#ifndef HTMLPAGE_H
#define HTMLPAGE_H

#include <QtNetwork>
#include <QtGui>
#include <QtXml>
#include <QRectF>

#include "mathutil.h"
#include "cookiejar.h"
#include "webasset.h"
#include "bookmarkmanager.h"

struct ImgurData {
    QString img_url;
};

struct RedditData {
    QString link_url;
    QString img_url;
    QString text_str;
    QString comment_str;
    QString rank_str;
    QString time_str;
    QString score_str;
    QString user_str;
};

class HTMLPage : public QObject
{
    Q_OBJECT
public:

    HTMLPage();
    virtual ~HTMLPage();

    void SetURL(const QUrl & u);
    QUrl GetURL() const;

    void SetCode(const QString & code);
    void Request(const QString & referer_url);

    void Update();

    WebAsset & GetWebAsset();

    //rooms are built from the data returned by these:
    QString GetTitle() const;

    const QList <RedditData> & RedditThings() const;
    const QList <ImgurData> & ImgurThings() const;
    const QList <QString> & NavLinks() const;

    const QVariantMap & GetData() const;
    const QVariantMap GetRoomData() const;

    void Clear();

    void ConstructData();

    bool FoundFireBoxContent() const;
    bool FoundRedditContent() const;    
    bool FoundRedditCommentContent() const;
    bool FoundImgurContent() const;
    bool FoundVimeoContent() const;
    bool FoundYoutubeContent() const;
    bool FoundFlickrContent() const;
    bool FoundStandardHTML() const;
    bool FoundSingleImageContent() const;
    bool FoundGeometryContent() const;
    bool FoundVideoContent() const;
    bool FoundError() const;
    bool FoundDirectoryListing() const;

    float GetProgress() const;   
    int GetErrorCode() const;

    void TraverseXmlNode(const QDomNode & node, const int depth, QVariantMap & parent_data);    

    //these do the work of parsing the different content/sites
    void ReadRegularHTMLContent(const QString & html_str);
    void ReadXMLContent(const QString & firebox_data);
    void ReadJSONContent(const QString & firebox_data);    
    void ReadRedditContent(const QString & reddit_data);
    void ReadRedditCommentContent(const QString & reddit_data);
    void ReadImgurContent(const QString & imgur_data);
    void ReadFlickrContent(const QString & imgur_data);
    void ReadBookmarksContent();
    void ReadWorkspacesContent();
    void ReadOldHomeArea();

    static bool HTMLExtract(const QString & start_tag, const QString & end_tag, const bool include_tags, const QString & html, QString & return_str);
    static void HTMLExtractAll(const QString & start_tag, const QString & end_tag, const bool include_tags, const QString & html, QList <QString> & strings);

private:

    void ClearData();

    void LoadFile();

    QString FormCompleteURL(const QString & url_str);

    //handy utility methods
    QStringList GetStringsBetween(const QString & s1, const QString & s2, const QString & str);

    //website translators (convert site code into
    QString GfyGenerateRoomCode(const QString & htmlIn);
    QString GfyGenerateGfyViewingRoom(const QString title, const QString webmUrl, const int width, const int height);
    QString GfyGenerateGfyDefault();   

    WebAsset webasset;
    QByteArray ba; //55.5 - parsed inside thread

    QString content_type;    
    QString title;       

    QVariantMap data;

    QList <RedditData> reddit_things;
    QList <ImgurData> imgur_things;

    QList <QString> nav_links;

    QSet <QString> urls_known;

    QVariantMap fireboxroom;
    QVariantMap assets;
    QVariantList assetobjectlist;
    QVariantList assetimagelist;
    QVariantList assetghostlist;
    QVariantList assetrecordinglist;
    QVariantList assetshaderlist;
    QVariantList assetscriptlist;
    QVariantList assetsoundlist;
    QVariantList assetvideolist;
    QVariantList assetwebsurfacelist;

    QVariantMap room;
    QVariantList lightlist;
    QVariantList objectlist;
    QVariantList imagelist;
    QVariantList ghostlist;
    QVariantList soundlist;
    QVariantList videolist;
    QVariantList linklist;
    QVariantList particlelist;    
    QVariantList textlist;
    QVariantList paragraphlist;
};

#endif // HTMLPAGE_H

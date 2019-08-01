#include "bookmarkmanager.h"

QVariantList BookmarkManager::bookmarks;
QVariantList BookmarkManager::workspaces;

BookmarkManager::BookmarkManager()
{    
}

void BookmarkManager::AddWorkspace(const QString url, const QString title, const QString thumbnail)
{
    const QDir d(url);
    qDebug() << "BookmarkManager::AddWorkspace" << d.path();
    QVariantMap o;
    o["url"] = d.path();
    o["title"] = title;
    o["thumbnail"] = thumbnail;

    //59.4 - add only if URL is unique, otherwise just update title/thumbnail properties
    for (int i=0; i<workspaces.size(); ++i) {
        if (workspaces[i].toMap()["url"] == url) {
            workspaces[i] = o;
            return;
        }
    }

    workspaces.push_back(o);
}

void BookmarkManager::AddBookmark(const QString url, const QString title, const QString thumbnail)
{
    const QString translator_path = MathUtil::GetTranslatorPath();

    QJsonObject o;
    o["url"] = url;
    o["title"] = title;

    //special treatment for thumbnail
    if (thumbnail.isEmpty()) {
        QString thumb;

        if (url.contains("reddit.com")) {
            thumb = QString(translator_path + "reddit/self.png");
        }
        else if (url.contains("drashvr.com")) {
            thumb = QString(translator_path + "drash/drash.jpg");
        }
        else if (url.contains("imgur.com")) {
            thumb = QString(translator_path + "imgur/thumb.jpg");
        }
        else if (url.contains("flickr.com")) {
            thumb = QString(translator_path + "flickr/flickr.jpg");
        }
        else if (url.contains("youtube.com")) {
            thumb = QString(translator_path + "youtube/thumb.jpg");
        }
        else if (url.contains("youtube.com")) {
            thumb = QString(translator_path + "youtube/thumb.jpg");
        }
        else if (url.contains("vimeo.com")) {
            thumb = QString(translator_path + "vimeo/thumb.jpg");
        }
        else if (url.contains("usagii.net/wikiverse")) {
            thumb = QString(translator_path + "usagii/wikiverse.jpg");
        }
        else if (url.contains("techn0shaman.one")) {
            thumb = QString(translator_path + "default/techno.jpg");
        }
        else if (url.contains("frog.thevirtualarts.com")) {
            thumb = QString(translator_path + "default/frogger.jpg");
        }
        else if (url.contains("minervavr.com")) {
            thumb = QString(translator_path + "default/minerva.jpg");
        }
        else if (url.contains("geniusvr.com")) {
            thumb = QString(translator_path + "default/geniusvr.jpg");
        }
        else if (url.contains("spyduck.net")) {
            thumb = QString(translator_path + "default/spyduck.jpg");
        }
        else if (url.contains("www.dgp.toronto.edu/~karan/janusvr/maya2janus")) {
            thumb = QString(translator_path + "default/animation.jpg");
        }
        else if (url.contains("vesta.janusvr.com")) {
            thumb = QString(translator_path + "default/vesta.jpg");
        }
        else if (url.contains("vrsites.com")) {
            thumb = QString("http://vrsites.com/thumb.jpg");
        }
        else {
            thumb = QString("thumb.jpg");
        }

        o["thumbnail"] = QUrl(url).resolved(thumb).toString();
    }
    else {
        o["thumbnail"] = thumbnail;
    }

    bookmarks.push_back(o.toVariantMap());
}

QString BookmarkManager::CleanedURL(const QString s) const
{
    QString u = s.toLower();
    if (u.endsWith("/")){
        u.remove(u.length() - 1, 1);
    }
    if (u.startsWith("https://")){
        u.remove(0, 8);
    }
    if (u.startsWith("http://")){
        u.remove(0, 7);
    }
    if (u.startsWith("www.")){
        u.remove(0, 4);
    }
    return u;
}

void BookmarkManager::RemoveBookmark(const QString url)
{
    const QString u = CleanedURL(url);
    for (int i=0; i<bookmarks.size(); ++i) {
        const QVariantMap m = bookmarks[i].toMap();
        if (CleanedURL(m["url"].toString()) == u) {
            bookmarks.removeAt(i);
            return;
        }
    }
}

bool BookmarkManager::GetBookmarked(const QString url) const
{
    const QString u = CleanedURL(url);
    for (int i=0; i<bookmarks.size(); ++i) {
        const QVariantMap m = bookmarks[i].toMap();
        if (CleanedURL(m["url"].toString()) == u) {
            return true;
        }
    }
    return false;
}

QVariantMap BookmarkManager::GetBookmark(const QString url) const
{
    for (int i=0; i<bookmarks.size(); ++i) {
        const QVariantMap m = bookmarks[i].toMap();
        if (QString::compare(m["url"].toString(), url) == 0) {
            return m;
        }
    }
    return QVariantMap();
}

QVariantList BookmarkManager::GetBookmarks() const
{
    return bookmarks;
}

QVariantList BookmarkManager::GetWorkspaces() const
{
    return workspaces;
}

void BookmarkManager::LoadWorkspaces()
{
    workspaces.clear();

    const QString path = MathUtil::GetWorkspacePath();
    QDir d(path);
    d.setFilter(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
    QStringList l = d.entryList();
    for (int i=0; i<l.size(); ++i) {

        QDir d2(path+l[i]);
        d2.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);

        QStringList l2 = d2.entryList();
        for (int j=0; j<l2.size(); ++j) {
            if (l2[j].right(4).toLower() == ".htm" || l2[j].right(5).toLower() == ".html") {
                const QString filepath = path + l[i] + "/" + l2[j];

                QVariantMap m;
                m["url"] = filepath;
                m["title"] = "";
                m["thumbnail"] = "";

                workspaces.push_back(m);
            }
        }
    }
}

void BookmarkManager::LoadBookmarks()
{
    bookmarks.clear();

    const QString filename = MathUtil::GetAppDataPath() + "bookmarks.json";

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "BookmarkManager::LoadBookmarks() - File" << filename << "not found, generating.";
        const QString path = MathUtil::GetApplicationURL() + "assets/3dui/thumbs/";
        AddBookmark("https://vesta.janusvr.com", "VESTA - JanusVR User Community", path+"28.jpg");
        AddBookmark("https://www.reddit.com/r/janusVR/", "Reddit r/janusVR", path+"29.jpg");
        AddBookmark("https://www.youtube.com", "YouTube", path+"27.jpg");
        AddBookmark("https://www.janusvr.com/newlobby/index.html", "JanusVR Lobby", path+"30.jpg");
        AddBookmark("https://vesta.janusvr.com/sandbox", "Vesta Sandbox", path+"31.jpg");
        AddBookmark("https://janusvr.com/help", "JanusVR Help", path+"32.jpg");
        SaveBookmarks();
    }
    else {
        const QByteArray ba = file.readAll();
        file.close();

        bookmarks = QJsonDocument::fromJson(ba).toVariant().toList();
        qDebug() << "BookmarkManager::LoadBookmarks() - Loaded" << bookmarks.size() << "bookmarks.";
    }
}

void BookmarkManager::SaveBookmarks()
{
    const QString filename = MathUtil::GetAppDataPath() + "bookmarks.json";
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "BookmarkManager::SaveBookmarks() - File" << filename << " can't be saved.";
        return;
    }

    QTextStream ofs(&file);
    ofs << QJsonDocument::fromVariant(bookmarks).toJson();
    file.close();
    qDebug() << "BookmarkManager::SaveBookmarks() - Saved" << bookmarks.size() << "bookmarks.";
}

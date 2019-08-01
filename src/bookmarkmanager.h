#ifndef BOOKMARKMANAGER_H
#define BOOKMARKMANAGER_H

#include "mathutil.h"

class BookmarkManager : public QObject
{

public:

    BookmarkManager();

    //bookmarks
    void AddBookmark(const QString url, const QString title="", const QString thumbnail="");
    void RemoveBookmark(const QString url);
    bool GetBookmarked(const QString url) const;

    QVariantMap GetBookmark(const QString url) const;
    QVariantList GetBookmarks() const;

    void LoadBookmarks();
    void SaveBookmarks();

    //workspaces
    void AddWorkspace(const QString url, const QString title="", const QString thumbnail="");
    void LoadWorkspaces();
    QVariantList GetWorkspaces() const;

private:

    QString CleanedURL(const QString s) const;

    static QVariantList bookmarks;
    static QVariantList workspaces;
};

#endif // BOOKMARKMANAGER_H

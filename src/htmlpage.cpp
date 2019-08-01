#include "htmlpage.h"

HTMLPage::HTMLPage()
{
}

HTMLPage::~HTMLPage()
{    
}

void HTMLPage::SetURL(const QUrl & u)
{
    if (u.toString().left(22).contains("reddit.com") && !u.toString().left(22).contains("old.reddit.com")) {
        webasset.SetURL(QUrl(u.toString().replace("www.reddit.com", "reddit.com").replace("reddit.com", "old.reddit.com")));
    }
    else {
        webasset.SetURL(u);
    }
}

QUrl HTMLPage::GetURL() const
{
    return webasset.GetURL();
}

void HTMLPage::SetCode(const QString & code)
{
    ba = code.toLatin1();
    webasset.SetStarted(true);
    webasset.SetLoaded(true);
    webasset.SetProcessing(true);

    LoadFile();
}

void HTMLPage::Request(const QString & )
{          
    const QUrl url = webasset.GetURL();
    const QFileInfo fi(url.toString());
    const QString suffix = fi.suffix().toLower();
    const QFileInfo host_fi(url.toString().split("/")[0]);
    const QString host_suffix = host_fi.suffix().toLower();

    webasset.SetStarted(true);

//    qDebug() << "HTMLPage::Request" << url << url.toString();
    if (QString::compare(url.toString(), "bookmarks", Qt::CaseInsensitive) == 0) {
        //qDebug() << "HTMLPage::Request" << url.toString();
        webasset.SetURL(QUrl("bookmarks"));
        ReadBookmarksContent();
        webasset.SetLoaded(true);
        webasset.SetProcessing(true);
    }
    else if (QString::compare(url.toString(), "workspaces", Qt::CaseInsensitive) == 0) {
        //qDebug() << "HTMLPage::Request" << url.toString();        
        webasset.SetURL(QUrl("workspaces"));
        ReadWorkspacesContent();
        webasset.SetLoaded(true);
        webasset.SetProcessing(true);
    }
    else if (MathUtil::img_extensions.contains(suffix) ||
             MathUtil::geom_extensions.contains(suffix) ||
             MathUtil::vid_extensions.contains(suffix)) {
        ba = webasset.GetData();
        webasset.SetLoaded(true);
        webasset.SetProcessing(true);
        LoadFile();
    }
    else {
//                qDebug() << "HTMLPage::Request() - Loading local file" << url.toLocalFile();
        QFileInfo f(url.toLocalFile());

        if (f.exists() && f.isDir()) {
            webasset.SetLoaded(true);
        }
        else if (f.exists() && f.isFile()) {
            QFile file(url.toLocalFile());
            if (file.open(QIODevice::ReadOnly)) {
                ba = file.readAll();
                file.close();

                LoadFile();
            }

            webasset.SetLoaded(true);
            webasset.SetProcessing(true);
        }
        else {
            if (MathUtil::domain_extensions.contains(suffix) || MathUtil::domain_extensions.contains(host_suffix)) {
                if (QString::compare("http", url.toString().left(4)) != 0) {
                    webasset.SetURL("http://" + url.toString());
                }
            }
            webasset.Load(webasset.GetURL());
        }
    }
}

WebAsset & HTMLPage::GetWebAsset()
{
    return webasset;
}

void HTMLPage::Update()
{    
    if (webasset.GetLoaded() && !webasset.GetProcessing()) {
//        qDebug() << "HTMLPage::Update()" << webasset.GetStarted() << webasset.GetLoaded() << webasset.GetProcessing();
        webasset.SetProcessing(true);

        const QUrl url = webasset.GetURL(); //59.3 - respect redirects
        const QString u = url.toString();

        if (QDir(u).exists() && !QFile(u).exists()) {
            content_type = "dir";
        }
        else {
            if (webasset.GetError()) {
                content_type = "error";
            }

            switch (webasset.GetStatusCode()) {
            case 400:
            case 401:
            case 402:
            case 403:
            case 404:
            case 408:
            case 500:          
                content_type = "error";
                break;

            case 0:
            case 200:
            default:                
                ba = webasset.GetData();
                LoadFile();                
                break;
            }
        }
    }
}

QString HTMLPage::FormCompleteURL(const QString & url_str)
{
    QString new_url_str = url_str;

    QUrl this_url(url_str);

    //qDebug() << "HTMLPage::FormCompleteURL() - Doing URL test with" << url_str << "and base URL" << url.toString();
    if (this_url.isRelative()) {
        new_url_str = webasset.GetURL().resolved(this_url).toString();
        //qDebug() << "HTMLPage::FormCompleteURL() - Found relative URL" << url_str << "and completed it" << new_url_str;
    }

    return new_url_str;

}

QStringList HTMLPage::GetStringsBetween(const QString & s1, const QString & s2, const QString & str)
{

    QStringList return_str_list;

    QString process_str = str;
    while (process_str.length() > 0) {

        //1.  cut up to the first string s1
        const int i1 = process_str.indexOf(s1);
        if (i1 == -1) {
            break;
        }
        process_str = process_str.right(process_str.length() - i1 - s1.length());

        //2. cut before the second string s2
        const int i2 = process_str.indexOf(s2);
        if (i2 == -1) {
            break;
        }
        return_str_list.push_back(process_str.left(i2));

        process_str = process_str.right(process_str.length() - i2 - s2.length());

    }

    return return_str_list;
}

void HTMLPage::HTMLExtractAll(const QString & start_tag, const QString & end_tag, const bool include_tags, const QString & html, QList <QString> & strings)
{
    int last_index = 0;
    do {
        int ind1 = html.indexOf(start_tag, last_index, Qt::CaseInsensitive);        
        if (ind1 >= 0) {
            int ind2 = html.indexOf(end_tag, ind1 + start_tag.length(), Qt::CaseInsensitive);

            QString return_str;
            if (include_tags) {
                return_str = html.left(ind2 + end_tag.length());
                return_str = return_str.right(return_str.length() - ind1);
                last_index = ind2 + end_tag.length();
            }
            else {
                return_str = html.left(ind2);
                return_str = return_str.right(return_str.length() - (ind1 + start_tag.length()));
                last_index = ind2;
            }

            return_str = return_str.trimmed();
            strings.push_back(return_str);
        }
        else {
            break;
        }
    } while (last_index >= 0);
}

bool HTMLPage::HTMLExtract(const QString & start_tag, const QString & end_tag, const bool include_tags, const QString & html, QString & return_str)
{
    //extract a title
    const int ind1 = html.indexOf(start_tag, 0, Qt::CaseInsensitive);
    if (ind1 >= 0) {
        const int ind2 = html.indexOf(end_tag, ind1+start_tag.length(), Qt::CaseInsensitive);

        if (include_tags) {
            return_str = html.left(ind2 + end_tag.length());
            return_str = return_str.right(return_str.length() - ind1);
        }
        else {
            return_str = html.left(ind2);
            return_str = return_str.right(return_str.length() - (ind1 + start_tag.length()));
        }
        return_str = return_str.trimmed();
        return true;
    }   
    else {
        return_str = QString("");
        return false;
    }

}

void HTMLPage::ConstructData()
{
//    qDebug() << "HTMLPage::ConstructData()";
    assets.insert("assetobject", assetobjectlist);
    assets.insert("assetimage", assetimagelist);
    assets.insert("assetghost", assetghostlist);
    assets.insert("assetrecording", assetrecordinglist);
    assets.insert("assetshader", assetshaderlist);
    assets.insert("assetscript", assetscriptlist);
    assets.insert("assetsound", assetsoundlist);
    assets.insert("assetvideo", assetvideolist);    
    assets.insert("assetwebsurface", assetwebsurfacelist);

    //do room
    room.insert("object", objectlist);
    room.insert("light", lightlist);
    room.insert("image", imagelist);
    room.insert("ghost", ghostlist);
    room.insert("sound", soundlist);
    room.insert("video", videolist);
    room.insert("link", linklist);
    room.insert("particle", particlelist);
    room.insert("text", textlist);
    room.insert("paragraph", paragraphlist);

    //
    fireboxroom.insert("assets", assets);
    fireboxroom.insert("room", room);

    data.insert("FireBoxRoom", fireboxroom);
}

void HTMLPage::ReadBookmarksContent()
{
    const QString translator_path = MathUtil::GetTranslatorPath();

    content_type = "firebox";

    room["title"] = "Bookmarks";
    room["col"] = "1 1 1";
    room["use_local_asset"] = "room_plane";
    room["visible"] = "true";

    const BookmarkManager b;
    const QVariantList urls = b.GetBookmarks();

    for (int i=0; i<urls.size(); ++i) {

        const QVariantMap bd = urls[i].toMap();

        const QString url = bd["url"].toString();
        const QString title = bd["title"].toString();
        const QString thumbnail = bd["thumbnail"].toString();

        QVariantMap img_data;
        img_data["src"] = thumbnail;
        img_data["id"] = QString("thumb") + QString::number(i);       

        QVariantMap link_data;
        link_data["id"] = QString("portal") + QString::number(i);
        link_data["url"] = url;
        link_data["title"] = title;
        link_data["pos"] = QString::number(-i*2 + (float(urls.size()) * 2.0f) * 0.5f - 1.0f) + " 0 5";
        link_data["fwd"] = "0 0 -1";
        link_data["scale"] = "1.8 2.5 1.0";
        link_data["thumb_id"] = img_data["id"];
        QColor c;
        c.setHsl(i*30, 128, 128);
        link_data["col"] = c.name();

        assetimagelist.push_back(img_data);
        linklist.push_back(link_data);
    }   

    ConstructData();
}

void HTMLPage::ReadWorkspacesContent()
{
    const QString thumb("thumb.jpg");

    //now generate the room
    content_type = "firebox";

    room["title"] = "Workspaces";
    room["col"] = QColor(255,255,255).name();
    room["use_local_asset"] = "room_plane";
    room["visible"] = "true";
    room["pos"] = "0 0 -1";
    room["fwd"] = "0 0 1";  

    //dynamically read all .html files in workspaces and those one subdirectory down
    BookmarkManager b;
    const QVariantList workspaces = b.GetWorkspaces();
    for (int i=0; i<workspaces.size(); ++i) {

        const QString url = workspaces[i].toMap()["url"].toString();

        QVariantMap img_data;
        if (QString::compare(url.left(4), "http") == 0) {
            img_data["src"] = QUrl(url).resolved(thumb).toString();
        }
        else {
            img_data["src"] = QUrl::fromLocalFile(url).resolved(thumb).toString();
        }
        img_data["id"] = QString("thumb") + QString::number(i);

        QVariantMap link_data;
        link_data["id"]= QString("portal") + QString::number(i);
        if (QString::compare(url.left(4), "http") == 0) {
            link_data["url"] = QUrl(url).toString();
        }
        else {
            link_data["url"] = QUrl::fromLocalFile(url).toString();
        }
        link_data["pos"] = QString::number(-i*2 + (float(workspaces.size()) * 2.0f) * 0.5f - 1.0f) + " 0 5";
        link_data["fwd"] = "0 0 -1";
        link_data["scale"] = "1.8 2.5 1.0";
//        link_data["thumb_id"] = img_data["id"];
        QColor c;
        c.setHsl(30 * i, 128, 128);
        link_data["col"] = c.name();

        assetimagelist.push_back(img_data);
        linklist.push_back(link_data);
    }    

    ConstructData();
}

void HTMLPage::ReadRegularHTMLContent(const QString & html_str)
{
    QStringList sl = html_str.split("<");
    //qDebug() << "HTMLPage::ReadRegularHTMLContent - HTML substrings" << sl.size();

    for (int i=0; i<sl.size(); ++i) {

        if (sl[i].contains("img ", Qt::CaseInsensitive) &&
                (sl[i].contains("src", Qt::CaseInsensitive) || sl[i].contains("data-thumb", Qt::CaseInsensitive))) {

            //qDebug() << sl[i];
            QStringList sl2 = sl[i].split("\"");

            for (int j=0; j<sl2.size()-1; ++j) {

                if (sl2[j].contains("src", Qt::CaseInsensitive) || sl2[j].contains("data-thumb", Qt::CaseInsensitive)) {

                    ++j;

                    //AddImage(sl2[j]);
                    //make both an assetimage and an env image
                    QString new_id = QString("id") + QString::number(i);
                    QString img_url = FormCompleteURL(sl2[j]);

                    QFileInfo fi(img_url);
                    QString suffix = fi.suffix();
                    //qDebug() << "maybeimg: " << img_url;
                    if (MathUtil::img_extensions.contains(suffix, Qt::CaseInsensitive)) {

                        if (!urls_known.contains(img_url)) {
                            QVariantMap img_data;
                            img_data["src"] = img_url;
                            img_data["id"] = new_id;
                            assetimagelist.push_back(img_data);

                            img_data.clear();
                            img_data["id"] = new_id;
                            imagelist.push_back(img_data);

                            //qDebug() << "img:" << img_url;
                            urls_known.insert(img_url);
                        }
                    }
                }
            }
        }
        else if ((sl[i].contains("a ", Qt::CaseInsensitive) &&
                  sl[i].contains("href", Qt::CaseInsensitive)) ||
                 (sl[i].contains("meta ", Qt::CaseInsensitive) &&
                  sl[i].contains("url", Qt::CaseInsensitive))) {

            QStringList sl2 = sl[i].split("\"");

            for (int j=0; j<sl2.size()-1; ++j) {

                if (sl2[j].contains("=", Qt::CaseInsensitive) &&
                    (sl2[j].contains("href", Qt::CaseInsensitive) || sl2[j].contains("data-href", Qt::CaseInsensitive))) {

                    ++j;

                    //if (sl2[j].right(5).contains(".") && !sl2[j].right(4).contains("htm")) {
                    //    continue;
                    //}

                    QString link_url = FormCompleteURL(sl2[j]);

                    if (!urls_known.contains(link_url)) {

                        QVariantMap new_link;
                        new_link["url"] = FormCompleteURL(sl2[j]);
                        linklist.push_back(new_link);

                        //qDebug() << "link:" << link_url;
                        urls_known.insert(link_url);
                    }
                }
            }
        }
        else if (sl[i].contains("div ", Qt::CaseInsensitive)) {

            //get this case working:
            //<div title="Google" align="left" id="hplogo" onload="window.lol&amp;&amp;lol()" style="background:url(/images/srpr/logo11w.png) no-repeat;background-size:269px 95px;height:95px;width:269px"><div nowrap="" style="color:#777;font-size:16px;font-weight:bold;position:relative;left:218px;top:70px">Canada</div></div>

            //HTMLData newdata;
            //newdata.type = DIV;
            //newdata.data = sl[i];
            //datas.push_back(newdata);

            //QStringList sl2 = sl[i].split("\"");
            QStringList sl2 = GetStringsBetween("\"", "\"", sl[i]);

            for (int j=0; j<sl2.size(); ++j) {

                QStringList sl3 = GetStringsBetween("(", ")", sl2[j]);

                for (int k=0; k<sl3.size(); ++k) {

                    QString img_url = FormCompleteURL(sl3[k]);

                    QFileInfo fi;
                    const QString suffix = fi.suffix();

                    //qDebug() << "maybediv: " << img_url;
                    if (MathUtil::img_extensions.contains(suffix, Qt::CaseInsensitive)) {

                        if (!urls_known.contains(img_url)) {

                            QString new_id = QString("id") + QString::number(i);

                            QVariantMap img_data;
                            img_data["src"] = img_url;
                            img_data["id"] = new_id;
                            assetimagelist.push_back(img_data);

                            img_data.clear();
                            img_data["id"] = new_id;
                            imagelist.push_back(img_data);

                            //qDebug() << "div:" << img_url;
                            urls_known.insert(img_url);
                        }
                    }
                }
            }
        }
        else {
            QString textpiece = sl[i].mid(sl[i].indexOf(">") + 1);
            textpiece = textpiece.trimmed();
            if (textpiece.length() > 0) {
                QVariantMap newdata;
                newdata["innertext"] = textpiece;
                paragraphlist.push_back(newdata);
            }
        //extract any other text
        /*
        QString textpiece = sl[i].mid(sl[i].indexOf(">") + 1);
        textpiece = textpiece.trimmed();
        if (textpiece.length() > 0) {

            HTMLData newdata;
            newdata.type = TEXT;
            newdata.text = textpiece;
            data_things.push_back(newdata);
            qDebug() << "text:" << textpiece;
            ++num_texts;

        }
        */
        }

    }
}

void HTMLPage::ReadImgurContent(const QString & imgur_data)
{
    //qDebug() << imgur_data;
    QList <QString> img_strings;

    HTMLExtractAll(QString("data-src=\""), QString("\""), false, imgur_data, img_strings);
    HTMLExtractAll(QString("<a href=\""), QString("\""), false, imgur_data, img_strings);
    HTMLExtractAll(QString("<img src=\""), QString("\""), false, imgur_data, img_strings);

//    qDebug() << "img_strings" << img_strings;
    //change url from smallImg version to big version of image
    for (int i=0; i<img_strings.size(); ++i) {
        QString smallImg = img_strings[i].right(5).left(1);
        if (QString::compare(smallImg, "s", Qt::CaseSensitive) == 0) {
            img_strings[i].replace(QString("s") + img_strings[i].right(4), img_strings[i].right(4));
        }
    }

    //remove duplicates
    for (int i=0; i<img_strings.size(); ++i) {
        for (int j=i+1; j<img_strings.size(); ++j) {
            if (QString::compare(img_strings[i], img_strings[j]) == 0) {
                img_strings.removeAt(j);
                --j;
            }
        }
    }

    //go through and add only ones that are links to actual images (have graphic format extension)
    for (int i=0; i<img_strings.size(); ++i) {

        const QFileInfo fi(img_strings[i]);
        const QString suffix = fi.suffix();

        if (MathUtil::img_extensions.contains(suffix, Qt::CaseInsensitive) && QString::compare(img_strings[i].left(7), "//pixel") != 0) {
            ImgurData img_data;
            img_data.img_url = FormCompleteURL(img_strings[i]);
            imgur_things.push_back(img_data);
        }

    }
}

void HTMLPage::ReadFlickrContent(const QString & imgur_data)
{
    QVector <QList <QString> > img_strings(9);

    HTMLExtractAll(QString("https://farm1.staticflickr.com/"), QString("\""), false, imgur_data, img_strings[0]);
    HTMLExtractAll(QString("https://farm2.staticflickr.com/"), QString("\""), false, imgur_data, img_strings[1]);
    HTMLExtractAll(QString("https://farm3.staticflickr.com/"), QString("\""), false, imgur_data, img_strings[2]);
    HTMLExtractAll(QString("https://farm4.staticflickr.com/"), QString("\""), false, imgur_data, img_strings[3]);
    HTMLExtractAll(QString("https://farm5.staticflickr.com/"), QString("\""), false, imgur_data, img_strings[4]);
    HTMLExtractAll(QString("https://farm6.staticflickr.com/"), QString("\""), false, imgur_data, img_strings[5]);
    HTMLExtractAll(QString("https://farm7.staticflickr.com/"), QString("\""), false, imgur_data, img_strings[6]);
    HTMLExtractAll(QString("https://farm8.staticflickr.com/"), QString("\""), false, imgur_data, img_strings[7]);
    HTMLExtractAll(QString("https://farm9.staticflickr.com/"), QString("\""), false, imgur_data, img_strings[8]);

    for (int i=0; i<img_strings.size(); ++i) {

        for (int j=0; j<img_strings[i].size(); ++j) {

            const QString  str = QString("https://farm") + QString::number(i+1) + QString(".staticflickr.com/") + img_strings[i][j];
            QString filetype = str.right(6);

            if (QString::compare(filetype, "_z.jpg", Qt::CaseInsensitive) == 0) { //||
                    //QString::compare(filetype, ".gif", Qt::CaseInsensitive) == 0 ||
                    //QString::compare(filetype, ".png", Qt::CaseInsensitive) == 0) {

                ImgurData img_data;
                img_data.img_url = FormCompleteURL(str);
                imgur_things.push_back(img_data);

            }
        }

    }
}

void HTMLPage::ReadRedditCommentContent(const QString & reddit_data)
{
    QList <QString> thread_score_strings;
    QList <QString> user_comment_strings;
    QList <QString> user_strings;
    QList <QString> score_strings;
    QList <QString> time_strings;

    HTMLExtractAll(QString("<div class=\"score unvoted"), QString("</div>"), true, reddit_data, thread_score_strings);
    HTMLExtractAll(QString("<div class=\"md\">"), QString("</div>"), true, reddit_data, user_comment_strings);
    HTMLExtractAll(QString("<a href=\"http://www.old.reddit.com/user"), QString("</a>"), true, reddit_data, user_strings);
    HTMLExtractAll(QString("<span class=\"score unvoted"), QString("</span>"), true, reddit_data, score_strings);
    HTMLExtractAll(QString("<time title=\""), QString("</time>"), true, reddit_data, time_strings);

//    qDebug () << "HTMLPage::ReadRedditCommentContent()" << user_comment_strings.size() << user_strings.size() << score_strings.size() << time_strings.size();
    score_strings.push_front("");

    for (int i=1; i<user_comment_strings.size(); ++i) {
        if (i < user_strings.size()) {
            user_strings.removeAt(i);
        }
        if (i < score_strings.size()) {
            score_strings.removeAt(i);
        }
        if (i < time_strings.size()) {
            time_strings.removeAt(i);
        }
    }

    for (int i=0; i<user_comment_strings.size(); ++i) {

        RedditData data;
        if (i == 0 && i < thread_score_strings.size()) {
            HTMLExtract(QString(">"), QString("<"), false, thread_score_strings[i], data.rank_str);
        }

        if (i < user_comment_strings.size()) {
            HTMLExtract(QString(">"), QString("</div>"), false, user_comment_strings[i], data.comment_str);
        }
        if (i <user_strings.size()) {
            HTMLExtract(QString(">"), QString("<"), false, user_strings[i], data.user_str);
        }
        if (i < score_strings.size()) {
            HTMLExtract(QString(">"), QString("<"), false, score_strings[i], data.score_str);
        }
        if (i < time_strings.size()) {
            HTMLExtract(QString(">"), QString("<"), false, time_strings[i], data.time_str);
        }
        reddit_things.push_back(data);
    }
}

void HTMLPage::ReadRedditContent(const QString & reddit_data)
{    
    const QString translator_path = MathUtil::GetTranslatorPath();
    //#1: find all tags that are: <a class="title loggedin" href="LINK">link description</a>
    //#2: find all tags that are: <div class="score unvoted">SCORE</div>
    QList <QString> things;

    HTMLExtractAll(QString("<div class=\" thing"), QString("<div class=\"clearleft\">"), true, reddit_data, things);

    QVector <QString> link_strings = QVector<QString>(things.size());
    QVector <QString> score_strings = QVector<QString>(things.size());
    QVector <QString> img_strings = QVector<QString>(things.size());
    QVector <QString> rank_strings = QVector<QString>(things.size());
    QVector <QString> comments_strings = QVector<QString>(things.size());
    QVector <QString> time_strings = QVector<QString>(things.size());
    QVector <QString> user_strings = QVector<QString>(things.size());

    for (int i=0; i<things.size(); ++i) {
        HTMLExtract(QString("<a class=\"title"), QString("</a>"), true, things[i], link_strings[i]);
        HTMLExtract(QString("<div class=\"score unvoted"), QString("</div>"), true, things[i], score_strings[i]);
        HTMLExtract(QString("<a class=\"thumbnail"), QString("</a>"), true, things[i], img_strings[i]);
        HTMLExtract(QString("<span class=\"rank"), QString("</span>"), true, things[i], rank_strings[i]);
        HTMLExtract(QString("<a class=\"comments"), QString("</a>"), true, things[i], comments_strings[i]);
        HTMLExtract(QString("<time title=\""), QString("</time>"), true, things[i], time_strings[i]);
        HTMLExtract(QString("<a href=\"http://www.old.reddit.com/user/"), QString("</a>"), true, things[i], user_strings[i]);
    }

    QString nav_substr;
    HTMLExtract(QString("<div class=\"nav-buttons\">"), QString("</div>"), false, reddit_data, nav_substr);
    QList <QString> nav_strings;
    HTMLExtractAll(QString("<a href=\""), QString("\""), false, nav_substr, nav_links);

    for (int i=0; i<link_strings.size(); ++i) {

        RedditData data;

        HTMLExtract(">", "<", false, link_strings[i], data.text_str);
        HTMLExtract(">", "<", false, score_strings[i], data.score_str);
        HTMLExtract(">", "<", false, rank_strings[i], data.rank_str);
        HTMLExtract(">", "<", false, comments_strings[i], data.comment_str);
        HTMLExtract("href=\"", "\"", false, link_strings[i], data.link_url);
        HTMLExtract(">", "<", false, time_strings[i], data.time_str);
        HTMLExtract(">", "<", false, user_strings[i], data.user_str);

        if (QString::compare(data.score_str, "&bull;") == 0) {
            data.score_str = "-";
        }

        bool img_found = HTMLExtract("src=\"", "\"", false, img_strings[i], data.img_url);
        if (img_found) {
            data.img_url = FormCompleteURL(data.img_url);
        }
        else {
            if (img_strings[i].contains("self", Qt::CaseInsensitive)) {
                data.img_url = translator_path + QString("reddit/self.png");
            }
            else {
                data.img_url = translator_path + QString("reddit/default.png");
            }
        }

        data.link_url = FormCompleteURL(data.link_url);

        reddit_things.push_back(data);
    }
}

void HTMLPage::TraverseXmlNode(const QDomNode & node, const int depth, QVariantMap & parent_data)
{    
    if (depth >= 32) {
        return;
    }   

//    qDebug() << __FUNCTION__ << node.toElement().tagName() << "depth" << depth;
    QDomElement e = node.toElement();
    QString tag_name = e.tagName().toLower();

    QVariantMap new_data;

    for (int i=0; i<e.attributes().size(); ++i) {
        QDomNode d = e.attributes().item(i);
        new_data[d.nodeName().toLower()] = d.nodeValue();
    }

    if ((tag_name == "text" || tag_name == "paragraph") && !e.text().isEmpty()) {
        new_data["text"] = e.text();
        new_data["innertext"] = e.text();
    }

    //now we iterate/recurse through all children of passed in node
    QDomNode domNode = node.firstChild();
    while (!domNode.isNull())
    {
        TraverseXmlNode(domNode, depth+1, new_data);
        domNode = domNode.nextSibling();
    }

    QVariantList l;
    if (QString::compare(tag_name, "assets", Qt::CaseInsensitive) == 0) {
        parent_data["assets"] = new_data;
    }
    else if (QString::compare(tag_name, "room", Qt::CaseInsensitive) == 0) {
        parent_data["room"] = new_data;
    }
    else {
        if (parent_data.contains(tag_name)) {
            l = parent_data[tag_name].toList();
        }
        l.push_back(new_data);
        parent_data[tag_name] = l;
    }
}

void HTMLPage::ReadJSONContent(const QString & firebox_data)
{
    QJsonParseError err;
    QJsonDocument d = QJsonDocument::fromJson(firebox_data.toUtf8(), &err);

    //check for JSON doc parsing error
    if (err.error != QJsonParseError::NoError) {
        const QString msg = "Error: HTMLPage::ReadJSONContent() - JSON Parse Error (" + QString::number(err.offset) + "): " + err.errorString();
//        qDebug() << msg;
        MathUtil::ErrorLog(msg);
        return;
    }

    data = d.toVariant().toMap();
}

void HTMLPage::ReadXMLContent(const QString & firebox_data)
{
//    qDebug() << "HTMLPage::ReadXMLContent()";
    QString error_msg;
    int error_line;
    int error_column;

    //qDebug() << "HTMLPage::ReadXMLContent " << firebox_data.size() << "...";
    QDomDocument doc;    
    doc.setContent(firebox_data, false, &error_msg, &error_line, &error_column);   

    QDomElement e = doc.documentElement();   

    QVariantMap m;

    QDomNode n = e.firstChild();
    while(!n.isNull()) {
        TraverseXmlNode(n, 1, m);
        n = n.nextSibling();
    }

    data["FireBoxRoom"] = m;
//    qDebug() << "HTMLPage::ReadXMLContent DATA!" << data;
}

float HTMLPage::GetProgress() const
{
    return webasset.GetProgress();
}

int HTMLPage::GetErrorCode() const
{
    return webasset.GetStatusCode();
}

void HTMLPage::LoadFile()
{    
    ClearData();

    QString ba_str(ba);
    QString firebox_data;
    QString reddit_data;

    //qDebug() << "HTMLPage::LoadFile() - ba_str len" << ba_str.size();
//    qDebug() << "HTMLPage::LoadFile()" << ba_str;

    //1.  set url info, other initializations
    const QString url_str = webasset.GetURL().toString();

    const QFileInfo fi(url_str);
    const QString suffix = fi.suffix();

    //2.  process based on type
    if (QString::compare(url_str.right(4), "json", Qt::CaseInsensitive) == 0) {
        title = url_str;
        content_type = "firebox";
        ReadJSONContent(ba_str);
    }
    else if (HTMLExtract("<fireboxroom>", "</fireboxroom>", true, ba_str, firebox_data)) {
        HTMLExtract("<title>", "</title>", false, ba_str, title);
        content_type = "firebox";
        ReadXMLContent(firebox_data);
    }
    else if (MathUtil::img_extensions.contains(suffix, Qt::CaseInsensitive)) {
        //qDebug() << "HTMLPage::LoadFile() - found single image content";
        content_type = "image";
        title = url_str;
        room["visible"] = "false";
        return;
    }
    else if (MathUtil::geom_extensions.contains(suffix, Qt::CaseInsensitive) ||
             MathUtil::geom_extensions.contains(url_str.right(3), Qt::CaseInsensitive) ||
             MathUtil::geom_extensions.contains(url_str.right(6), Qt::CaseInsensitive) ||
             MathUtil::geom_extensions.contains(url_str.right(7), Qt::CaseInsensitive)) {
        content_type = "geometry";
        title = url_str;
        room["visible"] = "false";
        return;
    }
    else if (MathUtil::vid_extensions.contains(suffix, Qt::CaseInsensitive)) {
        content_type = "video";
        title = url_str;
        room["visible"] = "false";
        return;
    }
    else if (url_str.left(26).contains("old.reddit.com")) {

        QString string;
//        qDebug() << "HTMLPage::ReadRedditContent:" << ba_str;
        if (HTMLExtract("<div id=\"siteTable\" class=\"sitetable linklisting\">", "<div class=\"footer-parent\">", true, ba_str, reddit_data)) {

            if (reddit_data.contains("permalink</a>")) {
                content_type = "reddit_comment";
                ReadRedditCommentContent(reddit_data);
            }
            else {
                content_type = "reddit";                
                ReadRedditContent(reddit_data);
            }

        }
        else if (HTMLExtract(QString("<div class=\"content over18\" style=\"text-align: center\">"), QString("</div>"), true, ba_str, reddit_data)) {
            content_type = "reddit_over18";
        }

        ConstructData();
    }
    else if (url_str.left(21).contains("imgur.com") && !MathUtil::img_extensions.contains(url_str.right(3)) && !MathUtil::img_extensions.contains(url_str.right(4))
             && !MathUtil::vid_extensions.contains(url_str.right(3)) && !MathUtil::vid_extensions.contains(url_str.right(4))
             && QString::compare(url_str.right(4), "gifv", Qt::CaseInsensitive) != 0) {
        content_type = "imgur";
        ReadImgurContent(ba_str);
        ConstructData();
    }
    else if (url_str.left(23).contains("youtube.com") && !MathUtil::img_extensions.contains(url_str.right(3)) && !MathUtil::img_extensions.contains(url_str.right(4))
             && !MathUtil::vid_extensions.contains(url_str.right(3)) && !MathUtil::vid_extensions.contains(url_str.right(4))
             && QString::compare(url_str.right(4), "gifv", Qt::CaseInsensitive) != 0) {
        content_type = "youtube";
    }
    else if (url_str.left(21).contains("vimeo.com") && !MathUtil::img_extensions.contains(url_str.right(3)) && !MathUtil::img_extensions.contains(url_str.right(4))
             && !MathUtil::vid_extensions.contains(url_str.right(3)) && !MathUtil::vid_extensions.contains(url_str.right(4))
             && QString::compare(url_str.right(4), "gifv", Qt::CaseInsensitive) != 0) {
        content_type = "vimeo";
    }
    else if (url_str.left(22).contains("flickr.com") && !MathUtil::img_extensions.contains(url_str.right(3)) && !MathUtil::img_extensions.contains(url_str.right(4))
             && !MathUtil::vid_extensions.contains(url_str.right(3)) && !MathUtil::vid_extensions.contains(url_str.right(4))) {
        content_type = "flickr";
        ReadFlickrContent(ba_str);
        ConstructData();
    }
    else {
        content_type = "html";
        ReadRegularHTMLContent(ba_str);
        ConstructData();
    }

    webasset.ClearData();
    ba.clear();
//    qDebug() << "DATA!" << data;
//    qDebug() << "HTMLPage::LoadFile() Loaded page" << title << data_things.first().children.size();
}

QString HTMLPage::GetTitle() const
{ 
    return title;
}

const QVariantMap & HTMLPage::GetData() const
{
    return data;
}

const QVariantMap HTMLPage::GetRoomData() const
{
    QVariantMap m = data["FireBoxRoom"].toMap();
    QVariantMap room;
    if (m.contains("Room")) {
        room = m["Room"].toMap();
    }
    else if (m.contains("room")) {
        room = m["room"].toMap();
    }
    return room;

}

const QList <RedditData> & HTMLPage::RedditThings() const
{
    return reddit_things;
}

const QList <ImgurData> & HTMLPage::ImgurThings() const
{
    return imgur_things;
}

const QList <QString> & HTMLPage::NavLinks() const
{
    return nav_links;
}

void HTMLPage::ClearData()
{
    reddit_things.clear();
    imgur_things.clear();
    nav_links.clear();

    //clear all the new containers
    data.clear();
    fireboxroom.clear();
    assets.clear();
    assetobjectlist.clear();
    assetimagelist.clear();
    assetghostlist.clear();
    assetrecordinglist.clear();
    assetshaderlist.clear();
    assetscriptlist.clear();
    assetsoundlist.clear();
    assetvideolist.clear();
    assetwebsurfacelist.clear();

    room.clear();
    objectlist.clear();
    imagelist.clear();
    ghostlist.clear();
    soundlist.clear();
    videolist.clear();
    linklist.clear();
    particlelist.clear();
    textlist.clear();
    paragraphlist.clear();

}

void HTMLPage::Clear()
{
//    qDebug() << "HTMLPage::Clear()" << this;
    ClearData();

    webasset.SetStarted(false);
    webasset.SetProcessing(false);
    webasset.SetProcessed(false);
    webasset.SetFinished(false);
    webasset.SetLoaded(false);
    webasset.ClearData();
}

bool HTMLPage::FoundRedditContent() const
{    
    return QString::compare(content_type, "reddit") == 0;
}

bool HTMLPage::FoundRedditCommentContent() const
{
    return QString::compare(content_type, "reddit_comment") == 0;
}

bool HTMLPage::FoundImgurContent() const
{
    return QString::compare(content_type, "imgur") == 0;
}

bool HTMLPage::FoundVimeoContent() const
{
    return QString::compare(content_type, "vimeo") == 0;
}

bool HTMLPage::FoundYoutubeContent() const
{
    return QString::compare(content_type, "youtube") == 0;
}

bool HTMLPage::FoundFlickrContent() const
{
    return QString::compare(content_type, "flickr") == 0;
}

bool HTMLPage::FoundFireBoxContent() const
{
    return QString::compare(content_type, "firebox") == 0;
}

bool HTMLPage::FoundStandardHTML() const
{
    return QString::compare(content_type, "html") == 0;
}

bool HTMLPage::FoundSingleImageContent() const
{
    return QString::compare(content_type, "image") == 0;
}

bool HTMLPage::FoundGeometryContent() const
{    
    return QString::compare(content_type, "geometry") == 0;
}

bool HTMLPage::FoundVideoContent() const
{
    return QString::compare(content_type, "video") == 0;
}

bool HTMLPage::FoundError() const
{
    return QString::compare(content_type, "error") == 0;
}

bool HTMLPage::FoundDirectoryListing() const
{
    return QString::compare(content_type, "dir") == 0;
}

QString HTMLPage::GfyGenerateRoomCode(const QString & htmlIn) {

    QString title;
    QString webmString;
    QString widthExtracted;
    QString heightExtracted;

    HTMLExtract("<title>", "</title>", false, htmlIn, title);
    bool webmFound = HTMLExtract(QString("<source id=\"webmsource\" src=\""), QString(".webm\""), false, htmlIn, webmString);    
    HTMLExtract("<meta name=\"twitter:player:width\" content=\"", "\">", false, htmlIn, widthExtracted);
    HTMLExtract("<meta name=\"twitter:player:height\" content=\"", "\">", false, htmlIn, heightExtracted);

    const int width = widthExtracted.toInt();
    const int height = heightExtracted.toInt();

    if (webmFound) {
        if (QString::compare(webmString.left(4), "http", Qt::CaseInsensitive) == 0) {
            return GfyGenerateGfyViewingRoom(title, webmString + ".webm", width, height);
        }
        else {
            return GfyGenerateGfyViewingRoom(title, "https:" + webmString + ".webm", width, height);
        }
    }
    else {
        return GfyGenerateGfyDefault();
    }
}

QString HTMLPage::GfyGenerateGfyViewingRoom(const QString title, const QString webmUrl, const int width, const int height) {

    return
        "<html>\n"
        "<head>\n"
        "<title>" + title + "</title>\n"
        "</head>\n"
        "<body>\n"
        "<FireBoxRoom>\n"
        "    <Assets>\n"
        "        <AssetImage id=\"black_img\" src=\"" + MathUtil::GetTranslatorPath() + "gfycat/black.png\" />\n"
//        "        <AssetVideo id=\"gfy\" loop=\"true\" auto_play=\"true\" src=\"" + webmUrl + "\" />\n"
        "        <AssetWebSurface id=\"gfy\" width=\"" + QString::number(width) + "\" height=\"" + QString::number(height) +"\" src=\"" + webmUrl + "\" />\n"
        "    </Assets>\n"
        "    <Room\n"
        "        use_local_asset = \"room_plane\"\n"
        "        visible = \"false\"\n"
        "        pos = \"0 0 0\"\n"
        "        xdir = \"-1 0 -1.26759e-06\"\n"
        "        ydir = \"0 1 0\"\n"
        "        zdir = \"0 0 -1\"\n"
        "        col = \"1 1 1\"\n"
        "        skybox_left_id = \"black_img\"\n"
        "        skybox_right_id = \"black_img\"\n"
        "        skybox_front_id = \"black_img\"\n"
        "        skybox_back_id = \"black_img\"\n"
        "        skybox_up_id = \"black_img\"\n"
        "        skybox_down_id = \"black_img\"\n"        
        "    >\n"
//        "        <Video id=\"gfy\" pos=\"0 1.8 -9\" scale=\"" + scale + "\" />\n"
        "        <Object id=\"plane\" websurface_id=\"gfy\" pos=\"0 1.8 -9\" scale=\"8 " + QString::number(8.0f*float(height)/float(width)) + " 1\" />\n"
        "    </Room>\n"
        "</FireBoxRoom>\n"
        "</body>\n"
        "</html>\n"
    ;
}

QString HTMLPage::GfyGenerateGfyDefault() {

    return
        "<html>\n"
        "<head>\n"
        "<title>GFYCat.com</title>\n"
        "</head>\n"
        "<body>\n"
        "<FireBoxRoom>\n"
        "    <Assets>\n"
        "        <AssetImage id=\"logo\" src=\"" + MathUtil::GetTranslatorPath() + "gfycat/gfycat_logo.png\" />\n"
        "    </Assets>\n"
        "    <Room\n"
        "          use_local_asset = \"room_1pedestal\"\n"
        "          pos=\"0 0 0\"\n"
        "          fwd=\"0 0 -1\"\n"        
        "    >\n"
        "        <Image id=\"logo\" pos=\"0 2 -10\" scale=\"2 2 .01\" />\n"
        "        <Link pos=\"0 0 -20\" url=\"http://www.old.reddit.com/r/gfycats/\" col=\"0.81 0.89 0.97\" title=\"/r/GFYCats\" scale=\"1.8 2.5 1\" />\n"
        "    </Room>\n"
        "</FireBoxRoom>\n"
        "</body>\n"
        "</html>\n"
    ;
}

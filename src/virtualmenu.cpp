#include "virtualmenu.h"

VirtualMenu::VirtualMenu() :
    menu_index(VirtualMenuIndex_MAIN),
    visible(false),
    taking_screenshot(false),
    cur_bookmark(0),
    num_bookmarks(0),
    cur_user(0),
    num_users(0),
    do_back(false),
    do_forward(false),
    do_reload(false),
    do_reset(false),
    do_exit(false),
    do_create_portal(false),
    do_bookmark_add(false),
    do_bookmark_remove(false),
    do_search(false),
    do_setuserid(false)
{
    assetobjs["cube"] = QPointer<AssetObject>(new AssetObject());
    assetobjs["cube"]->SetSrc(MathUtil::GetApplicationURL(), QString("assets/primitives/cube.obj"));
    assetobjs["cube"]->GetProperties()->SetID("cube");
    assetobjs["cube"]->GetProperties()->SetPrimitive(true);
    assetobjs["cube"]->GetProperties()->SetSaveToMarkup(false);
    assetobjs["cube"]->Load();

    assetobjs["plane"] = QPointer<AssetObject>(new AssetObject());
    assetobjs["plane"]->SetSrc(MathUtil::GetApplicationURL(), QString("assets/primitives/plane.obj"));
    assetobjs["plane"]->GetProperties()->SetID("cube");
    assetobjs["plane"]->GetProperties()->SetPrimitive(true);
    assetobjs["plane"]->GetProperties()->SetSaveToMarkup(false);
    assetobjs["plane"]->Load();

    //connect(&partymode_request_timer, SIGNAL(timeout()), this, SLOT(UpdatePartyModeList()));
    //partymode_request_timer.start(5000);
}

VirtualMenu::~VirtualMenu()
{
    Clear();
    //disconnect(&partymode_request_timer, 0, 0, 0);
}

void VirtualMenu::SetBookmarkManager(QPointer <BookmarkManager> b)
{
    bookmarkmanager = b;
}

void VirtualMenu::SetMultiPlayerManager(QPointer <MultiPlayerManager> m)
{
    multi_players = m;
}

VirtualMenuButton * VirtualMenu::AddNewButton(const VirtualMenuIndex index, const QString js_id, const QString label, const QMatrix4x4 m)
{
    VirtualMenuButton * b = new VirtualMenuButton(js_id, label, m);

    b->button->SetAssetObject(assetobjs["cube"]);
    b->button->SetCollisionAssetObject(assetobjs["cube"]);
    b->label->SetAssetObject(assetobjs["cube"]);

    envobjects[index][b->button->GetProperties()->GetJSID()] = b->button;
    envobjects_text[index][b->label->GetProperties()->GetJSID()] = b->label;
    return b;
}

VirtualMenuImageButton * VirtualMenu::AddNewImageButton(const VirtualMenuIndex index, const QString js_id, const QString url, const QString thumb_id, const QMatrix4x4 m)
{
    if (!assetimgs.contains(thumb_id)) {
        assetimgs[thumb_id] = QPointer<AssetImage>(new AssetImage());
        assetimgs[thumb_id]->SetSrc(thumb_id, thumb_id);
        assetimgs[thumb_id]->Load();
    }

    VirtualMenuImageButton * b = new VirtualMenuImageButton(js_id, url, m);
    b->button->SetAssetObject(assetobjs["cube"]);
    b->button->SetCollisionAssetObject(assetobjs["cube"]);
    b->button->SetAssetImage(assetimgs[thumb_id]);

    envobjects[index][b->button->GetProperties()->GetJSID()] = b->button;
    envobjects_text[index][b->label->GetProperties()->GetJSID()] = b->label;

    return b;
}

VirtualMenuIconButton * VirtualMenu::AddNewIconButton(const VirtualMenuIndex index, const QString js_id, const QString imageurl, const QMatrix4x4 m)
{
    if (!assetimgs.contains(imageurl)) {
        assetimgs[imageurl] = QPointer<AssetImage>(new AssetImage());
        assetimgs[imageurl]->SetSrc(MathUtil::GetApplicationURL(), imageurl);
        assetimgs[imageurl]->Load();
    }

    VirtualMenuIconButton * b = new VirtualMenuIconButton(js_id, m);
    b->button->SetAssetObject(assetobjs["cube"]);
    b->button->SetCollisionAssetObject(assetobjs["cube"]);
    b->overlay->SetAssetObject(assetobjs["plane"]);
    b->overlay->SetAssetImage(assetimgs[imageurl]);

    envobjects[index][b->button->GetProperties()->GetJSID()] = b->button;
    envobjects_text[index][b->overlay->GetProperties()->GetJSID()] = b->overlay;

    return b;
}

VirtualMenuImageUserButton * VirtualMenu::AddNewImageUserButton(const VirtualMenuIndex index, const QString js_id, const QString user, const QString url, const QString thumb_id, const QMatrix4x4 m)
{
    if (!assetimgs.contains(thumb_id)) {
        assetimgs[thumb_id] = QPointer<AssetImage>(new AssetImage());
        assetimgs[thumb_id]->SetSrc(thumb_id, thumb_id);
        assetimgs[thumb_id]->Load();
    }

    VirtualMenuImageUserButton * b = new VirtualMenuImageUserButton(js_id, user, url, m);
    b->button->SetAssetObject(assetobjs["cube"]);
    b->button->SetCollisionAssetObject(assetobjs["cube"]);
    b->button->SetAssetImage(assetimgs[thumb_id]);

    envobjects[index][b->button->GetProperties()->GetJSID()] = b->button;
    envobjects_text[index][b->labelurl->GetProperties()->GetJSID()] = b->labelurl;
    envobjects_text[index][b->labeluser->GetProperties()->GetJSID()] = b->labeluser;

    return b;
}

void VirtualMenu::SetVisible(const bool b)
{
    visible = b;
}

bool VirtualMenu::GetVisible() const
{
    return visible;
}

void VirtualMenu::SetTakingScreenshot(const bool b)
{
    taking_screenshot = b;
}

bool VirtualMenu::GetTakingScreenshot() const
{
    return taking_screenshot;
}

void VirtualMenu::SetMenuIndex(VirtualMenuIndex index)
{
    menu_index = index;
}

VirtualMenuIndex VirtualMenu::GetMenuIndex() const
{
    return menu_index;
}

void VirtualMenu::Update()
{
    //hide menu if we moved
    const QString cur_url = multi_players->GetCurURL();
    if (cur_url != last_url) {
        visible = false;
    }
    last_url = cur_url;

    for (QPointer <AssetImage> & a : assetimgs) {
        if (a) {
            a->UpdateGL();
        }
    }

    for (QPointer <AssetObject> & a : assetobjs) {
        if (a) {
            if (!a->GetStarted()) {
                a->Load();
            }
            a->Update();
            a->UpdateGL();
        }
    }

    if (menu_index == VirtualMenuIndex_SEARCH && do_search && !entered_search.isEmpty() && entered_search_time.elapsed() > 250) {
        do_search = false;
        if (!search_data_request.GetStarted() || search_data_request.GetProcessed()) {
            search_data_request.Load(QUrl("https://vesta.janusxr.org/api/search?query=" + entered_search));
        }
    }   

    /*if (partymode_data_request.GetLoaded() && !partymode_data_request.GetProcessed()) {
        const QByteArray & ba = partymode_data_request.GetData();
        MathUtil::GetPartyModeData() = QJsonDocument::fromJson(ba).toVariant().toMap()["data"].toList();
        partymode_data_request.SetProcessed(true);
        ConstructSubmenus();        
        //qDebug() << MathUtil::GetPartyModeData();
    }*/

    if (search_data_request.GetLoaded() && !search_data_request.GetProcessed()) {
        const QByteArray & ba = search_data_request.GetData();
        search_data = QJsonDocument::fromJson(ba).toVariant().toMap()["data"].toList();
        search_data_request.SetProcessed(true);
        //qDebug() << "VirtualMenu::Update()" << search_data;
        ConstructSubmenus();
    }
}

void VirtualMenu::DrawGL(QPointer <AssetShader> shader)
{
    for (QPointer <RoomObject> & o : envobjects[menu_index]) {
        if (o) {
            o->Update(0.0f);
            o->DrawGL(shader, true, QVector3D(0,0,0));
        }
    }

    for (QPointer <RoomObject> & o : envobjects_text[menu_index]) {
        if (o) {
            o->Update(0.0f);
            o->DrawGL(shader, true, QVector3D(0,0,0));
        }
    }

    if (menu_index == VirtualMenuIndex_AVATAR) {
        MathUtil::PushModelMatrix();
        MathUtil::MultModelMatrix(modelmatrix);
        QPointer <RoomObject> player_avatar = multi_players->GetPlayer();
        if (player_avatar) {
            QVector <GhostFrame> frames;
            frames.push_back(GhostFrame());
            frames.push_back(GhostFrame());
            frames[0].time_sec = 0.0f;
            frames[1].time_sec = 1.0f;

            //65.2 incorporate head-tracking into frame's head_xform
            QMatrix4x4 m;
            m.rotate(180.0f,0,1,0);

            const QVector3D y = m * player_avatar->GetProperties()->GetUpDir()->toQVector3D();
            const QVector3D z = m * player_avatar->GetProperties()->GetViewDir()->toQVector3D();
            const QVector3D orig_d = player_avatar->GetDir();

            const QVector3D d = modelmatrix.column(2).toVector3D();

            frames[0].dir = d;
            frames[0].SetHeadXForm(y, z);
            frames[0].dir = orig_d;

            frames[1].dir = d;
            frames[1].SetHeadXForm(y, z);
            frames[1].dir = orig_d;

            if (player_avatar->GetAssetGhost()) {
                player_avatar->GetAssetGhost()->SetFromFrames(frames, 1000);
            }

            const QVector3D p = player_avatar->GetPos();
            const QString s = player_avatar->GetHMDType();
            player_avatar->SetHMDType("");
            player_avatar->GetProperties()->SetPos(QVector3D(0,0,0));            
            player_avatar->Update(0.0f);
            player_avatar->DrawGL(shader, true, QVector3D(0,0,0));
            player_avatar->SetHMDType(s);

            player_avatar->GetProperties()->SetPos(p);
            player_avatar->SetDir(orig_d);
        }

        MathUtil::PopModelMatrix();
    }
}

QHash <QString, QPointer <RoomObject> > & VirtualMenu::GetEnvObjects()
{
    return envobjects[menu_index];
}

void VirtualMenu::mousePressEvent(const QString selected)
{

}

void VirtualMenu::mouseReleaseEvent(const QString selected)
{
    //62.2 - ignore release events if not on objects in this menu
    if (!envobjects[menu_index].contains(selected)) {
        return;
    }

    switch (menu_index) {
    case VirtualMenuIndex_MAIN:
        if (selected == "__bookmarkadd") {
            do_bookmark_add = true;
        }
        else if (selected == "__bookmarkremove") {
            do_bookmark_remove = true;
        }
        else if (selected == "__url") {
            entered_url = "http://";
            if (envobjects_text[VirtualMenuIndex_URL]["__enteredurl_label"]) {
                envobjects_text[VirtualMenuIndex_URL]["__enteredurl_label"]->SetText(entered_url);
            }
            menu_index = VirtualMenuIndex_URL;
        }
        else if (selected == "__back") {
            do_back = true;
        }
        else if (selected == "__forward") {
            do_forward = true;
        }
        else if (selected == "__reload") {
            do_reload = true;
        }
        else if (selected == "__home") {
            do_reset = true;
        }
        else if (selected == "__bookmarks") {
            menu_index = VirtualMenuIndex_BOOKMARKS;
        }
        else if (selected == "__avatar") {
            menu_index = VirtualMenuIndex_AVATAR;
        }
        else if (selected == "__search") {
            menu_index = VirtualMenuIndex_SEARCH;
        }
        else if (selected == "__social") {
            menu_index = VirtualMenuIndex_SOCIAL;
        }
        else if (selected == "__exit") {
            do_exit = true;
        }
        break;
    case VirtualMenuIndex_URL:
        if (selected == "__backspace") {
            entered_url = entered_url.left(entered_url.length()-1);
            if (envobjects_text[VirtualMenuIndex_URL]["__enteredurl_label"]) {
                envobjects_text[VirtualMenuIndex_URL]["__enteredurl_label"]->SetText(entered_url);
            }
        }
        else if (selected == "__enter") {
            menu_index = VirtualMenuIndex_MAIN;
            do_create_portal = true;
            create_portal_url = entered_url;
        }
        else if (selected == "__enteredurl") {

        }
        else {
            entered_url += selected.right(1);
            if (envobjects_text[VirtualMenuIndex_URL]["__enteredurl_label"]) {
                envobjects_text[VirtualMenuIndex_URL]["__enteredurl_label"]->SetText(entered_url);
            }
        }
        break;
    case VirtualMenuIndex_SEARCH:
        if (selected == "__backspace") {
            do_search = true;
            entered_search_time.restart();
            entered_search = entered_search.left(entered_search.length()-1);
            if (envobjects_text[VirtualMenuIndex_SEARCH]["__enteredsearch_label"]) {
                envobjects_text[VirtualMenuIndex_SEARCH]["__enteredsearch_label"]->SetText(entered_search);
            }
        }
        else if (selected == "__space") {
            do_search = true;
            entered_search_time.restart();
            entered_search += " ";
            if (envobjects_text[VirtualMenuIndex_SEARCH]["__enteredsearch_label"]) {
                envobjects_text[VirtualMenuIndex_SEARCH]["__enteredsearch_label"]->SetText(entered_search);
            }
        }
        else if (selected == "__enteredsearch") {
        }
        else if (selected.left(10) == "__bookmark") {
            if (envobjects[VirtualMenuIndex_SEARCH][selected]) {
                menu_index = VirtualMenuIndex_MAIN;
                do_create_portal = true;
                create_portal_url = envobjects[VirtualMenuIndex_SEARCH][selected]->GetURL();
            }
        }
        else {
            do_search = true;
            entered_search_time.restart();
            entered_search += selected.right(1);
            if (envobjects_text[VirtualMenuIndex_SEARCH]["__enteredsearch_label"]) {
                envobjects_text[VirtualMenuIndex_SEARCH]["__enteredsearch_label"]->SetText(entered_search);
            }
        }
        break;
    case VirtualMenuIndex_AVATAR:
        if (selected == "__backspace") {
            entered_userid = entered_userid.left(entered_userid.length()-1);
            if (envobjects_text[VirtualMenuIndex_AVATAR]["__entereduserid_label"]) {
                envobjects_text[VirtualMenuIndex_AVATAR]["__entereduserid_label"]->SetText(entered_userid);
            }
        }
        else if (selected == "__enter") {
            do_setuserid = true;
        }
        else if (selected == "__entered_userid") {

        }
        else {
            entered_userid += selected.right(1);
            if (envobjects_text[VirtualMenuIndex_AVATAR]["__entereduserid_label"]) {
                envobjects_text[VirtualMenuIndex_AVATAR]["__entereduserid_label"]->SetText(entered_userid);
            }
        }
        break;
    case VirtualMenuIndex_BOOKMARKS:
        if (selected == "__bookmarkup") {
            if (cur_bookmark+16 < num_bookmarks) {
                cur_bookmark += 16;
                ConstructSubmenus();
            }
        }
        else if (selected == "__bookmarkdown") {
            menu_index = VirtualMenuIndex_BOOKMARKS;
            if (cur_bookmark>0) {
                cur_bookmark -= 16;
                ConstructSubmenus();
            }
        }
        else {
            QPointer <RoomObject> o = envobjects[VirtualMenuIndex_BOOKMARKS][selected];
            if (o) {
                menu_index = VirtualMenuIndex_MAIN;
                do_create_portal = true;
                create_portal_url = o->GetProperties()->GetURL();
                create_portal_thumb = o->GetAssetImage() ? o->GetAssetImage()->GetURL().toString() : "";
            }
        }
        break;
    case VirtualMenuIndex_SOCIAL:
        if (selected == "__socialup") {
            if (cur_user+16 < num_users) {
                cur_user += 16;
                ConstructSubmenus();
            }
        }
        else if (selected == "__socialdown") {
            menu_index = VirtualMenuIndex_BOOKMARKS;
            if (cur_user>0) {
                cur_user -= 16;
                ConstructSubmenus();
            }
        }
        else {
            QPointer <RoomObject> o = envobjects[VirtualMenuIndex_SOCIAL][selected];
            if (o) {
                menu_index = VirtualMenuIndex_MAIN;
                do_create_portal = true;
                create_portal_url = o->GetProperties()->GetURL();
                create_portal_thumb = o->GetAssetImage() ? o->GetAssetImage()->GetURL().toString() : "";
            }
        }
        break;
    case VirtualMenuIndex_KEYBOARD:
        if (websurface && selected.length() > 2) {
            QString text = selected.right(selected.length()-2);
            //qDebug() << "virtual kb text" << text;
            if (selected == "__backspace") {
                QKeyEvent * e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
                QKeyEvent * e2 = new QKeyEvent(QKeyEvent::KeyRelease, Qt::Key_Backspace, Qt::NoModifier);
                websurface->keyPressEvent(e);
                websurface->keyReleaseEvent(e2);
            }
            else if (selected == "__enter") {
                QKeyEvent * e = new QKeyEvent(QKeyEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
                QKeyEvent * e2 = new QKeyEvent(QKeyEvent::KeyRelease, Qt::Key_Enter, Qt::NoModifier);
                websurface->keyPressEvent(e);
                websurface->keyReleaseEvent(e2);
            }
            else if (selected == "__space") {
                QKeyEvent * e = new QKeyEvent(QKeyEvent::KeyPress, 0, Qt::NoModifier, " ");
                QKeyEvent * e2 = new QKeyEvent(QKeyEvent::KeyRelease, 0, Qt::NoModifier);
                websurface->keyPressEvent(e);
                websurface->keyReleaseEvent(e2);
            }
            else {
                QKeyEvent * e = new QKeyEvent(QKeyEvent::KeyPress, 0, Qt::NoModifier, text);
                QKeyEvent * e2 = new QKeyEvent(QKeyEvent::KeyRelease, 0, Qt::NoModifier);
                websurface->keyPressEvent(e);
                websurface->keyReleaseEvent(e2);
            }
        }
        break;
    default:
        break;
    }
}

void VirtualMenu::SetModelMatrix(const QMatrix4x4 m)
{
    modelmatrix = m;
}

QMatrix4x4 VirtualMenu::GetModelMatrix() const
{
    return modelmatrix;
}

bool VirtualMenu::GetDoBack()
{
    if (do_back) {
        do_back = false;
        return true;
    }
    else {
        return false;
    }
}

bool VirtualMenu::GetDoForward()
{
    if (do_forward) {
        do_forward = false;
        return true;
    }
    else {
        return false;
    }
}

bool VirtualMenu::GetDoReload()
{
    if (do_reload) {
        do_reload = false;
        return true;
    }
    else {
        return false;
    }
}

bool VirtualMenu::GetDoReset()
{
    if (do_reset) {
        do_reset = false;
        return true;
    }
    else {
        return false;
    }
}

bool VirtualMenu::GetDoSetUserID()
{
    if (do_setuserid) {
        do_setuserid = false;
        return true;
    }
    else {
        return false;
    }
}

QString VirtualMenu::GetEnteredUserID()
{
    return entered_userid;
}

bool VirtualMenu::GetDoExit()
{
    if (do_exit) {
        do_exit = false;
        return true;
    }
    else {
        return false;
    }
}

bool VirtualMenu::GetDoCreatePortal()
{
    if (do_create_portal) {
        do_create_portal = false;
        return true;
    }
    else {
        return false;
    }
}

QString VirtualMenu::GetDoCreatePortalURL()
{
    return create_portal_url;
}

QString VirtualMenu::GetDoCreatePortalThumb()
{
    return create_portal_thumb;
}

bool VirtualMenu::GetDoBookmarkAdd()
{
    if (do_bookmark_add) {
        do_bookmark_add = false;
        return true;
    }
    else {
        return false;
    }
}

bool VirtualMenu::GetDoBookmarkRemove()
{
    if (do_bookmark_remove) {
        do_bookmark_remove = false;
        return true;
    }
    else {
        return false;
    }
}

void VirtualMenu::SetWebSurface(QPointer <AssetWebSurface> w)
{
    websurface = w;
}

QPointer<AssetWebSurface> VirtualMenu::GetWebSurface() const
{
    return websurface;
}

void VirtualMenu::MenuButtonPressed()
{
    if (!visible) {
        visible = true;
        menu_index = VirtualMenuIndex_MAIN;
        ConstructSubmenus();
    }
    else {
        switch (menu_index) {
        case VirtualMenuIndex_MAIN:
        case VirtualMenuIndex_KEYBOARD:
            visible = false;
            break;            
        default:
            menu_index = VirtualMenuIndex_MAIN;
            break;
        }
    }
}

void VirtualMenu::Clear()
{
    envobjects.clear();
    envobjects_text.clear();
}

void VirtualMenu::ConstructSubmenus()
{
    Clear();

    //construct submenus
    ConstructSubmenuMain();
    ConstructSubmenuURL();
    ConstructSubmenuBookmarks();
    ConstructSubmenuAvatar();
    ConstructSubmenuSearch();
    ConstructSubmenuSocial();
    ConstructSubmenuKeyboard();
}

void VirtualMenu::ConstructSubmenuMain()
{
    QMatrix4x4 m = modelmatrix;
    m.translate(0,2.25f,0);

    QMatrix4x4 m3 = m;
    m3.scale(4,1,1);
    AddNewButton(VirtualMenuIndex_MAIN, "__url", multi_players->GetCurURL(), m3);

    m.translate(0,-0.25f,0);

    QMatrix4x4 m2 = m;
    m2.scale(0.2f, 0.2f, 1.0f);
    m2.translate(-2.0f, 0,0);
    AddNewIconButton(VirtualMenuIndex_MAIN, "__back", "assets/icons/back.png", m2);
    m2.translate(1.0f,0,0);

    AddNewIconButton(VirtualMenuIndex_MAIN, "__forward", "assets/icons/forward.png", m2);
    m2.translate(1.0f,0,0);

    AddNewIconButton(VirtualMenuIndex_MAIN, "__reload", "assets/icons/reload.png", m2);
    m2.translate(1.0f,0,0);

    AddNewIconButton(VirtualMenuIndex_MAIN, "__home", "assets/icons/home.png", m2);
    m2.translate(1.0f,0,0);

    bool bookmarked = false;
    if (bookmarkmanager && multi_players) {
        const QString cur_url = multi_players->GetCurURL();
        bookmarked = bookmarkmanager->GetBookmarked(cur_url);
    }
    AddNewIconButton(VirtualMenuIndex_MAIN, bookmarked? "__bookmarkremove" : "__bookmarkadd", bookmarked? "assets/icons/bookmarked.png" : "assets/icons/bookmark.png", m2);

    m.translate(0,-0.25f,0);

    AddNewButton(VirtualMenuIndex_MAIN, "__bookmarks", "Bookmarks", m);
    m.translate(0,-0.25f,0);

    AddNewButton(VirtualMenuIndex_MAIN, "__avatar", "Avatar", m);
    m.translate(0,-0.25f,0);

    AddNewButton(VirtualMenuIndex_MAIN, "__search", "Search", m);
    m.translate(0,-0.25f,0);

    AddNewButton(VirtualMenuIndex_MAIN, "__social", "Social", m);
    m.translate(0,-0.25f,0);

    AddNewButton(VirtualMenuIndex_MAIN, "__exit", "Exit", m);
    m.translate(0,-0.25f,0);
}

void VirtualMenu::ConstructSubmenuURL()
{
    QMatrix4x4 m = modelmatrix;
    m.translate(0,1.75f,0);
    m.scale(3.2f,0.8f,1);
    VirtualMenuButton * b = AddNewButton(VirtualMenuIndex_URL, "__enteredurl", entered_url, m);
    b->label->GetProperties()->SetJSID("__enteredurl_label");

    QList <QString> rows;
    rows.push_back("~1234567890-_+");
    rows.push_back("qwertyuiop");
    rows.push_back("asdfghjkl:");
    rows.push_back("zxcvbnm,./");

    for (int i=0; i<rows.size(); ++i) {
        QMatrix4x4 m = modelmatrix;
        m.translate(-1.5f, 1.5f - i*0.17f,0);
        m.scale(0.2f, 0.8f, 1);

        if (i == 1) {
            m.translate(1.5f,0,0);
        }
        else if (i == 2) {
            m.translate(2.0f,0,0);
        }
        else if (i == 3) {
            m.translate(2.5f,0,0);
        }

        for (int j=0; j<rows[i].length(); ++j) {
            AddNewButton(VirtualMenuIndex_URL, "__" + rows[i].mid(j,1), rows[i].mid(j,1), m);
            m.translate(1.05f,0,0);
        }

        if (i == 0) {
            m.translate(-0.5f,0,0);
            m.scale(2,1,1);
            m.translate(0.5f,0,0);
            AddNewButton(VirtualMenuIndex_URL, "__backspace", "<--", m);
        }
        else if (i == 2) {
            m.translate(-0.5f,0,0);
            m.scale(2,1,1);
            m.translate(0.5f,0,0);
            VirtualMenuButton * b = AddNewButton(VirtualMenuIndex_URL, "__enter", "Enter", m);
            b->button->GetProperties()->SetColour(QVector4D(0.5f,1.0f,0.5f,1.0f));
        }
    }
}

void VirtualMenu::ConstructSubmenuBookmarks()
{
    if (bookmarkmanager && multi_players) {        
        QVariantList list = bookmarkmanager->GetBookmarks() + bookmarkmanager->GetWorkspaces();
        num_bookmarks = list.length();

        //qDebug() << "VirtualMenu::ConstructSubmenuBookmarks()" << cur_bookmark << num_bookmarks;
        if (cur_bookmark > 0) {
            QMatrix4x4 m_down = modelmatrix;
            m_down.translate(-1.35f, 1.25f, 0.0f);
            m_down.scale(0.5f, 1.0f, 1.0f);
            AddNewButton(VirtualMenuIndex_BOOKMARKS, "__bookmarkdown", "<-", m_down);
        }

        if (cur_bookmark + 16 < num_bookmarks) {
            QMatrix4x4 m_up = modelmatrix;
            m_up.translate(1.35f, 1.25f, 0.0f);
            m_up.scale(0.5f, 1.0f, 1.0f);
            AddNewButton(VirtualMenuIndex_BOOKMARKS, "__bookmarkup", "->", m_up);
        }

        for (int i=cur_bookmark; i<qMin(num_bookmarks, cur_bookmark+16); ++i) {
            int x = i % 4;
            int y = (i/4) % 4;
            QMatrix4x4 m = modelmatrix;
            m.translate(x*0.55f-0.85f, 2.1f-y*0.55f, 0);
            m.scale(0.5f, 0.5f, 1.0f);

            QMap <QString, QVariant> o = list[i].toMap();
            const QString url = o["url"].toString();
            const QString thumbnail = o["thumbnail"].toString();
            AddNewImageButton(VirtualMenuIndex_BOOKMARKS, "__bookmark" + QString::number(i), url, thumbnail, m);
        }
    }
}

void VirtualMenu::ConstructSubmenuAvatar()
{
    if (entered_userid.isEmpty()) {
        entered_userid = multi_players->GetPlayer()->GetProperties()->GetID();
    }

    QMatrix4x4 m = modelmatrix;
    m.translate(0,1.1f,0.8f);
    m.scale(1.5f,0.8f,1);
    m.rotate(-30.0f, 1, 0, 0);
    VirtualMenuButton * b = AddNewButton(VirtualMenuIndex_AVATAR, "__entereduserid", entered_userid, m);
    b->label->GetProperties()->SetJSID("__entereduserid_label");

    QList <QString> rows;
    rows.push_back("~1234567890-_");
    rows.push_back("qwertyuiop[]");
    rows.push_back("asdfghjkl:");
    rows.push_back("zxcvbnm,.");

    for (int i=0; i<rows.size(); ++i) {
        QMatrix4x4 m = modelmatrix;
        m.translate(0,1.0f,1);
        m.rotate(-30.0f, 1, 0, 0);
        m.translate(-0.75f, -i*0.1f, 0);
        m.scale(0.1f, 0.4f, 1);

        if (i == 1) {
            m.translate(0.75f,0,0);
        }
        else if (i == 2) {
            m.translate(1.0f,0,0);
        }
        else if (i == 3) {
            m.translate(1.25f,0,0);
        }

        for (int j=0; j<rows[i].length(); ++j) {
            AddNewButton(VirtualMenuIndex_AVATAR, "__" + rows[i].mid(j,1), rows[i].mid(j,1), m);
            m.translate(1.05f,0,0);
        }

        if (i == 0) {
            m.translate(-0.5f,0,0);
            m.scale(2,1,1);
            m.translate(0.5f,0,0);
            AddNewButton(VirtualMenuIndex_AVATAR, "__backspace", "<--", m);
        }
        else if (i == 2) {
            m.translate(-0.5f,0,0);
            m.scale(2,1,1);
            m.translate(0.5f,0,0);
            AddNewButton(VirtualMenuIndex_AVATAR, "__enter", "Set", m);
        }
    }

    /*
    m = modelmatrix;
    m.translate(0,1.2f,1);
    m.rotate(-30.0f, 1, 0, 0);
    m.translate(-0.1f, -0.5f, 0);
    m.scale(0.6f, 0.4f, 1);
    AddNewButton(VirtualMenuIndex_SEARCH, "__space", " ", m);
    */
}

void VirtualMenu::ConstructSubmenuSocial()
{
    //qDebug() << "VirtualMenu::ConstructSubmenuSocial()";
    QVariantList & d = MathUtil::GetPartyModeData();
    num_users = d.size();

    if (cur_user > 0) {
        QMatrix4x4 m_down = modelmatrix;
        m_down.translate(-1.35f, 1.25f, 0.0f);
        m_down.scale(0.5f, 1.0f, 1.0f);
        AddNewButton(VirtualMenuIndex_SOCIAL, "__socialdown", "<-", m_down);
    }

    if (cur_user + 9 < num_users) {
        QMatrix4x4 m_up = modelmatrix;
        m_up.translate(1.35f, 1.25f, 0.0f);
        m_up.scale(0.5f, 1.0f, 1.0f);
        AddNewButton(VirtualMenuIndex_SOCIAL, "__socialup", "->", m_up);
    }

    for (int i=cur_user; i<qMin(num_users, cur_user+9); ++i) {
        int x = i % 3;
        int y = (i/3) % 3;
        QMatrix4x4 m = modelmatrix;
        m.translate(x*0.65f-0.65f, 2.0f-y*0.65f, 0);
        m.scale(0.6f, 0.6f, 1.0f);

        QMap<QString, QVariant> map = d[i].toMap();
        const QString userid = map["userId"].toString();
        const QString url = map["url"].toString();

        const QString thumb_id = "https://thumbnails.janusxr.org/" + MathUtil::MD5Hash(url) + "/thumb.jpg";
        AddNewImageUserButton(VirtualMenuIndex_SOCIAL, "__user"+QString::number(i), userid, url, thumb_id, m);
    }
}

void VirtualMenu::ConstructSubmenuKeyboard()
{
    QList <QString> rows;
    rows.push_back("`!@#$%^&*()_+");
    rows.push_back("~1234567890-=");
    rows.push_back("qwertyuiop[]\\");
    rows.push_back("asdfghjkl:'\"");
    rows.push_back("zxcvbnm,./?");

    for (int i=0; i<rows.size(); ++i) {
        QMatrix4x4 m = modelmatrix;
        m.translate(0,1.2f,1);
        m.rotate(-30.0f, 1, 0, 0);
        m.translate(-0.75f, -i*0.1f, 0);
        m.scale(0.1f, 0.4f, 1);

        if (i == 2) {
            m.translate(0.75f,0,0);
        }
        else if (i == 3) {
            m.translate(1.0f,0,0);
        }
        else if (i == 4) {
            m.translate(1.25f,0,0);
        }

        for (int j=0; j<rows[i].length(); ++j) {
            AddNewButton(VirtualMenuIndex_KEYBOARD, "__" + rows[i].mid(j,1), rows[i].mid(j,1), m);
            m.translate(1.05f,0,0);
        }

        if (i == 1) {
            m.translate(-0.5f,0,0);
            m.scale(2,1,1);
            m.translate(0.5f,0,0);
            AddNewButton(VirtualMenuIndex_KEYBOARD, "__backspace", "<--", m);
        }
        else if (i == 3) {
            m.translate(-0.5f,0,0);
            m.scale(2,1,1);
            m.translate(0.5f,0,0);
            VirtualMenuButton * b = AddNewButton(VirtualMenuIndex_KEYBOARD, "__enter", "Ent", m);
            b->button->GetProperties()->SetColour(QVector4D(0.5f,1.0f,0.5f,1.0f));
        }
    }
    QMatrix4x4 m = modelmatrix;
    m.translate(0,1.2f,1);
    m.rotate(-30.0f, 1, 0, 0);
    m.translate(-0.1f, -0.5f, 0);
    m.scale(0.6f, 0.4f, 1);
    AddNewButton(VirtualMenuIndex_KEYBOARD, "__space", " ", m);
}

void VirtualMenu::UpdatePartyModeList()
{
    //qDebug() << "VirtualMenu::UpdatePartyModeList()";
    //if not visible
    if (!MathUtil::GetPartyModeData().isEmpty()) {
        if (!visible || menu_index != VirtualMenuIndex_SOCIAL) {
            return;
        }
    }

    if (!partymode_data_request.GetStarted() || partymode_data_request.GetProcessed()) {
        partymode_data_request.Load(QUrl("https://vesta.janusxr.org/api/party_mode"));
    }
}

void VirtualMenu::ConstructSubmenuSearch()
{
    QMatrix4x4 m = modelmatrix;
    m.translate(0,1.3f,0.8f);
    m.scale(1.5f,0.8f,1);
    m.rotate(-30.0f, 1, 0, 0);
    VirtualMenuButton * b = AddNewButton(VirtualMenuIndex_SEARCH, "__enteredsearch", entered_search, m);
    b->label->GetProperties()->SetJSID("__enteredsearch_label");

    QList <QString> rows;
    rows.push_back("`!@#$%^&*()_+");
    rows.push_back("~1234567890-=");
    rows.push_back("qwertyuiop[]\\");
    rows.push_back("asdfghjkl:'\"");
    rows.push_back("zxcvbnm,./?");

    for (int i=0; i<rows.size(); ++i) {
        QMatrix4x4 m = modelmatrix;
        m.translate(0,1.2f,1);
        m.rotate(-30.0f, 1, 0, 0);
        m.translate(-0.75f, -i*0.1f, 0);
        m.scale(0.1f, 0.4f, 1);

        if (i == 2) {
            m.translate(0.75f,0,0);
        }
        else if (i == 3) {
            m.translate(1.0f,0,0);
        }
        else if (i == 4) {
            m.translate(1.25f,0,0);
        }

        for (int j=0; j<rows[i].length(); ++j) {
            AddNewButton(VirtualMenuIndex_SEARCH, "__" + rows[i].mid(j,1), rows[i].mid(j,1), m);
            m.translate(1.05f,0,0);
        }

        if (i == 1) {
            m.translate(-0.5f,0,0);
            m.scale(2,1,1);
            m.translate(0.5f,0,0);
            AddNewButton(VirtualMenuIndex_SEARCH, "__backspace", "<--", m);
        }
    }

    m = modelmatrix;
    m.translate(0,1.2f,1);
    m.rotate(-30.0f, 1, 0, 0);
    m.translate(-0.1f, -0.5f, 0);
    m.scale(0.6f, 0.4f, 1);
    AddNewButton(VirtualMenuIndex_SEARCH, "__space", " ", m);

    if (!entered_search.isEmpty()) {
        for (int i=0; i<search_data.size(); ++i) {
            int x = i % 4;
            int y = (i/4);
            QMatrix4x4 m = modelmatrix;
            m.translate(x*0.55f-0.85f, 1.6f+y*0.55f, 0);
            m.scale(0.5f, 0.5f, 1.0f);

            QMap <QString, QVariant> o = search_data[i].toMap();
            const QString url = o["roomUrl"].toString();
            const QString thumbnail = o["thumbnail"].toString();
            AddNewImageButton(VirtualMenuIndex_SEARCH, "__bookmark" + QString::number(i), url, thumbnail, m);
        }
    }
}

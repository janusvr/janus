#include "game.h"

#ifdef WIN32
QEvent::Type Game::keypress = QEvent::KeyPress;
#else
QEvent::Type Game::keypress = (QEvent::Type)6;
#endif

Game::Game() :
    do_exit(false),
    fadestate(FADE_NONE),
    gamepad_button_press(false),
    draw_cursor(false),
    cursor_active(-1),
    rmb_held(false),
    delta_time(0.0f)
{
    deltat_time.start();
    player = new Player();

    for (int i=0; i<2; ++i) {
        spatial_active[i] = false;
        undo_object[i] = QPointer <RoomObject> (new RoomObject());
        controller_x[i] = 0.0f;
        controller_y[i] = 0.0f;
        last_controller_x[i] = 0.0f;
        last_controller_y[i] = 0.0f;
    }

    bookmarks = new BookmarkManager();
    bookmarks->LoadBookmarks();
    bookmarks->LoadWorkspaces();

    controller_manager = new ControllerManager();    
    env = new Environment();
    multi_players = new MultiPlayerManager();

    virtualmenu = new VirtualMenu();
    virtualmenu->SetBookmarkManager(bookmarks);
    virtualmenu->SetMultiPlayerManager(multi_players);

    global_uuid = 0;
    state = JVR_STATE_DEFAULT;
    show_position_mode = false;

    teleport_hold_time_required = 125;

    unit_scale = 1;
    unit_translate_amount = QVector<float>({1.0f, 0.1f, 0.01f, 0.001f}); //std::initializer_lists
    unit_rotate_amount = QVector<float>({90.0f, 15.0f, 5.0f, 1.0f});
    unit_scale_amount = QVector<float>({1.0f, 0.1f, 0.01f, 0.001f});

    WebAsset::LoadAuthData();

    Initialize();
}

Game::~Game()
{
    multi_players->SetEnabled(false);

    //    Analytics::PostEvent("session", "end", NULL,  "end");
    SoundManager::StopAll();
    delete env;   

    //access filtered cubemap thing
    FilteredCubemapManager::GetSingleton()->SetShutdown(true);

    MathUtil::FlushErrorLog();

    // This needs cleaned up while Renderer is still valid
    AssetImage::null_cubemap_tex_handle = nullptr;
    AssetImage::null_image_tex_handle = nullptr;

    GeomIOSystem::SetShuttingDown(true);
}

void Game::Initialize()
{    
    env->Reset();

    multi_players->Initialize();    
    multi_players->SetEnabled(SettingsManager::GetMultiplayerEnabled());

    const float s = 0.05f;
    info_text_geom.SetFixedSize(true, s);
    info2_text_geom.SetFixedSize(true, s);

    WebAsset::SetUseCache(SettingsManager::GetCacheEnabled());

    bool cache_setting = WebAsset::GetUseCache();
    WebAsset::SetUseCache(cache_setting);

    FilteredCubemapManager* cubemap_manager = FilteredCubemapManager::GetSingleton();
    cubemap_manager->Initialize();
}

void Game::AddPrivateWebsurface()
{
    const unsigned int index = private_websurfaces.size();
    PrivateWebsurface p;

    const QString u = SettingsManager::GetWebsurfaceURL();

    p.asset = new AssetWebSurface();
    p.asset->GetProperties()->SetID("__web_id");
    p.asset->GetProperties()->SetWidth(1366);
    p.asset->GetProperties()->SetHeight(768);
    p.asset->GetProperties()->SetSaveToMarkup(false);
    p.asset->SetSrc(u, u);

    p.plane_obj = new AssetObject();
    p.plane_obj->SetSrc(MathUtil::GetApplicationURL(), "assets/primitives/plane.obj");
    p.plane_obj->Load();

    p.obj = new RoomObject();
    p.obj->SetType(TYPE_OBJECT);
    p.obj->SetInterfaceObject(true);
    p.obj->GetProperties()->SetID("plane");
    p.obj->GetProperties()->SetJSID("__plane" + QString::number(index));
    p.obj->GetProperties()->SetLighting(false);
    p.obj->GetProperties()->SetWebsurfaceID("__web_id" + QString::number(index));
    p.obj->GetProperties()->SetCullFace("none");
    p.obj->GetProperties()->SetVisible("false");
    p.obj->SetAssetObject(p.plane_obj);
    p.obj->SetAssetWebSurface(p.asset);

    private_websurfaces.push_back(p);
}

void Game::RemovePrivateWebsurface()
{
    if (!private_websurfaces.isEmpty()) {
        int ind = private_websurfaces.size()-1;
        for (int i=0; i<private_websurfaces.size(); ++i) {
            if (websurface_selected[0] == private_websurfaces[i].asset) {
                ind = i;
                break;
            }
        }

        PrivateWebsurface & p = private_websurfaces[ind];
        if (p.asset) {
            delete p.asset;
        }
        if (p.plane_obj) {
            delete p.plane_obj;
        }
        if (p.obj) {
            delete p.obj;
        }
        private_websurfaces.removeAt(ind);
    }
}

void Game::SetPrivateWebsurfacesVisible(const bool b)
{
    for (int i=0; i<private_websurfaces.size(); ++i) {
        QPointer <RoomObject> o = private_websurfaces[i].obj;
        if (o) {
            if (b) {
                QMatrix4x4 xform = player->GetTransform();
                const float angle = -(float(i%5) - float(qMin(private_websurfaces.size()-1, 4))/2.0f) * 45.0f;
                xform.rotate(angle, 0, 1, 0);
                xform.translate(0,player->GetProperties()->GetEyePoint().y()
                                - player->GetProperties()->GetPos()->toQVector3D().y() + (i/5) * 0.5f - 0.2f,-1.0f);

                o->GetProperties()->SetPos(xform.map(QVector3D(0,0,0)));
                o->GetProperties()->SetXDir(xform.mapVector(QVector3D(1,0,0)));
                o->GetProperties()->SetYDir(xform.mapVector(QVector3D(0,1,0)));
                o->GetProperties()->SetZDir(xform.mapVector(QVector3D(0,0,1)));
                o->GetProperties()->SetScale(QVector3D(0.8f, 0.45f, 1.0f));
            }
            o->GetProperties()->SetVisible(b);
        }
    }
}

bool Game::GetPrivateWebsurfacesVisible() const
{
    for (int i=0; i<private_websurfaces.size(); ++i) {
        if (private_websurfaces[i].obj && private_websurfaces[i].obj->GetProperties()->GetVisible()) {
            return true;
        }
    }
    return false;
}

void Game::UpdatePrivateWebsurfaces()
{
    for (int i=0; i<private_websurfaces.size(); ++i) {
        QPointer <AssetWebSurface> a = private_websurfaces[i].asset;
        if (a) {
            if (!a->GetStarted()) {
                a->SetStarted(true);
                a->Load();
            }
            if (a->GetWebView()) {
                a->GetWebView()->update();
            }
            a->UpdateGL();
        }
    }
}

void Game::DrawPrivateWebsurfacesGL(QPointer <AssetShader> shader)
{
    for (int i=0; i<private_websurfaces.size(); ++i) {
        if (private_websurfaces[i].asset) {
            private_websurfaces[i].asset->UpdateGL();
        }
        if (private_websurfaces[i].plane_obj) {
            private_websurfaces[i].plane_obj->Update();
            private_websurfaces[i].plane_obj->UpdateGL();
        }
        if (private_websurfaces[i].obj) {
            //qDebug() << "draw" << i;
            private_websurfaces[i].obj->Update(0.0f);
            private_websurfaces[i].obj->DrawGL(shader, true, QVector3D(0,0,0));
        }
    }
}

void Game::Update()
{
    if (env.isNull() || env->GetCurRoom().isNull()) {
        return;
    }

    //delta_t processing
    delta_time = 0.0;
    if (deltat_time.elapsed() > 0) {
        delta_time = double(deltat_time.restart()) / 1000.0;
    }
    player->SetDeltaTime(delta_time);    

    //Update imported files
    UpdateImportList();

    //Controllers (gamepad, Touch, Vive)
    UpdateControllers();   

    //Update private websurfaces
    UpdatePrivateWebsurfaces();

    //update virtual menu
    UpdateVirtualMenu();   

    //Update AssetRecordings
    UpdateAssetRecordings();

    //Update "player"
    RoomObject::SetDrawAssetObjectTeleport(state == JVR_STATE_INTERACT_TELEPORT);

    QPointer <Room> r = env->GetCurRoom();
    if (r) {
        const QPair <QVector3D, QVector3D> reset_volume = r->GetResetVolume();
        const QVector3D p = player->GetProperties()->GetPos()->toQVector3D();
        if (fadestate == FADE_NONE &&
                p.x() > qMin(reset_volume.first.x(), reset_volume.second.x()) &&
                p.y() > qMin(reset_volume.first.y(), reset_volume.second.y()) &&
                p.z() > qMin(reset_volume.first.z(), reset_volume.second.z()) &&
                p.x() < qMax(reset_volume.first.x(), reset_volume.second.x()) &&
                p.y() < qMax(reset_volume.first.y(), reset_volume.second.y()) &&
                p.z() < qMax(reset_volume.first.z(), reset_volume.second.z())) {
            StartResetPlayer();
        }

        //update player's movement in the current room
        const float walk_speed = r->GetProperties()->GetWalkSpeed();
        const float run_speed = r->GetProperties()->GetRunSpeed();
        player->Update((player->GetFlying() ||
                        player->GetRunning()) ? run_speed : walk_speed);

        //auto-load any portals?
        QHash <QString, QPointer <RoomObject> > & envobjects = r->GetRoomObjects();
        for (auto & p : envobjects) {
            if (p && p->GetType() == TYPE_LINK) {
                if (!p->GetProperties()->GetOpen() && p->GetProperties()->GetAutoLoad() && !p->GetProperties()->GetAutoLoadTriggered()) {
                    p->GetProperties()->SetOpen(true);
                    p->GetProperties()->SetAutoLoadTriggered(true);
                    env->AddRoom(p);
                }
            }
        }
    }

    //update cursors
    UpdateCursorAndTeleportTransforms();

    //update overlays
    UpdateOverlays();

    //update follow mode
    //UpdateFollowMode();

    WebAsset::SetUseCache(SettingsManager::GetCacheEnabled());

    //update VOIP
    UpdateAudio();

    //update assets
    UpdateAssets();

    //update multiplayers
    UpdateMultiplayer();

    //do environment::update1 and process portal crossings
    QPointer <Room> r0 = env->GetCurRoom();
    env->Update1(player, multi_players);
    QPointer <Room> r1 = env->GetCurRoom();

    //clear selection if player moved and hide menu
    if (r0 != r1) {
        ClearSelection(0);
        ClearSelection(1);
        SetPrivateWebsurfacesVisible(false);       
    }

    if (r1) {
        player->GetProperties()->SetURL(r1->GetProperties()->GetURL());
    }

    //63.1 - log errors as they happen
    MathUtil::FlushErrorLog();
}

float Game::UpdateCursorRaycast(const QMatrix4x4 transform, const int cursor_index)
{
    //qDebug() << "Game::UpdateCursorRaycast" << transform;
    //Ray emit position is defined by column index 3 (position)
    //Ray emit direction is defined by negation of column index 2 (z basis vector)
    const QVector3D ray_p = transform.column(3).toVector3D();
    const QVector3D ray_d = transform.column(2).toVector3D();

    struct IntersectionElement {
        QPointer <RoomObject> envobject;        
        QPointer <RoomObject> webobject;
        QPointer <RoomObject> menuobject;
        QVector3D v;
        QVector3D n;
        QVector2D uv;
    };

    QPointer <Room> r = env->GetCurRoom();
    QList <IntersectionElement> intersection_list;
    QHash <QString, QPointer <RoomObject> > & envobjects = r->GetRoomObjects();
    const bool room_overrides_teleport = r->GetProperties()->GetTeleportOverride();

    //intersect with room template
    QPointer <RoomTemplate> room_temp = r->GetRoomTemplate();
    if (!room_overrides_teleport && room_temp && room_temp->GetEnvObject()) {
        QList <QVector3D> vs;
        QList <QVector3D> ns;
        QList <QVector2D> uvs;
        if (room_temp->GetEnvObject()->GetRaycastIntersection(transform, vs, ns, uvs)) {
            for (int i=0; i<vs.size(); ++i) {
                IntersectionElement intersection_element;
                intersection_element.envobject = room_temp->GetEnvObject();
                intersection_element.v = vs[i];
                intersection_element.n = ns[i];
                intersection_element.uv = uvs[i];
                intersection_list.push_back(intersection_element);
            }
        }
    }

    //intersect with room objects (includes room portals)
    QPointer <RoomObject> select = r->GetRoomObject(selected[cursor_index]);

    for (QPointer <RoomObject> & o : envobjects) {
        //null or already selected
        if (o.isNull() || o == select) {
            continue;
        }

        //56.0 - even invisible walls should keep player in
        if (o->GetInterfaceObject() && !o->GetProperties()->GetVisible()) {
            continue;
        }

        //Room teleport overrides
        if (room_overrides_teleport && o->GetTeleportAssetObject() == 0 && o->GetType() == TYPE_OBJECT) {
            continue;
        }

        QList <QVector3D> vs;
        QList <QVector3D> ns;
        QList <QVector2D> uvs;
        o->GetRaycastIntersection(transform, vs, ns, uvs);

        for (int i=0; i<vs.size(); ++i) {
            IntersectionElement intersection_element;
            intersection_element.envobject = o;
            intersection_element.v = vs[i];
            intersection_element.n = ns[i];
            intersection_element.uv = uvs[i];
            intersection_list.push_back(intersection_element);
        }
    }

    //private websurfaces
    if (GetPrivateWebsurfacesVisible()) {
        for (int j=0; j<private_websurfaces.size(); ++j) {
            QList <QVector3D> vs;
            QList <QVector3D> ns;
            QList <QVector2D> uvs;            
            if (private_websurfaces[j].obj->GetRaycastIntersection(transform, vs, ns, uvs)) {
                for (int i=0; i<vs.size(); ++i) {
                    IntersectionElement intersection_element;
                    intersection_element.webobject = private_websurfaces[j].obj;
                    intersection_element.v = vs[i];
                    intersection_element.n = ns[i];
                    intersection_element.uv = uvs[i];
                    intersection_list.push_back(intersection_element);
                }
            }
        }
    }

    //virtual menu
    if (virtualmenu->GetVisible()) {
        QHash <QString, QPointer <RoomObject> > & os = virtualmenu->GetEnvObjects();
        for (QPointer <RoomObject> & o : os) {
            if (o) {
                QList <QVector3D> vs;
                QList <QVector3D> ns;
                QList <QVector2D> uvs;
                if (o->GetRaycastIntersection(transform, vs, ns, uvs)) {
                    for (int i=0; i<vs.size(); ++i) {
                        IntersectionElement intersection_element;
                        intersection_element.menuobject = o;
                        intersection_element.v = vs[i];
                        intersection_element.n = ns[i];
                        intersection_element.uv = uvs[i];
                        intersection_list.push_back(intersection_element);
                        //qDebug() << "menu hit" << virtualmenu->GetVisible() << o->GetProperties()->GetJSID();
                    }
                }
            }
        }
    }

    //find closest intersection point as the object for interaction
    float min_dist = FLT_MAX;
    int min_index = -1;
    for (int i=0; i<intersection_list.size(); ++i) {
        const float each_dist = (ray_p - intersection_list[i].v).length();
        if (each_dist < min_dist - 0.001f) { //56.0 - subtract some min distance from minimum, prevents "selection aliasing"
            min_index = i;
            min_dist = each_dist;
            /*if (intersection_list[i].envobject) {
                qDebug() << "new min" << min_dist << intersection_list[i].envobject->GetJSID() << intersection_list[i].envobject->GetTeleportID();
            }*/
        }
    }

    //set user's cursor based on nearest interaction point
    bool clear_websurface = true;
    bool clear_video = true;

    if (min_index >= 0) {
        player->SetCursorPos(intersection_list[min_index].v, cursor_index);
        player->SetCursorScale(player->ComputeCursorScale(cursor_index), cursor_index);

        QVector3D x(1,0,0);
        QVector3D y(0,1,0);
        QVector3D z = intersection_list[min_index].n;

        if (QVector3D::dotProduct(ray_d, z) > 0.0f) {
            z = -z;
        }

        if (fabsf(QVector3D::dotProduct(y, z)) > 0.9f) {
            x = player->GetRightDir();
            y = QVector3D::crossProduct(z, x).normalized();
            x = QVector3D::crossProduct(y, z).normalized();
        }
        else {
            x = QVector3D::crossProduct(y, z).normalized();
            y = QVector3D::crossProduct(z, x).normalized();
        }

        player->SetCursorXDir(x, cursor_index);
        player->SetCursorYDir(y, cursor_index);
        player->SetCursorZDir(z, cursor_index);

        if (intersection_list[min_index].envobject) {
            player->SetCursorObject(intersection_list[min_index].envobject->GetProperties()->GetJSID(), cursor_index);

            const QPointer <AssetWebSurface> web = intersection_list[min_index].envobject->GetAssetWebSurface();
            const QPointer <AssetVideo> vid = intersection_list[min_index].envobject->GetAssetVideo();
            if (web) {
                //57.1 - select websurfaces only on hover
                if (controller_manager->GetUsingSpatiallyTrackedControllers() && !controller_manager->GetStates()[cursor_index].GetClick().hover) {

                }
                else {
                    //websurface_selected[cursor_index] = web;
                }
                clear_websurface = false;
            }
            if (vid) {
                video_selected[cursor_index] = vid;
                clear_video = false;
            }
        }        
        else if (intersection_list[min_index].menuobject) {
            player->SetCursorObject(intersection_list[min_index].menuobject->GetProperties()->GetJSID(), cursor_index);
        }
        else if (intersection_list[min_index].webobject) {
            player->SetCursorObject(intersection_list[min_index].webobject->GetProperties()->GetJSID(), cursor_index);
            //websurface_selected[cursor_index] = intersection_list[min_index].webobject->GetAssetWebSurface();
            clear_websurface = false;
        }

        player->SetCursorActive(true, cursor_index);

        cursor_uv[cursor_index] = QPointF(intersection_list[min_index].uv.x(), intersection_list[min_index].uv.y());
    }
    else {
        player->SetCursorObject("", cursor_index);
        player->SetCursorPos(player->GetProperties()->GetEyePoint() + player->GetProperties()->GetViewDir()->toQVector3D(), cursor_index);
        player->SetCursorXDir(QVector3D::crossProduct(player->GetProperties()->GetUpDir()->toQVector3D(),
                                                      -player->GetProperties()->GetViewDir()->toQVector3D()).normalized(), cursor_index);
        player->SetCursorYDir(player->GetProperties()->GetUpDir()->toQVector3D(), cursor_index);
        player->SetCursorZDir(-player->GetProperties()->GetViewDir()->toQVector3D(), cursor_index);
        player->SetCursorScale(0.0f, cursor_index);
        player->SetCursorActive(false, cursor_index);

        cursor_uv[cursor_index] = QPointF(0,0);
    }

    if (clear_websurface) {
        websurface_selected[cursor_index].clear();
    }
    if (clear_video) {
        video_selected[cursor_index].clear();
    }

    //qDebug() << "Game::UpdateCursorRaycast" << ray_p << ray_d << intersection_list.size() << player->GetCursorPos(0);
    return min_dist;
}

void Game::DrawFadingGL()
{
    QPointer <AssetShader> shader = Room::GetTransparencyShader();
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }

    if (fadestate == FADE_NONE) {
        return;
    }

    Renderer * renderer = Renderer::m_pimpl;
    renderer->SetDefaultFaceCullMode(FaceCullMode::DISABLED);
    renderer->SetStencilOp(StencilOp(StencilOpAction::KEEP, StencilOpAction::KEEP, StencilOpAction::KEEP));
    renderer->SetStencilFunc(StencilFunc(StencilTestFuncion::ALWAYS, StencilReferenceValue(0), StencilMask(0xffffffff)));
    renderer->SetDepthFunc(DepthFunc::ALWAYS);

    shader->SetUseTextureAll(false);
    shader->SetUseClipPlane(false);
    shader->SetFogEnabled(false);

    const QVector3D v = player->GetProperties()->GetEyePoint();
    const float s = (GetCurrentNearDist()+GetCurrentFarDist())*0.5f; //60.0 - cube needs to fit within clip planes

    MathUtil::PushModelMatrix();
    MathUtil::ModelMatrix().translate(v);
    MathUtil::ModelMatrix().scale(s);

    switch (fadestate) {
    case FADE_NONE:
        break;

    case FADE_RELOAD1:
    case FADE_RELOAD2:
    case FADE_RESETPLAYER1:
    case FADE_RESETPLAYER2:
    case FADE_FORWARD_PLAYER1:
    case FADE_FORWARD_PLAYER2:
    case FADE_TELEPORT1:
    case FADE_TELEPORT2:    
    {
        const float duration = (fadestate == FADE_TELEPORT1 || fadestate == FADE_TELEPORT2) ? 250.0f : 1000.0f;
        const float alpha = (fadestate == FADE_RELOAD1 ||
                             fadestate == FADE_RESETPLAYER1 ||
                             fadestate == FADE_FORWARD_PLAYER1 ||
                             fadestate == FADE_TELEPORT1) ? (float(fade_time.elapsed()) / duration) : 1.0f - (float(fade_time.elapsed()) / duration);

        shader->SetAmbient(QVector3D(1,1,1));
        shader->SetDiffuse(QVector3D(1,1,1));
        shader->SetEmission(QVector3D(0,0,0));        
        shader->SetConstColour(QVector4D(0,0,0,alpha));
        shader->SetUseTextureAll(false);
        shader->SetUseLighting(false);
        shader->UpdateObjectUniforms();

        Renderer * renderer = Renderer::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                renderer->GetTexturedCube2PrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetTexturedCube2VAO(),
                                shader->GetProgramHandle(),
                                shader->GetFrameUniforms(),
                                shader->GetRoomUniforms(),
                                shader->GetObjectUniforms(),
                                shader->GetMaterialUniforms(),
                                renderer->GetCurrentlyBoundTextures(),
                                renderer->GetDefaultFaceCullMode(),
                                renderer->GetDepthFunc(),
                                renderer->GetDepthMask(),
                                renderer->GetStencilFunc(),
                                renderer->GetStencilOp(),
                                renderer->GetColorMask());
        renderer->PushAbstractRenderCommand(a);

        if (fade_time.elapsed() >= duration) {
            switch (fadestate) {
            case FADE_RELOAD1:
                fade_time.start();
                fadestate = FADE_RELOAD2;
                env->ReloadRoom();
                break;
            case FADE_RELOAD2:
                fadestate = FADE_NONE;
                break;
            case FADE_RESETPLAYER1:
            {
                fade_time.start();
                fadestate = FADE_RESETPLAYER2;

                if (env->GetCurRoom() && env->GetCurRoom()->GetParent()) {
                    env->NavigateToRoom(player, env->GetCurRoom()->GetParent());
                }
                else {
                    ResetPlayer();
                }
            }
                break;
            case FADE_RESETPLAYER2:
                fadestate = FADE_NONE;
                break;
            case FADE_FORWARD_PLAYER1:
            {
                fade_time.start();
                fadestate = FADE_FORWARD_PLAYER2;

                bool moved_forward = false;
                if (env->GetCurRoom()) {
                    //navigate "forward" to last of current room's child rooms
                    QList <QPointer <Room> > & children = env->GetCurRoom()->GetChildren();
                    if (!children.isEmpty() && children.last()) {
                        env->NavigateToRoom(player, children.last());
                        moved_forward = true;
                    }
                }

                if (!moved_forward) {
                    ResetPlayer();
                }
            }
                break;
            case FADE_FORWARD_PLAYER2:
                fadestate = FADE_NONE;
                break;
            case FADE_TELEPORT1:
                fade_time.start();
                fadestate = FADE_TELEPORT2;
                TeleportPlayer();
                break;
            case FADE_TELEPORT2:
                fadestate = FADE_NONE;
                break;            
            default:
                break;
            }
        }

    }
        break;
    }

    shader->SetConstColour(QVector4D(1,1,1,1));
    renderer->SetDefaultFaceCullMode(FaceCullMode::BACK);
    renderer->SetDepthFunc(DepthFunc::LEQUAL);

    MathUtil::PopModelMatrix();
}

QMatrix4x4 Game::GetMouseCursorTransform() const
{
    return mouse_cursor_transform;
}

void Game::ComputeMouseCursorTransform(const QSize p_windowSize, const QPointF p_mousePos)
{
    /*
    qDebug() << "Game::ComputeMouseCursorTransform" << p_mousePos << p_windowSize;
    qDebug() << "model" << MathUtil::ModelMatrix();
    qDebug() << "view" << MathUtil::ViewMatrix();
    qDebug() << "Game::ComputeMouseCursorTransform" << (MathUtil::ModelMatrix() * MathUtil::ViewMatrix()).column(2) << (MathUtil::ModelMatrix() * MathUtil::ViewMatrix()).column(3);
    */
    GLdouble modelMatrix[16];
    GLdouble projMatrix[16];
    GLint viewport[4];

    QMatrix4x4 persp;

    QPointer <Room> r = env->GetCurRoom();
    const float near_dist = r->GetProperties()->GetNearDist();
    const float far_dist = r->GetProperties()->GetFarDist();

    if (player->GetHMDEnabled()) { //54.10 hack: forces cursor to remain at centre (look to click)
        persp.perspective(0.01f, float(p_windowSize.width()) / float(p_windowSize.height()), near_dist, far_dist);
    }
    else {
        persp.perspective(SettingsManager::GetFOV(), float(p_windowSize.width()) / float(p_windowSize.height()), near_dist, far_dist);
    }

    for (uint32_t i = 0; i < 16; ++i)
    {
        modelMatrix[i] = (MathUtil::ModelMatrix() * MathUtil::ViewMatrix()).constData()[i];
        projMatrix[i] = persp.constData()[i]; //52.11 - bugfix for skewed projection matrices, make ray come out from centre
    }

    viewport[0] = 0;
    viewport[1] = 0;
    viewport[2] = p_windowSize.width();
    viewport[3] = p_windowSize.height();

    GLdouble new_x, new_y, new_z;
    MathUtil::UnProject(p_mousePos.x(), p_mousePos.y(), 0.9999f, modelMatrix, projMatrix, viewport, &new_x, &new_y, &new_z);

    const QVector3D z = (QVector3D(new_x, new_y, new_z) - player->GetProperties()->GetEyePoint()).normalized();
    const QVector3D x = QVector3D::crossProduct(QVector3D(0,1,0), z).normalized();
    const QVector3D y = QVector3D::crossProduct(x, z).normalized();

    QMatrix4x4 m;
    m.setColumn(0, x);
    m.setColumn(1, y);
    m.setColumn(2, z);
    m.setColumn(3, player->GetProperties()->GetEyePoint());
    m.setRow(3, QVector4D(0,0,0,1));

    //qDebug() << "Game::ComputeMouseCursorTransform()" << cursor_win << new_x << new_y << new_z << m;

    mouse_cursor_transform = m;
}

void Game::DrawGL(const float ipd, const QMatrix4x4 head_xform, const bool set_modelmatrix, const QSize p_windowSize, const QPointF p_mousePos)
{    
    QPointer <AssetShader> trans_shader = Room::GetTransparencyShader();
    if (trans_shader == NULL || !trans_shader->GetCompiled()) {
        return;
    }

    //setup camera
    if (!set_modelmatrix) {
        MathUtil::PushModelMatrix();
        player->SetViewGL(true, ipd, head_xform);
        MathUtil::PopModelMatrix();
    }
    else {
        player->SetViewGL(true, ipd, head_xform);
    }

    //54.10 - move update call here, because some player position stuff has not updated, and this causes JS set attribs to jump around
    env->Update2(player, multi_players);

    if (!controller_manager->GetUsingSpatiallyTrackedControllers()) {
        ComputeMouseCursorTransform(p_windowSize, p_mousePos);
        UpdateCursorRaycast(GetMouseCursorTransform(), 0);
    }

    // Draw current room
    env->draw_current_room(multi_players, player, true);

    // TODO: This is temporaily disabled while I implement the new system using Bullet Physics
    /*
        if (show_position_mode && show_collision_volumes)
        {
            trans_shader->BindShaderGL();
            env->GetPlayerRoom()->DrawCollisionModelGL(trans_shader);
            trans_shader->UnbindShaderGL();
        }*/


    // Update the cursor (Object ID, normal, world-space location)
    QPointer <Room> r = env->GetCurRoom();
    Renderer::m_pimpl->BeginScope(RENDERER::RENDER_SCOPE::VIRTUAL_MENU);
    r->BindShader(Room::GetTransparencyShader());
    DrawVirtualMenu();    
    r->UnbindShader(Room::GetTransparencyShader());
    Renderer::m_pimpl->EndCurrentScope();

    // Draw child rooms
    env->draw_child_rooms(multi_players, player, true); 

    // Always update player avatar's head transform
    QPointer <RoomObject> player_avatar = multi_players->GetPlayer();
    if (player_avatar) {
        player_avatar->GetProperties()->SetViewDir(player->GetProperties()->GetViewDir());
        player_avatar->GetProperties()->SetUpDir(player->GetProperties()->GetUpDir());
    }

    //draw player avatar (if enabled)
    Renderer::m_pimpl->BeginScope(RENDERER::RENDER_SCOPE::AVATARS);
    if (SettingsManager::GetSelfAvatar()) {

        QPointer <AssetShader> shader = Room::GetTransparencyShader();
        if (r->GetAssetShader()) {
            shader = r->GetAssetShader();
        }

        r->BindShader(shader);

        QPointer <RoomObject> player_avatar = multi_players->GetPlayer();

        GhostFrame frame0;
        GhostFrame frame1;

        frame1.time_sec = 1.0f;
        frame1.pos = player->GetProperties()->GetPos()->toQVector3D();
        frame1.dir = player->GetProperties()->GetDir()->toQVector3D();
        frame1.dir.setY(0.0f);
        frame1.dir.normalize();
        frame1.SetHeadXForm(player->GetProperties()->GetUpDir()->toQVector3D(),
                            player->GetProperties()->GetViewDir()->toQVector3D());
        frame1.hands.first = player->GetHand(0);
        frame1.hands.second = player->GetHand(1);

        player_avatar->SetHMDType(player->GetHMDType());
        player_avatar->GetProperties()->SetPos(player->GetProperties()->GetPos()->toQVector3D());

        // set first and last frames to be the same (current packet)
        frame0 = frame1;
        frame0.time_sec = 0.0f;

        QVector <GhostFrame> frames;
        frames.push_back(frame0);
        frames.push_back(frame1);
        player_avatar->GetAssetGhost()->SetFromFrames(frames, 1000);

        // ghost needs to be processed        
        player_avatar->Update(player->GetDeltaTime());        
        player_avatar->DrawGL(shader, true, player->GetProperties()->GetPos()->toQVector3D());        

        r->UnbindShader(shader);
    }
    Renderer::m_pimpl->EndCurrentScope();

    //draw controllers (vive, oculus touch, Leap Motion, etc.)
    Renderer::m_pimpl->BeginScope(RENDERER::RENDER_SCOPE::CONTROLLERS);
    if (controller_manager) {
        QPointer <AssetShader> shader = Room::GetTransparencyShader();
        if (r->GetAssetShader()) {
            shader = r->GetAssetShader();
        }

        r->BindShader(shader);
        controller_manager->DrawGL(shader, player->GetTransform());
        r->UnbindShader(shader);
    }
    Renderer::m_pimpl->EndCurrentScope();

    // Draw menu before cursor update if it is selected
    Renderer::m_pimpl->BeginScope(RENDERER::RENDER_SCOPE::CURSOR);
    DrawCursorGL();
    Renderer::m_pimpl->EndCurrentScope();

    Renderer::m_pimpl->BeginScope(RENDERER::RENDER_SCOPE::OVERLAYS);
    DrawOverlaysGL();
    DrawFadingGL();
    Renderer::m_pimpl->EndCurrentScope();

    // Recomputes the AABB's for all RoomObjects and adds them to a fresh instance of
    // the rooms physics world then draws it
    /*if (show_collision_volumes)
    {
        env->GetPlayerRoom()->ResetPhysicsWorld();
    }*/
}

void Game::initializeGL()
{
    //cursor stuff
    state = JVR_STATE_DEFAULT;

    AssetImage::initializeGL();
    TextGeom::initializeGL();
    RoomObject::initializeGL();
    SpinAnimation::initializeGL();
    Room::initializeGL();

    fade_time.start();
}

void Game::mouseMoveEvent(QMouseEvent * e, const float x, const float y, const int cursor_index, const QSize , const QPointF )
{
    cursor_win += QPointF(x, y);
    mouse_move_accum += QPointF(x, y) * 0.05f;

    const bool left_btn = ((e->buttons() & Qt::LeftButton) > 0);

    QPointer <Room> r = env->GetCurRoom();
    QPointer <RoomObject> selected_obj = r->GetRoomObject(selected[cursor_index]);
    QPointer <RoomObject> cursor_obj = r->GetRoomObject(player->GetCursorObject(cursor_index));

    switch (state) {
    case JVR_STATE_DEFAULT:
    case JVR_STATE_INTERACT_TELEPORT:
    case JVR_STATE_DRAGDROP:

        //release 60.0 - spin/tilt view BEFORE passing mouse move event to websurface
        player->SpinView(x * 0.1f, true);
        if (GetMouseDoPitch()) {
            const float tilt_amount = y * 0.1f * (SettingsManager::GetInvertYEnabled() ? 1.0f : -1.0f);
            player->TiltView(tilt_amount);
        }

        r->CallJSFunction("room.onMouseMove", player, multi_players);
        if (left_btn) {
            r->CallJSFunction("room.onMouseDrag", player, multi_players);
        }

        //qDebug() << "Game::mouseMoveEvent" << selected[cursor_index] << envobjects.contains(selected[cursor_index]);
        if (state == JVR_STATE_DRAGDROP) {
            UpdateDragAndDropPosition(selected_obj, cursor_index);
        }

        if (websurface_selected[cursor_index]){
            const QPoint cursor_pos(float(websurface_selected[cursor_index]->GetProperties()->GetWidth())*cursor_uv[cursor_index].x(),
                                    float(websurface_selected[cursor_index]->GetProperties()->GetHeight())*cursor_uv[cursor_index].y());
            //Release 60.0 - note: player spin/tilt has to happen before this, as this method does not seem to contnue after this call
            QMouseEvent e2(QEvent::MouseMove, cursor_pos, e->button(), e->buttons(), e->modifiers());
            websurface_selected[cursor_index]->mouseMoveEvent(&e2, cursor_index);

            QString url_str = websurface_selected[cursor_index]->GetLinkClicked(cursor_index).toString().trimmed();
            if (state != JVR_STATE_DRAGDROP && !(url_str == websurface_selected[cursor_index]->GetURL() || url_str == "") && ((e->buttons() & Qt::RightButton) > 0) && rmb_held_time.elapsed() > 100) {
                DragAndDropFromWebsurface(keys[Qt::Key_Control]?"Drag+Drop":"Drag+Pin", cursor_index);
                //websurface_selected[cursor_index].clear();
            }
        }
        else if (cursor_obj) {

            //qDebug() << player->GetCursorObject(cursor_index) << o->GetJSID() << o->GetOriginalURL() << o->GetWebSurfaceID() << o->GetUUID();
            if (websurface_selected[cursor_index] && cursor_obj->GetType() == TYPE_OBJECT && cursor_obj->GetAssetWebSurface()) {
                /*
                if websurface is still on about:blank, load it
                if (websurface_selected[cursor_index]->GetURL() == "about:blank") {
                    websurface_selected[cursor_index]->SetURL(websurface_selected[cursor_index]->GetS("src"));
                }
                */

                //52.8 - disable mousemove on websurfaces to prevent native drag and drop operation
                QPoint cursor_pos(float(websurface_selected[cursor_index]->GetProperties()->GetWidth())*cursor_uv[cursor_index].x(),
                                  float(websurface_selected[cursor_index]->GetProperties()->GetHeight())*cursor_uv[cursor_index].y());

                QMouseEvent e2(QEvent::MouseMove, cursor_pos, e->button(), e->buttons(), Qt::NoModifier);
                websurface_selected[cursor_index]->mouseMoveEvent(&e2, cursor_index);
            }
            else if (video_selected[cursor_index] && (cursor_obj->GetType() == TYPE_VIDEO || cursor_obj->GetType() == TYPE_OBJECT) && cursor_obj->GetAssetVideo()) {
                if (left_btn) {
                    QPoint cursor_pos(float(video_selected[cursor_index]->GetWidth(cursor_obj->GetMediaContext()))*cursor_uv[cursor_index].x(),
                                      float(video_selected[cursor_index]->GetHeight(cursor_obj->GetMediaContext()))*cursor_uv[cursor_index].y());
                    QMouseEvent e2(QEvent::MouseMove, cursor_pos, e->button(), e->buttons(), Qt::NoModifier);
                    video_selected[cursor_index]->mouseMoveEvent(cursor_obj->GetMediaContext(), &e2);
                }
            }
        }

        break;

    case JVR_STATE_UNIT_TRANSLATE:
        if (selected_obj) {
            if (fabsf(mouse_move_accum.x()) > 1.0f) {
                EditModeTranslate(selected_obj,mouse_move_accum.x(),0,0);
                mouse_move_accum.setX(0.0f);
            }
            if (fabsf(mouse_move_accum.y()) > 1.0f) {
                EditModeTranslate(selected_obj,0, mouse_move_accum.y(),0);
                mouse_move_accum.setY(0.0f);
            }
        }
        break;
    case JVR_STATE_UNIT_ROTATE:
        if (selected_obj) {
            if (fabsf(mouse_move_accum.x()) > 1.0f) {
                EditModeRotate(selected_obj,mouse_move_accum.x(),0,0);
                mouse_move_accum.setX(0.0f);
            }
            if (fabsf(mouse_move_accum.y()) > 1.0f) {
                EditModeRotate(selected_obj,0, mouse_move_accum.y(),0);
                mouse_move_accum.setY(0.0f);
            }
        }
        break;
    case JVR_STATE_UNIT_SCALE:
        if (selected_obj) {
            if (fabsf(mouse_move_accum.x()) > 1.0f) {
                EditModeScale(selected_obj,mouse_move_accum.x(),0,0);
                mouse_move_accum.setX(0.0f);
            }
            if (fabsf(mouse_move_accum.y()) > 1.0f) {
                EditModeScale(selected_obj,0, mouse_move_accum.y(),0);
                mouse_move_accum.setY(0.0f);
            }
        }
        break;
    case JVR_STATE_UNIT_COLLISIONSCALE:
        if (selected_obj) {
            if (fabsf(mouse_move_accum.x()) > 1.0f) {
                EditModeCollisionScale(selected_obj,mouse_move_accum.x(),0,0);
                mouse_move_accum.setX(0.0f);
            }
            if (fabsf(mouse_move_accum.y()) > 1.0f) {
                EditModeCollisionScale(selected_obj,0, mouse_move_accum.y(),0);
                mouse_move_accum.setY(0.0f);
            }
        }
        break;

    default:
        break;
    }
}

void Game::mousePressEvent(QMouseEvent * e, const int cursor_index, const QSize , const QPointF )
{
    if (e->button() == Qt::RightButton) {
        rmb_held = true;
        rmb_held_time.start();
    }

    mouse_move_accum = QPointF(0,0);

    QPointer <Room> r = env->GetCurRoom();

    switch (state) {
    case JVR_STATE_DEFAULT:
        if (e->button() == Qt::LeftButton) {
            r->CallJSFunction("room.onMouseDown", player, multi_players);

            if (r->GetProperties()->GetCursorVisible()) {
                SoundManager::Play(SOUND_CLICK1, false, player->GetCursorPos(cursor_index), 1.0f);
            }

            StartOpInteractionDefault(cursor_index);
        }
        else if (e->button() == Qt::MiddleButton) {
            QPointer <RoomObject> o = r->GetRoomObject(player->GetCursorObject(cursor_index));
            if (o && o->GetType() == TYPE_OBJECT && o->GetAssetWebSurface()) {
                websurface_selected[cursor_index] = o->GetAssetWebSurface();
                QPoint cursor_pos(float(websurface_selected[cursor_index]->GetProperties()->GetWidth())*cursor_uv[cursor_index].x(),
                                  float(websurface_selected[cursor_index]->GetProperties()->GetHeight())*cursor_uv[cursor_index].y());
                QMouseEvent e2(QEvent::MouseButtonPress, cursor_pos, Qt::MiddleButton, Qt::MiddleButton, Qt::NoModifier);
                websurface_selected[cursor_index]->mousePressEvent(&e2, cursor_index);
            }
            if (!controller_manager->GetUsingSpatiallyTrackedControllers() && GetAllowTeleport(cursor_index)) {
                StartOpInteractionTeleport(cursor_index);
            }
        }
        else if (e->button() == Qt::RightButton) {
            QPointer <RoomObject> o = r->GetRoomObject(player->GetCursorObject(cursor_index));
            if (o && o->GetType() == TYPE_OBJECT && o->GetAssetWebSurface()) {
                websurface_selected[cursor_index] = o->GetAssetWebSurface();
                QPoint cursor_pos(float(websurface_selected[cursor_index]->GetProperties()->GetWidth())*cursor_uv[cursor_index].x(),
                                  float(websurface_selected[cursor_index]->GetProperties()->GetHeight())*cursor_uv[cursor_index].y());
                QMouseEvent e2(QEvent::MouseButtonPress, cursor_pos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
                websurface_selected[cursor_index]->mousePressEvent(&e2, cursor_index);
            }
        }

        break;

    default:
        break;
    }
}

void Game::wheelEvent(QWheelEvent * e)
{
    QPointer <Room> r = env->GetCurRoom();
    QPointer <RoomObject> o = r->GetRoomObject(selected[0]);

    const int delta = ((e->delta() > 0) ? 1 : -1);

    switch (state) {
    case JVR_STATE_DEFAULT:
        if (websurface_selected[0]) {
            websurface_selected[0]->wheelEvent(e);
        }

        break;

    case JVR_STATE_SELECT_ASSET:
        if (o) {
            r->SelectAssetForObject(selected[0], (e->delta() > 0) ? 1 : -1);
            o->PlayCreateObject();
            o->GetProperties()->SetSync(true);
        }
        break;

    case JVR_STATE_UNIT_TRANSLATE:
        if (o) {
            EditModeTranslate(o,0,0,delta);
        }
        break;
    case JVR_STATE_UNIT_ROTATE:
        if (o) {
            EditModeRotate(o,0,0,(delta > 0) ? 1 : -1);
        }
        break;
    case JVR_STATE_UNIT_SCALE:
        if (o) {
            EditModeScale(o,0,0,delta);
        }
        break;
    case JVR_STATE_UNIT_COLLISIONSCALE:
        if (o) {
            EditModeCollisionScale(o,0,0,delta);
        }
        break;
    default:
        break;
    }
}

bool Game::GetAllowTeleport(const int cursor_index)
{
    const QString cur_obj = player->GetCursorObject(cursor_index);
    QPointer <Room> r = env->GetCurRoom();
    QPointer <RoomObject> o = r->GetRoomObject(cur_obj);

    //59.4 - cursor becomes inactive on release
    if (fadestate != FADE_NONE) {
        return false;
    }

    //teleport destination in view frustum test with spatial controllers
    /*
    if (controller_manager->GetUsingSpatiallyTrackedControllers()) {
        const QVector3D cz = -player->GetTransform().mapVector(controller_manager->GetStates()[cursor_index].xform.column(2).toVector3D());
        if (QVector3D::dotProduct(player->GetV("view_dir"), cz) < 0.717f) {
            return false;
        }
    }
    */

    const float cursor_dist = (player->GetProperties()->GetPos()->toQVector3D() - player->GetCursorPos(cursor_index)).length();
    if (cursor_dist > r->GetProperties()->GetTeleportMaxDist()) {
        return false;
    }
    if (cursor_dist < r->GetProperties()->GetTeleportMinDist()) {
        return false;
    }
    if (websurface_selected[cursor_index]) {
        return false;
    }
    if (o && o->GetType() != TYPE_LINK && o->GetTeleportAssetObject() == 0 && r->GetProperties()->GetTeleportOverride()) {
        return false;
    }
    if (o && o->GetType() == TYPE_LINK && (!o->GetProperties()->GetActive() || !o->GetProperties()->GetOpen())) {
        return false;
    }
    if (spatial_active[cursor_index]) {
        return false;
    }

    return true;
}

void Game::mouseReleaseEvent(QMouseEvent * e, const int cursor_index, const QSize , const QPointF )
{
    const bool left_btn = (e->button() == Qt::LeftButton);
    if (e->button() == Qt::RightButton) {
        rmb_held = false;
    }
    teleport_portal = NULL;

    QPointer <Room> r = env->GetCurRoom();
    QPointer <RoomObject> o = r->GetRoomObject(selected[cursor_index]);

    if (r->GetProperties()->GetCursorVisible()) {
        SoundManager::Play(SOUND_CLICK2, false, player->GetCursorPos(cursor_index), 1.0f);
    }

    if (spatial_active[cursor_index]) {
        return;
    }

    switch (state) {
    case JVR_STATE_DEFAULT:
    {
        if (e->button() == Qt::LeftButton) {

            r->CallJSFunction("room.onMouseUp", player, multi_players);
            r->CallJSFunction("room.onClick", player, multi_players);

            const int i = cursor_index;
            if (websurface_selected[i]){
                websurface_selected[i]->SetFocus(true);
            }

            EndOpInteractionDefault(cursor_index);
        }
        else if (e->button() == Qt::RightButton) {

            //qDebug() << "new_selected" << new_selected << "selected" << selected << b << g << r;
            //Only allow room edits (object creation or selection), if the room is not globally locked
            mouse_move_accum = QPointF(0,0);

            if (r->GetProperties()->GetLocked()) {
                //59.6 - send out error message this is not supported
                MathUtil::ErrorLog("Warning: cannot do edit, room.locked=true");
            }
            else if (!r->GetProperties()->GetLocked() && SettingsManager::GetEditModeEnabled())
            {
                if (!(websurface_selected[cursor_index] && rmb_held_time.elapsed() > 500)) {
                    if (player->GetCursorObject(cursor_index) == selected[cursor_index]) {
                        ClearSelection(cursor_index);
                    }
                    else {
                        selected[cursor_index] = player->GetCursorObject(cursor_index);
                    }

                    o = r->GetRoomObject(selected[cursor_index]);
                    const bool did_select = r->SetSelected(selected[cursor_index], true);
                    if (o && did_select) {
                        // If there's an object under the cursor, and it is now selected, begin manipulating it
                        if (undo_object[cursor_index]) {
                            delete undo_object[cursor_index];
                        }
                        undo_object[cursor_index] = new RoomObject();
                        undo_object[cursor_index]->Copy(o);
                        undo_object[cursor_index]->GetProperties()->SetJSID(o->GetProperties()->GetJSID());
                        state = JVR_STATE_UNIT_TRANSLATE; //or some manipulation state we should have
                    }
                    else {
                        ClearSelection(cursor_index);

                        //59.9 - ensure cursor position is at least 2 meters away (if either too close or too far)
                        QVector3D diff_vec = player->GetCursorPos(cursor_index) - player->GetProperties()->GetEyePoint();
                        const float len = diff_vec.length();
                        if (len > 100.0f || len <= 1.0f) {
                            player->SetCursorPos(player->GetProperties()->GetEyePoint() + diff_vec * 2.0f / len, cursor_index);
                        }

                        if (keys[Qt::Key_Shift]) {
                            selected[cursor_index] = r->AddText_Interactive(player->GetCursorPos(cursor_index), player->GetCursorXDir(cursor_index), player->GetCursorYDir(cursor_index), player->GetCursorZDir(cursor_index), GetGlobalUUID());
                            o = r->GetRoomObject(selected[cursor_index]);
                            if (o) {
                                r->SetSelected(selected[cursor_index], true);
                                state = JVR_STATE_EDIT_TEXT;
                                SoundManager::Play(SOUND_NEWTEXT, false, o->GetPos(), 1.0f);
                                o->GetProperties()->SetSync(true);
                            }
                        }
                        else if (keys[Qt::Key_Control]) {
                            selected[cursor_index] = r->AddImage_Interactive(player->GetCursorPos(cursor_index), player->GetCursorXDir(cursor_index), player->GetCursorYDir(cursor_index), player->GetCursorZDir(cursor_index), GetGlobalUUID());
                            o = r->GetRoomObject(selected[cursor_index]);
                            if (o) {
                                r->SetSelected(selected[cursor_index], true);
                                r->SelectAssetForObject(selected[cursor_index], 0);
                                state = JVR_STATE_SELECT_ASSET;
                                o->PlayCreateObject();
                                o->GetProperties()->SetSync(true);
                            }
                        }
                        else {
                            selected[cursor_index] = r->AddObject_Interactive(player->GetCursorPos(cursor_index), player->GetCursorXDir(cursor_index), player->GetCursorZDir(cursor_index), -player->GetCursorYDir(cursor_index), GetGlobalUUID());
                            o = r->GetRoomObject(selected[cursor_index]);
                            if (o) {
                                r->SetSelected(selected[cursor_index], true);
                                r->SelectAssetForObject(selected[cursor_index], 0);
                                state = JVR_STATE_SELECT_ASSET;
                                o->PlayCreateObject();
                                o->GetProperties()->SetSync(true);
                            }
                        }
                    }
                }
            }
        }
        else if (e->button() == Qt::MiddleButton) {
            if (websurface_selected[cursor_index]) {
                QString url_str = websurface_selected[cursor_index]->GetLinkClicked(cursor_index).toString().trimmed();

                if (!(url_str == websurface_selected[cursor_index]->GetURL() || url_str == "")) {
                    //remove stuff after ? (59.0 - but not if it's a Google link - contains /url?q=)(62.9 - versioning in Vesta)
                    if (url_str.contains("?") && !url_str.contains("/url?q=") && !url_str.contains("?v=")) {
                        url_str = url_str.left(url_str.indexOf("?"));
                    }

                    CreatePortal(url_str, true);
                    return;
                }
            }
        }
    }
        break;

    case JVR_STATE_INTERACT_TELEPORT:
        if (e->button() == Qt::MiddleButton) {
            EndOpInteractionTeleport(cursor_index);
            state = JVR_STATE_DEFAULT;
        }
        break;

    case JVR_STATE_DRAGDROP:
        /*if (e->button() == Qt::RightButton) {
            r->DeleteSelected(selected[cursor_index]);
        }*/
        state = JVR_STATE_DEFAULT;
        ClearSelection(cursor_index);
        break;

    case JVR_STATE_UNIT_TRANSLATE:
    case JVR_STATE_UNIT_ROTATE:
    case JVR_STATE_UNIT_SCALE:
    case JVR_STATE_UNIT_COLOUR:
    case JVR_STATE_UNIT_COLLISIONID:
    case JVR_STATE_UNIT_COLLISIONSCALE:
    case JVR_STATE_UNIT_LIGHTING:
    case JVR_STATE_UNIT_CULL_FACE:
    case JVR_STATE_UNIT_BLEND_SRC:
    case JVR_STATE_UNIT_BLEND_DEST:
    case JVR_STATE_UNIT_MIRROR:
        if (!left_btn && undo_object[cursor_index] && undo_object[cursor_index]->GetProperties()->GetJSID().length() > 0 && o) {
            o->Copy(undo_object[cursor_index]);
            //update others we are undoing
            o->GetProperties()->SetSync(true);
        }

        r->SetSelected(selected[cursor_index], false);
        ClearSelection(cursor_index);
        state = JVR_STATE_DEFAULT;
        break;

    case JVR_STATE_SELECT_ASSET:
    case JVR_STATE_EDIT_TEXT:
        if (!left_btn) {
            r->DeleteSelected(selected[cursor_index]);
        }
        r->SetSelected(selected[cursor_index], false);
        ClearSelection(cursor_index);
        state = JVR_STATE_DEFAULT;
        break;

    default:
        r->SetSelected(selected[cursor_index], false);
        if (!left_btn) {
            multi_players->SetRoomDeleteCode(r->GetSelectedCode(selected[cursor_index]));
            r->DeleteSelected(selected[cursor_index]);
        }

        ClearSelection(cursor_index);
        break;
    }

}

void Game::EndKeyboardFocus()
{
    //59.6 - "release" all keys (prevents e.g stuck jumping when pressing T)
    QMap<int, bool>::iterator it;
    for (it=keys.begin(); it!=keys.end(); ++it) {
        it.value() = false;
    }
}

bool Game::GetPlayerEnteringText()
{
    return state == JVR_STATE_EDIT_TEXT
            || (websurface_selected[0]) // && websurface_selected[0]->GetTextEditing())
            || (websurface_selected[1]); // && websurface_selected[1]->GetTextEditing());
}

void Game::keyPressEvent(QKeyEvent * e)
{
    bool pressed = gamepad_button_press;
    gamepad_button_press = false;

    keys[Qt::Key_Control] = (e->modifiers().testFlag(Qt::ControlModifier));
    //qDebug() << "Game::keyPressEvent" << e->key();
    keys[e->key()] = true;

    QPointer <AssetVideo> vid_sel;
    QPointer <AssetWebSurface> web_sel;
    QString sel;

    for (int i=0; i<2; ++i) {
        if (video_selected[i]) {
            vid_sel = video_selected[i];
        }
        if (websurface_selected[i]) {
            web_sel = websurface_selected[i];
        }
        if (selected[i].length() > 0) {
            sel = selected[i];
        }
    }

    //qDebug() << "Game::keyPressEvent" << e->key() << keys[Qt::Key_Shift] << keys[Qt::Key_Tab] << e->matches(QKeySequence::StandardKey )
    QPointer <Room> r = env->GetCurRoom();

    switch (e->key()) {

    case Qt::Key_F5:
        if (!e->isAutoRepeat()) {
            web_sel ? web_sel->Reload() : env->ReloadRoom();
        }
        return;

    case Qt::Key_AsciiTilde:
        if (!GetPlayerEnteringText() && !e->isAutoRepeat()) {
            //remove
            RemovePrivateWebsurface();
            ClearSelection(0);
            ClearSelection(1);
        }
        break;

    case Qt::Key_QuoteLeft:
        if (!GetPlayerEnteringText() && !e->isAutoRepeat()) {
            if (GetPrivateWebsurfacesVisible()) {
                //surfaces already visible, just add another
                AddPrivateWebsurface();
                SetPrivateWebsurfacesVisible(true);
            }
            else {
                //surfaces not visible.  if none, create, if some, just show
                if (private_websurfaces.isEmpty()) {
                    AddPrivateWebsurface();
                }
                SetPrivateWebsurfacesVisible(true);
            }

            ClearSelection(0);
            ClearSelection(1);
        }
        break;

    case Qt::Key_Backspace:
    case Qt::Key_Delete:

        //only allow deletion if not editing text, and room is not locked
        if (!GetPlayerEnteringText() && sel.length() > 0 && !e->isAutoRepeat() && !r->GetProperties()->GetLocked()) {
            QPointer <RoomObject> o = r->GetRoomObject(sel);
            if (o) {
                const QString delete_obj_code = o->GetXMLCode();
                const QVector3D delete_pos = o->GetPos();
                if (r->DeleteSelected(sel)) {
                    SoundManager::Play(SOUND_DELETING, false, delete_pos, 1.0f);
                    multi_players->SetRoomDeleteCode(delete_obj_code);
                }
            }
            ClearSelection(0);
            ClearSelection(1);
            state = JVR_STATE_DEFAULT;
        }

        break;

    default:
        break;
    }

    QPointer <RoomObject> obj = r->GetRoomObject(sel);
    if (web_sel) {
        if (e->key() == Qt::Key_Left && e->modifiers().testFlag(Qt::ControlModifier)) {
            web_sel->GoBack();
        }
        else if (e->key() == Qt::Key_Right && e->modifiers().testFlag(Qt::ControlModifier)) {
            web_sel->GoForward();
        }
        else if (!pressed) { // && (web_sel->GetTextEditing())) {
            web_sel->keyPressEvent(e);
        }
    }

    //54.9 - allow activity as long as player not entering text
    if (!GetPlayerEnteringText()) {
        switch (state) {

        case JVR_STATE_DEFAULT:
        case JVR_STATE_INTERACT_TELEPORT:
        {
            const bool do_recording = (e->key() == Qt::Key_F9) && !keys[Qt::Key_Control];
            if (!e->isAutoRepeat() && SoundManager::GetCaptureDeviceEnabled() && do_recording && !player->GetRecording()) {
                player->SetRecording(true);
            }

            if ((e->modifiers().testFlag(Qt::ShiftModifier) || e->key() == Qt::Key_Shift)) {
                player->SetRunning(true);
            }

            bool defaultPrevented = r->RunKeyPressEvent(e, player, multi_players);
            if (defaultPrevented) {
                keys[e->key()] = false;
                break;
            }

            switch (e->key()) {

            case Qt::Key_Tab:
                virtualmenu->MenuButtonPressed();
                break;

            case Qt::Key_W:
            case Qt::Key_Up:
                if (!GetPlayerEnteringText() && !keys[Qt::Key_Control]) {
                    player->SetWalkForward(true);
                }
                break;

            case Qt::Key_A:
            case Qt::Key_Left:
                if (!GetPlayerEnteringText() && !keys[Qt::Key_Control]) {
                    player->SetWalkLeft(true);
                }
                break;

            case Qt::Key_S:
            case Qt::Key_Down:
                if (!GetPlayerEnteringText() && !keys[Qt::Key_Control]) {
                    player->SetWalkBack(true);
                }
                else if (e->key() == Qt::Key_S && keys[Qt::Key_Control]) {
                    SaveRoom(r->GetSaveFilename());
                }
                break;

            case Qt::Key_D:
            case Qt::Key_Right:
                if (!GetPlayerEnteringText() && !keys[Qt::Key_Control]) {
                    player->SetWalkRight(true);
                }
                break;

            case Qt::Key_Backspace:
                if (!GetPlayerEnteringText() && web_sel.isNull()) {
                    StartResetPlayer();
                }
                break;

            case Qt::Key_R:
                if (e->modifiers().testFlag(Qt::ControlModifier)) {
                    StartReloadPortal();
                }
                break;

            case Qt::Key_F:
            {
                const bool b = !player->GetFlying();
                SoundManager::Play(b ? SOUND_FLIGHT : SOUND_NOFLIGHT, false, player->GetProperties()->GetPos()->toQVector3D(), 10.0f);
                player->SetFlying(b);
                player->SetFollowMode(false);
                player->SetFollowModeUserID("");
            }
                break;

            case Qt::Key_C:
                if (keys[Qt::Key_Control]) {
                    SoundManager::Play(SOUND_COPYING, false, player->GetProperties()->GetPos()->toQVector3D(), 1.0f);
                    QApplication::clipboard()->setText(r->GetProperties()->GetURL());
                }
                break;

            case Qt::Key_V:
                if (keys[Qt::Key_Control]) {
                    if (r->GetProperties()->GetLocked()) {
                        MathUtil::ErrorLog("Warning: cannot do paste, room.locked=true");
                    }
                    else {
                        QPointer <RoomObject> o = r->GetRoomObject(copy_selected);
                        if (o) {
                            QString new_jsid;

                            if (keys[Qt::Key_Shift] ) {
                                new_jsid = r->PasteSelected(copy_selected, player->GetCursorPos(0), o->GetXDir(), o->GetYDir(), o->GetZDir(), GetGlobalUUID());
                            }
                            else {
                                new_jsid = r->PasteSelected(copy_selected, player->GetCursorPos(0), player->GetCursorXDir(0), player->GetCursorZDir(0), -player->GetCursorYDir(0), GetGlobalUUID());
                            }

                            if (r->GetRoomObject(new_jsid)) {
                                r->GetRoomObject(new_jsid)->GetProperties()->SetSync(true);
                                SoundManager::Play(SOUND_PASTING, false, player->GetCursorPos(0), 1.0f);
                            }
                        }
                        else {
                            //59.6 - try to copy/paste from clipboard
                            const QClipboard *clipboard = QApplication::clipboard();
                            const QMimeData *mimeData = clipboard->mimeData();

                            if (mimeData->hasImage()) {
                                //setPixmap(qvariant_cast<QPixmap>(mimeData->imageData()));
                                //Create an AssetImage, but set the data
                                QImage image = qvariant_cast<QImage>(mimeData->imageData());
                                const float ar = float(image.height()) / float(image.width());

                                QByteArray arr;
                                QBuffer buffer(&arr);
                                buffer.open(QIODevice::WriteOnly);
                                image.save(&buffer, "jpg", 80);

                                //qDebug() << "Generating" << arr.size();
                                const QString global_uuid = GetGlobalUUID();
                                QPointer <AssetImage> a = new AssetImage();
                                a->GetProperties()->SetID(global_uuid);
                                a->SetSrc("", QString("data:image/gif;base64,")+arr.toBase64());
                                a->GetProperties()->SetSync(true);
                                r->AddAssetImage(a);

                                QPointer <RoomObject> new_object = new RoomObject();
                                new_object->SetType(TYPE_OBJECT);
                                new_object->GetProperties()->SetJSID(global_uuid);
                                new_object->GetProperties()->SetID("plane");
                                new_object->GetProperties()->SetCollisionID("plane");
                                new_object->GetProperties()->SetImageID(global_uuid);
                                new_object->GetProperties()->SetCullFace("none");
                                new_object->GetProperties()->SetLighting(false);
                                new_object->GetProperties()->SetSync(true);
                                new_object->GetProperties()->SetScale(QVector3D(1.0f, ar, 1.0f));

                                ClearSelection(0);
                                ClearSelection(1);
                                selected[0] = r->AddRoomObject(new_object);
                                state = JVR_STATE_DRAGDROP;
                                UpdateDragAndDropPosition(new_object, 0);
                            }
                            else if (mimeData->hasHtml()) {
                                  //setText(mimeData->html());
                                  //setTextFormat(Qt::RichText);
                            }
                            else if (mimeData->hasText()) {
                                  //setText(mimeData->text());
                                  //setTextFormat(Qt::PlainText);
                            }
                        }
                    }
                }
                break;

            case Qt::Key_Slash:
            case Qt::Key_Backslash:
                show_position_mode = !show_position_mode;
                break;

            case Qt::Key_M:
                ToggleSoundEnabled();
                break;

            case Qt::Key_G:
                if (keys[Qt::Key_Control]) {
                    multi_players->GetRecording() ? StopRecording() : StartRecording(keys[Qt::Key_Shift]);
                }
                break;

            case Qt::Key_0:
            case Qt::Key_1:
            case Qt::Key_2:
            case Qt::Key_3:
            case Qt::Key_4:
            case Qt::Key_5:
            case Qt::Key_6:
            case Qt::Key_7:
            case Qt::Key_8:
            case Qt::Key_9:
            {
                int index = 0;
                switch (e->key()) {
                case Qt::Key_0:
                    index = 0;
                    break;
                case Qt::Key_1:
                    index = 1;
                    break;
                case Qt::Key_2:
                    index = 2;
                    break;
                case Qt::Key_3:
                    index = 3;
                    break;
                case Qt::Key_4:
                    index = 4;
                    break;
                case Qt::Key_5:
                    index = 5;
                    break;
                case Qt::Key_6:
                    index = 6;
                    break;
                case Qt::Key_7:
                    index = 7;
                    break;
                case Qt::Key_8:
                    index = 8;
                    break;
                case Qt::Key_9:
                    index = 9;
                    break;
                }

                const QVariantList urls = bookmarks->GetBookmarks();
                if (SettingsManager::GetPortalHotkeys() && index < urls.size()) {
                    const QString url_str = urls[index].toMap()["url"].toString();
                    CreatePortal(url_str, true);
                }
            }
                break;

            default:
                break;
            }
        }

            break;

        case JVR_STATE_DRAGDROP:
        {
            switch (e->key()) {
            case Qt::Key_A:
            case Qt::Key_Left:
            {
                dragdrop_xform.rotate(90.0f, 0, -1, 0);
            }
                break;
            case Qt::Key_D:
            case Qt::Key_Right:
            {
                dragdrop_xform.rotate(90.0f, 0, 1, 0);
            }
                break;
            case Qt::Key_Q:
            {
                dragdrop_xform.rotate(90.0f, 0, 0, -1);
            }
                break;
            case Qt::Key_E:
            {
                dragdrop_xform.rotate(90.0f, 0, 0, 1);
            }
                break;
            case Qt::Key_W:
            case Qt::Key_Up:
            {
                dragdrop_xform.rotate(90.0f, -1, 0, 0);
            }
                break;
            case Qt::Key_S:
            case Qt::Key_Down:
            {
                dragdrop_xform.rotate(90.0f, 1, 0, 0);
            }
                break;
            }
            UpdateDragAndDropPosition(r->GetRoomObject(selected[0]), 0);
        }
            break;

        case JVR_STATE_UNIT_TRANSLATE:
        {
            switch (e->key()) {
            case Qt::Key_A:
            case Qt::Key_Left:
                EditModeTranslate(obj,-1,0,0);
                break;
            case Qt::Key_D:
            case Qt::Key_Right:
                EditModeTranslate(obj,1,0,0);
                break;
            case Qt::Key_S:
            case Qt::Key_Down:
                EditModeTranslate(obj,0,-1,0);
                break;
            case Qt::Key_W:
            case Qt::Key_Up:
                EditModeTranslate(obj,0,1,0);
                break;
            case Qt::Key_E:
                EditModeTranslate(obj,0,0,1);
                break;
            case Qt::Key_Q:
                EditModeTranslate(obj,0,0,-1);
                break;
            case Qt::Key_Backtab:
                state = JVR_STATE_UNIT_MIRROR;
                break;
            case Qt::Key_Tab:
                state = JVR_STATE_UNIT_ROTATE;
                break;
            }
        }
            break;

        case JVR_STATE_UNIT_ROTATE:
        {
            switch (e->key()) {
            case Qt::Key_A:
            case Qt::Key_Left:
                EditModeRotate(obj, -1, 0, 0);
                break;
            case Qt::Key_D:
            case Qt::Key_Right:
                EditModeRotate(obj, 1, 0, 0);
                break;
            case Qt::Key_Q:
                EditModeRotate(obj, 0,0,-1);
                break;
            case Qt::Key_E:
                EditModeRotate(obj, 0,0,1);
                break;
            case Qt::Key_W:
            case Qt::Key_Up:
                EditModeRotate(obj, 0,-1,0);
                break;
            case Qt::Key_S:
            case Qt::Key_Down:
                EditModeRotate(obj, 0,1,0);
                break;
            case Qt::Key_Backtab:
                state = JVR_STATE_UNIT_TRANSLATE;
                break;
            case Qt::Key_Tab:
                state = JVR_STATE_UNIT_SCALE;
                break;
            }

        }
            break;

        case JVR_STATE_UNIT_SCALE:
        {
            switch (e->key()) {
            case Qt::Key_A:
            case Qt::Key_Left:
                EditModeScale(obj,-1,0,0);
                break;
            case Qt::Key_D:
            case Qt::Key_Right:
                EditModeScale(obj,1,0,0);
                break;
            case Qt::Key_S:
            case Qt::Key_Down:
                EditModeScale(obj,0,-1,0);
                break;
            case Qt::Key_W:
            case Qt::Key_Up:
                EditModeScale(obj,0,1,0);
                break;
            case Qt::Key_Q:
                EditModeScale(obj,0,0,-1);
                break;
            case Qt::Key_E:
                EditModeScale(obj,0,0,1);
                break;
            case Qt::Key_Backtab:
                state = JVR_STATE_UNIT_ROTATE;
                break;
            case Qt::Key_Tab:
                state = JVR_STATE_UNIT_COLOUR;
                break;
            }
        }
            break;

        case JVR_STATE_UNIT_COLOUR:
        {
            QColor c = MathUtil::GetVector4AsColour(obj->GetProperties()->GetColour()->toQVector4D());

            switch (e->key()) {
            case Qt::Key_A:
            case Qt::Key_Left:
                if (c.red() > 0) {
                    c.setRed(c.red()-1);
                    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(c));
                    obj->GetProperties()->SetSync(true);
                }
                break;
            case Qt::Key_D:
            case Qt::Key_Right:
                if (c.red() < 255) {
                    c.setRed(c.red()+1);
                    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(c));
                    obj->GetProperties()->SetSync(true);
                }
                break;
            case Qt::Key_S:
            case Qt::Key_Down:
                if (c.green() > 0) {
                    c.setGreen(c.green()-1);
                    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(c));
                    obj->GetProperties()->SetSync(true);
                }
                break;
            case Qt::Key_W:
            case Qt::Key_Up:
                if (c.green() < 255) {
                    c.setGreen(c.green()+1);
                    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(c));
                    obj->GetProperties()->SetSync(true);
                }
                break;
            case Qt::Key_Q:
                if (c.blue() > 0) {
                    c.setBlue(c.blue()-1);
                    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(c));
                    obj->GetProperties()->SetSync(true);
                }
                break;
            case Qt::Key_E:
                if (c.blue() < 255) {
                    c.setBlue(c.blue()+1);
                    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(c));
                    obj->GetProperties()->SetSync(true);
                }
                break;
            case Qt::Key_F:
                if (c.alphaF() > 0.0f) {
                    c.setAlphaF(qMax(0.0f, float(c.alphaF())-0.01f));
                    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(c));
                    obj->GetProperties()->SetSync(true);
                }
                break;
            case Qt::Key_R:
                if (c.alphaF() < 1.0f) {
                    c.setAlphaF(qMin(1.0f, float(c.alphaF())+0.01f));
                    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(c));
                    obj->GetProperties()->SetSync(true);
                }
                break;
            case Qt::Key_Backtab:
                state = JVR_STATE_UNIT_SCALE;
                break;
            case Qt::Key_Tab:
                state = JVR_STATE_UNIT_COLLISIONID;
                break;
            }
        }
            break;

        case JVR_STATE_UNIT_COLLISIONID:
        {
            QList <QString> col_id_list;
            col_id_list.push_back("");            
            col_id_list.push_back("capsule");
            col_id_list.push_back("cone");
            col_id_list.push_back("cube");
            col_id_list.push_back("cylinder");
            col_id_list.push_back("pipe");
            col_id_list.push_back("plane");
            col_id_list.push_back("pyramid");
            col_id_list.push_back("sphere");
            col_id_list.push_back("torus");
            if (!col_id_list.contains(obj->GetProperties()->GetID())) {
                col_id_list.push_back(obj->GetProperties()->GetID());
            }

            int cur_index = qMax(col_id_list.indexOf(obj->GetProperties()->GetCollisionID()), 0);

            switch (e->key()) {
            case Qt::Key_W:
            case Qt::Key_Q:
            case Qt::Key_Up:
            case Qt::Key_A:
            case Qt::Key_Left:
                cur_index = ((cur_index + col_id_list.size() - 1) % col_id_list.size());
                break;
            case Qt::Key_D:
            case Qt::Key_Right:
            case Qt::Key_S:
            case Qt::Key_Down:
            case Qt::Key_E:                
                cur_index = ((cur_index+1) % col_id_list.size());
                break;
            case Qt::Key_Backtab:
                state = JVR_STATE_UNIT_COLOUR;
                break;
            case Qt::Key_Tab:
                state = JVR_STATE_UNIT_COLLISIONSCALE;
                break;
            default:
                break;
            }

            obj->GetProperties()->SetCollisionID(col_id_list[cur_index]);
            r->SelectCollisionAssetForObject(sel, col_id_list[cur_index]);
            obj->GetProperties()->SetSync(true);
        }

            break;

        case JVR_STATE_UNIT_COLLISIONSCALE:
        {
            switch (e->key()) {
            case Qt::Key_A:
            case Qt::Key_Left:
                EditModeCollisionScale(obj,-1,0,0);
                break;
            case Qt::Key_D:
            case Qt::Key_Right:
                EditModeCollisionScale(obj,1,0,0);
                break;
            case Qt::Key_S:
            case Qt::Key_Down:
                EditModeCollisionScale(obj,0,-1,0);
                break;
            case Qt::Key_W:
            case Qt::Key_Up:
                EditModeCollisionScale(obj,0,1,0);
                break;
            case Qt::Key_Q:
                EditModeCollisionScale(obj,0,0,-1);
                break;
            case Qt::Key_E:
                EditModeCollisionScale(obj,0,0,1);
                break;
            case Qt::Key_Backtab:
                state = JVR_STATE_UNIT_COLLISIONID;
                break;
            case Qt::Key_Tab:
                state = JVR_STATE_UNIT_LIGHTING;
                break;
            }
        }
            break;

        case JVR_STATE_UNIT_LIGHTING:
        {
            switch (e->key()) {
            case Qt::Key_W:
            case Qt::Key_Up:
            case Qt::Key_A:
            case Qt::Key_Left:
            case Qt::Key_D:
            case Qt::Key_Right:
            case Qt::Key_S:
            case Qt::Key_Down:
            case Qt::Key_Q:
            case Qt::Key_E:

                obj->GetProperties()->SetLighting(!obj->GetProperties()->GetLighting());
                obj->GetProperties()->SetSync(true);

                break;
            case Qt::Key_Backtab:
                state = JVR_STATE_UNIT_COLLISIONSCALE;
                break;
            case Qt::Key_Tab:
                state = JVR_STATE_UNIT_CULL_FACE;
                break;
            }
        }

            break;

        case JVR_STATE_UNIT_CULL_FACE:
        {
            switch (e->key()) {
            case Qt::Key_W:
            case Qt::Key_Up:
            case Qt::Key_A:
            case Qt::Key_Left:
            case Qt::Key_D:
            case Qt::Key_Right:
            case Qt::Key_S:
            case Qt::Key_Down:
            case Qt::Key_Q:
            case Qt::Key_E:
                if (obj->GetProperties()->GetCullFace() == "back")
                {
                    obj->GetProperties()->SetCullFace("front");
                }
                else if (obj->GetProperties()->GetCullFace() == "front")
                {
                    obj->GetProperties()->SetCullFace("none");
                }
                else
                {
                    obj->GetProperties()->SetCullFace("back");
                }

                obj->GetProperties()->SetSync(true);

                break;
            case Qt::Key_Backtab:
                state = JVR_STATE_UNIT_LIGHTING;
                break;
            case Qt::Key_Tab:
                state = JVR_STATE_UNIT_MIRROR;
                break;
            }
        }

            break;

        case JVR_STATE_UNIT_MIRROR:
        {
            switch (e->key()) {
            case Qt::Key_W:
            case Qt::Key_Up:
            case Qt::Key_A:
            case Qt::Key_Left:
            case Qt::Key_D:
            case Qt::Key_Right:
            case Qt::Key_S:
            case Qt::Key_Down:
                obj->GetProperties()->SetMirror(!obj->GetProperties()->GetMirror());
                break;
            case Qt::Key_Backtab:
                state = JVR_STATE_UNIT_CULL_FACE;
                break;
            case Qt::Key_Tab:
                state = JVR_STATE_UNIT_TRANSLATE;
                break;
            }
        }
            break;

        case JVR_STATE_SELECT_ASSET:
            switch (e->key()) {
            case Qt::Key_S:
            case Qt::Key_Down:
            case Qt::Key_A:
            case Qt::Key_Left:
            case Qt::Key_Q:
                r->SelectAssetForObject(sel, -1);
                if (obj) {
                    SoundManager::Play(SOUND_CLICK1, false, obj->GetProperties()->GetPos()->toQVector3D(), 1.0f);
                    obj->GetProperties()->SetSync(true);
                }
                break;

            case Qt::Key_W:
            case Qt::Key_Up:
            case Qt::Key_D:
            case Qt::Key_Right:
            case Qt::Key_E:
                r->SelectAssetForObject(sel, 1);
                if (obj) {
                    SoundManager::Play(SOUND_CLICK2, false, obj->GetProperties()->GetPos()->toQVector3D(), 1.0f);
                    obj->GetProperties()->SetSync(true);
                }
                break;
            default:
                break;
            }

        default:
            break;

        }

        if (state == JVR_STATE_UNIT_TRANSLATE ||
                state == JVR_STATE_UNIT_ROTATE ||
                state == JVR_STATE_UNIT_SCALE ||
                state == JVR_STATE_UNIT_COLOUR ||
                state == JVR_STATE_UNIT_COLLISIONID ||
                state == JVR_STATE_UNIT_LIGHTING ||
                state == JVR_STATE_UNIT_CULL_FACE ||
                state == JVR_STATE_UNIT_BLEND_SRC ||
                state == JVR_STATE_UNIT_BLEND_DEST ||
                state == JVR_STATE_UNIT_MIRROR ||
                state == JVR_STATE_SELECT_ASSET) {

            switch (e->key()) {
            case Qt::Key_1:
                unit_scale = 0;
                break;
            case Qt::Key_2:
                unit_scale = 1;
                break;
            case Qt::Key_3:
                unit_scale = 2;
                break;
            case Qt::Key_4:
                unit_scale = 3;
                break;
            case Qt::Key_C:
                if (keys[Qt::Key_Control]) {
                    copy_selected = sel;
                    r->SetSelected(sel, false);
                    selected[0].clear();
                    selected[1].clear();
                    state = JVR_STATE_DEFAULT;
                    SoundManager::Play(SOUND_COPYING, false, player->GetProperties()->GetPos()->toQVector3D(), 1.0f);
                }
                break;
            }
        }
    }
    else {
        if (state == JVR_STATE_EDIT_TEXT) {
            switch (e->key()) {
            case Qt::Key_Escape:
                r->DeleteSelected(selected[0]);
                r->SetSelected(selected[0], false);
                state = JVR_STATE_DEFAULT;
                selected[0].clear();
                selected[1].clear();
                break;

            case Qt::Key_Enter:
            case Qt::Key_Return:
                r->SetSelected(selected[0], false);
                state = JVR_STATE_DEFAULT;
                selected[0].clear();
                selected[1].clear();
                break;

            default:
                r->EditText(sel, e->text(), (e->key() == Qt::Key_Backspace));
                if (obj) {
                    obj->GetProperties()->SetSync(true);
                }
                break;
            }
        }
    }
}

void Game::SetMouseDoPitch(const bool b)
{
    SettingsManager::SetMousePitchEnabled(b);
}

bool Game::GetMouseDoPitch()
{
    return SettingsManager::GetMousePitchEnabled();
}

QPointer <ControllerManager> Game::GetControllerManager()
{
    return controller_manager;
}

QPointer <BookmarkManager> Game::GetBookmarkManager()
{
    return bookmarks;
}

void Game::keyReleaseEvent(QKeyEvent * e)
{
    keys[e->key()] = false;
    //qDebug() << "Game::keyReleaseEvent" << e->key();

    QPointer <Room> r = env->GetCurRoom();

    QPointer <AssetVideo> vid_sel;
    QPointer <AssetWebSurface> web_sel;
    QString sel;

    for (int i=0; i<2; ++i) {
        if (video_selected[i]) {
            vid_sel = video_selected[i];
        }
        if (websurface_selected[i]) {
            web_sel = websurface_selected[i];
        }
        if (selected[i].length() > 0) {
            sel = selected[i];
        }
    }

    switch (e->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:
        if (!GetPlayerEnteringText() && !keys[Qt::Key_Control]) {
            player->SetWalkForward(false);
        }
        break;

    case Qt::Key_A:
    case Qt::Key_Left:
        if (!GetPlayerEnteringText() && !keys[Qt::Key_Control]) {
            player->SetWalkLeft(false);
        }
        break;

    case Qt::Key_S:
    case Qt::Key_Down:
        if (!GetPlayerEnteringText() && !keys[Qt::Key_Control]) {
            player->SetWalkBack(false);
        }
        else if (e->key() == Qt::Key_S && keys[Qt::Key_Control]) {
            SaveRoom(r->GetSaveFilename());
        }
        break;

    case Qt::Key_D:
    case Qt::Key_Right:
        if (!GetPlayerEnteringText() && !keys[Qt::Key_Control]) {
            player->SetWalkRight(false);
        }
        break;

    case Qt::Key_Shift:
        if (!GetPlayerEnteringText()) {
            player->SetRunning(false);
        }
        break;

    default:
        break;
    }    

    //logic for stop speech
    //if capturedeviceenabled, and !micalwayson, and speaking, set speaking false
    const bool do_recording = (e->key() == Qt::Key_F9) && !keys[Qt::Key_Control];
    if (!e->isAutoRepeat() && SoundManager::GetCaptureDeviceEnabled() && do_recording && player->GetRecording()) {
        player->SetRecording(false);

    }

    if (web_sel) { // && web_sel->GetTextEditing()) {
        web_sel->keyReleaseEvent(e);
    }
    else if (GetPlayerEnteringText()) {
        return;
    }

    bool defaultPrevented = r->RunKeyReleaseEvent(e, player, multi_players);
    if (defaultPrevented) {
        return;
    }

    // any non-modifier key release events should go here
}

void Game::ResetPlayer()
{
    QPointer <Room> r = env->GetCurRoom();
    if (r) {
        QPointer <RoomObject> o = r->GetEntranceObject();
        if (o) {
            player->GetProperties()->SetPos(o->GetPos() + o->GetZDir()*0.5f);
            player->GetProperties()->SetDir(o->GetZDir());
            player->UpdateDir();
        }

        r->SetPlayerInRoom(player);
    }
    else {
        player->GetProperties()->SetPos(QVector3D(0,0,0));
        player->GetProperties()->SetDir(QVector3D(0,0,-1));
        player->UpdateDir();
    }

    player->GetProperties()->SetVel(QVector3D(0,0,0));
    player->SetHMDCalibrated(false);
}

void Game::TeleportPlayer()
{
    //qDebug() << "Game::TeleportPlayer()" << teleport_portal << teleport_portal->GetActive() << teleport_portal->GetRoom() << teleport_portal->GetRoom()->GetProcessed();
    ClearSelection(0);
    ClearSelection(1);
    SetPrivateWebsurfacesVisible(false);

    QPointer <Room> r = env->GetCurRoom()->GetConnectedRoom(teleport_portal);
    if (teleport_portal && teleport_portal->GetProperties()->GetActive() && r) {
        env->MovePlayer(teleport_portal, player, true);
    }
    else {
        //qDebug() << "Game::TeleportPlayer()" << player->GetV("pos") << teleport_p;
        player->GetProperties()->SetPos(teleport_p); //59.0 - do not change player orientation, fixes 180 rotation bug using vive
    }
    player->GetProperties()->SetVel(QVector3D(0,0,0));
    player->SetFollowMode(false);
    player->SetFollowModeUserID("");

    //Note: at this point in the code, player's curroom may have changed
    env->GetCurRoom()->SetPlayerInRoom(player);   
}

float Game::GetCurrentNearDist()
{
    QPointer <Room> r = env->GetCurRoom();
    return (r ? r->GetProperties()->GetNearDist() : 0.1f);
}

float Game::GetCurrentFarDist()
{
    QPointer <Room> r = env->GetCurRoom();
    return (r ? r->GetProperties()->GetFarDist() : 1000.0f);
}

void Game::SetState(const JVR_GameState & s)
{
    state = s;
}

void Game::ResetCursor(const QPoint p)
{
    cursor_win = p;
    last_cursor_win = p;
}

JVR_GameState Game::GetState() const
{
    return state;
}

void Game::ToggleSoundEnabled()
{
    const bool enabled = !SoundManager::GetEnabled();
    SoundManager::SetEnabled(enabled);
    SettingsManager::SetSoundsEnabled(enabled);

    QList <QPointer <Room> > nodes = env->GetRootRoom()->GetAllChildren();
    for (int i=0; i<nodes.size(); ++i) {
        if (nodes[i]) {
            nodes[i]->SetSoundsEnabled(enabled);
        }
    }
}

void Game::SaveRoom(const QString out_filename)
{
    //first ensure that the file is either online (not on local filesystem)
    //or it ends in .htm or .html (and can thus be overwritten)
    QFileInfo check_file(out_filename);
    if (check_file.exists()) {
        if (out_filename.right(3).toLower() != "htm"
                && out_filename.right(4).toLower() != "html"
                && out_filename.right(4).toLower() != "json") {
            qDebug() << "Game::SaveRoom() - error: cannot overwrite non-html/json files on local filesystem";
            return;
        }

        //copy existing file for backup
        QFile::copy(out_filename, MathUtil::GetSaveTimestampFilename());
    }

    QPointer <Room> r = env->GetCurRoom();

    bool success;
    if (out_filename.right(4).toLower() == "json") {
        success = r->SaveJSON(out_filename);
    }
    else {
        success = r->SaveXML(out_filename);
    }

    if (success) {
        SoundManager::Play(SOUND_SAVED, false, player->GetProperties()->GetPos()->toQVector3D(), 1.0f);
    }

    //add URL to saved file to workspaces list
    bookmarks->AddWorkspace(QUrl(out_filename).toString(), r->GetProperties()->GetTitle());
}

void Game::ExportRoomAFrame(const QString out_filename)
{
    QFileInfo check_file(out_filename);
    if (check_file.exists()) {
        if (out_filename.right(3).toLower() != "htm"
                && out_filename.right(4).toLower() != "html") {
            qDebug() << "Game::ExportRoomAFrame() - error: cannot overwrite non-html files on local filesystem";
            return;
        }
    }

    if (env->GetCurRoom()->SaveAFrame(out_filename)) {
        SoundManager::Play(SOUND_SAVED, false, player->GetProperties()->GetPos()->toQVector3D(), 1.0f);
    }
}

void Game::UpdateCursorAndTeleportTransforms()
{
    QPointer <Room> r = env->GetCurRoom();
    ControllerState * s = controller_manager->GetStates();

    cursor_active = -1;
    if (!controller_manager->GetUsingSpatiallyTrackedControllers()) {
        cursor_active = 0;
    }
    else {
        if (s[0].GetClick().pressed || s[0].GetTeleport().pressed) {
            cursor_active = 0;
        }
        else if (s[1].GetClick().pressed || s[1].GetTeleport().pressed) {
            cursor_active = 1;
        }
        else if (s[0].GetClick().hover || s[0].GetTeleport().hover) {
            cursor_active = 0;
        }
        else if (s[1].GetClick().hover || s[1].GetTeleport().hover) {
            cursor_active = 1;
        }
    }

    if (cursor_active >= 0) {
        const QVector3D pos = player->GetProperties()->GetPos()->toQVector3D();
        const QVector3D p = player->GetCursorPos(cursor_active);
        const QVector3D z = player->GetCursorZDir(cursor_active);
        const QVector3D e = player->GetProperties()->GetEyePoint();
        const QVector3D v = player->GetProperties()->GetViewDir()->toQVector3D();
        const float z_dot_y = QVector3D::dotProduct(z, QVector3D(0, 1, 0));
        const float max_dist = r->GetProperties()->GetTeleportMaxDist();

        teleport_z = QVector3D(0, 1, 0);
        if (fabsf(z_dot_y) < 0.1f) {
            teleport_y = -z;
        }
        else {
            teleport_y = v;
        }
        teleport_y -= teleport_z * QVector3D::dotProduct(teleport_y, teleport_z);
        teleport_y.normalize();
        teleport_x = QVector3D::crossProduct(teleport_y, teleport_z);

        if ((pos - p).length() > max_dist) {
            const QVector3D tele_dir = (p - e).normalized();
            teleport_p = e + tele_dir * max_dist;
        }
        else if (fabsf(z_dot_y) < 0.1f) {
            teleport_p = p + z;
        }
        else if (z_dot_y < -0.9f) {
            teleport_p = p + z * 2.0f;
        }
        else {
            teleport_p = p;
        }
    }

    //cursor position updated?
    if (player->GetCursorActive(0)) {
        multi_players->SetCursorPosition(player->GetCursorPos(0), player->GetCursorXDir(0), player->GetCursorYDir(0), player->GetCursorZDir(0), player->GetCursorScale(0));
    }

    //qDebug() << "Game::UpdateCursorAndTeleportTransforms()" << cursor_active << teleport_p;
}

void Game::DrawCursorGL()
{
    QPointer <Room> r = env->GetCurRoom();
    QPointer <RoomObject> cursor_obj = r->GetRoomObject(player->GetCursorObject(cursor_active));
    QPointer <AssetShader> shader = Room::GetTransparencyShader();

    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }

    //draw crosshair
    if (state == JVR_STATE_DEFAULT || state == JVR_STATE_INTERACT_TELEPORT) {

        Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::DISABLED);
        Renderer::m_pimpl->SetDepthFunc(DepthFunc::ALWAYS);
        Renderer::m_pimpl->SetDepthMask(DepthMask::DEPTH_WRITES_DISABLED);

        if (cursor_active >= 0) {
            const float dist = (player->GetProperties()->GetEyePoint() - player->GetCursorPos(cursor_active)).length();
            QVector3D p = player->GetCursorPos(cursor_active); // - player->GetV("view_dir") * dist * 0.1f;
            const float s = dist * 0.1f;
            const float alpha = qMax(qMin(dist-1.0f, 1.0f), 0.0f);
            const QVector3D x = player->GetCursorXDir(cursor_active);
            const QVector3D y = player->GetCursorYDir(cursor_active);
            const QVector3D z = player->GetCursorZDir(cursor_active);

            shader->SetUseTextureAll(false);
            shader->SetUseTexture(0, true);
            shader->SetUseLighting(false);
            shader->SetUseSkelAnim(false);
            shader->SetUseClipPlane(false);
            shader->UpdateObjectUniforms();

            if (virtualmenu->GetVisible() ||
                    ((draw_cursor || (controller_manager->GetUsingSpatiallyTrackedControllers() && controller_manager->GetStates()[cursor_active].GetClick().hover))
                    && r->GetProperties()->GetCursorVisible())) {
                MathUtil::PushModelMatrix();
                MathUtil::LoadModelIdentity();
                MathUtil::FacePosDirsGL(p, x, y, z);
                MathUtil::ModelMatrix().scale(s, s, s);

                //56.0 - hacky bugfix - if we do not draw something in the "cursor" scope, the 2DUI websurface does not update properly
                //qDebug() << p << x << y << z << s << alpha;
                if (virtualmenu->GetVisible() ||
                        websurface_selected[cursor_active] ||
                        (video_selected[cursor_active] && cursor_obj && !video_selected[cursor_active]->GetPlaying(cursor_obj->GetMediaContext()))) {
                    if (RoomObject::cursor_arrow_obj) {
                        RoomObject::cursor_arrow_obj->DrawGL(shader, QColor(255,255,255,255));
                    }
                }
                else if (rmb_held) {
                    if (RoomObject::cursor_arrow_obj) {
                        RoomObject::cursor_arrow_obj->DrawGL(shader, QColor(0,255,0,255));
                    }
                }
                else if (cursor_obj &&
                         ((cursor_obj->GetType() == TYPE_LINK && cursor_obj->GetProperties()->GetActive()) || cursor_obj->GetType() == TYPE_GHOST)) {
                    if (RoomObject::cursor_hand_obj) {
                        RoomObject::cursor_hand_obj->DrawGL(shader, QColor(255,255,255,int(255.0f*alpha)));
                    }
                }
                else if (SettingsManager::GetCrosshairEnabled() && cursor_active == 0) {
                    if (RoomObject::cursor_crosshair_obj) {
                        RoomObject::cursor_crosshair_obj->DrawGL(shader, QColor(255,255,255,255));
                    }
                }

                MathUtil::PopModelMatrix();
            }

            if (state == JVR_STATE_INTERACT_TELEPORT && GetAllowTeleport(cursor_active) && player->GetCursorActive(cursor_active) && teleport_held_time.elapsed() > teleport_hold_time_required) {

                //59.13 - keep teleport_p completely up-to-date
                teleport_p = p;

                //only draw the teleport avatar is a websurface is not selected
                shader->SetUseLighting(true);
                shader->UpdateObjectUniforms();

                MathUtil::PushModelMatrix();
                MathUtil::FacePosDirsGL(teleport_p, teleport_x, teleport_y, teleport_z);
                MathUtil::ModelMatrix().rotate(90.0f, 1, 0, 0);
                MathUtil::ModelMatrix().rotate(180.0f, 0, 1, 0);

                QPointer <RoomObject> player_avatar = multi_players->GetPlayer();
                if (player_avatar) {
                    QVector <GhostFrame> frames;
                    frames.push_back(GhostFrame());
                    frames.push_back(GhostFrame());
                    frames[0].time_sec = 0.0f;
                    frames[1].time_sec = 1.0f;
                    if (player_avatar->GetAssetGhost()) {
                        player_avatar->GetAssetGhost()->SetFromFrames(frames, 1000);
                    }

                    const QColor c = MathUtil::GetVector4AsColour(player_avatar->GetProperties()->GetColour()->toQVector4D());
                    const QVector3D p = player_avatar->GetPos();
                    QColor c2 = c;
                    c2.setAlpha(128);

                    const QString s = player_avatar->GetHMDType();
                    player_avatar->SetHMDType("");
                    player_avatar->GetProperties()->SetPos(QVector3D(0,0,0));

                    player_avatar->GetProperties()->SetColour(MathUtil::GetColourAsVector4(c2));
                    player_avatar->Update(player->GetDeltaTime());
                    player_avatar->DrawGL(shader, true, player->GetProperties()->GetPos()->toQVector3D());
                    player_avatar->GetProperties()->SetColour(MathUtil::GetColourAsVector4(c));
                    player_avatar->SetHMDType(s);

                    player_avatar->GetProperties()->SetPos(p);
                }

                MathUtil::PopModelMatrix();
            }
        }

        shader->SetConstColour(QVector4D(1,1,1,1));

        Renderer::m_pimpl->SetDepthFunc(DepthFunc::LEQUAL);
        Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::BACK);
    }
}

void Game::UpdateFollowMode()
{
    QPointer <Room> r = env->GetCurRoom();

    if (player->GetFollowMode()) {

        const QString follow_userid = player->GetFollowModeUserID();
        QString follow_url;

        //1.  find followed players' current URL
        QVariantList & partymode_data = MathUtil::GetPartyModeData();
        for (int i=0; i<partymode_data.size(); ++i) {
            //party mode attribs: userId, url (optional), roomId, name
            //qDebug() << partymode_data[i].toMap()["userId"];
            QMap<QString, QVariant> m = partymode_data[i].toMap();
            const QString userid = m["userId"].toString();
            const QString url = m["url"].toString();

            if (userid == follow_userid) {
                follow_url = url;
                break;
            }
        }

        //2. create room to followed user if we are not there, and move there
        if (r->GetProperties()->GetURL() != follow_url) {
            QPointer <RoomObject> p = CreatePortal(follow_url, false);
            env->MovePlayer(p, player, false); //move player to new room, updates history
        }

        //3.  if we are in the followed player's room with them, move us to their position
        QList <QPointer <RoomObject> > users = multi_players->GetPlayersInRoom(follow_url);
        for (int i=0; i<users.size(); ++i) {
            if (users[i] && users[i]->GetProperties()->GetID() == follow_userid) {
                player->GetProperties()->SetPos(users[i]->GetPos());
                player->GetProperties()->SetVel(QVector3D(0,0,0));
                break;
            }
        }

        //4. allow breaking out of follow mode by walking
        if (player->GetWalking()) {
            player->SetFollowMode(false);
            player->SetFollowModeUserID("");
        }
    }
}

void Game::UpdateOverlays()
{    
    QPointer <Room> r = env->GetCurRoom();
    PerformanceLogger & perf_logger = r->GetPerformanceLogger();
    //qDebug() << "perf:" << QString::number(perf_logger.GetAverageMainThreadCPUTime(),'f', 1);
    const QVector3D player_pos = player->GetProperties()->GetPos()->toQVector3D();

    if (state == JVR_STATE_UNIT_TRANSLATE ||
            state == JVR_STATE_UNIT_ROTATE ||
            state == JVR_STATE_UNIT_SCALE ||
            state == JVR_STATE_UNIT_COLOUR ||
            state == JVR_STATE_UNIT_COLLISIONID ||
            state == JVR_STATE_UNIT_COLLISIONSCALE ||
            state == JVR_STATE_UNIT_LIGHTING ||
            state == JVR_STATE_UNIT_CULL_FACE ||
            state == JVR_STATE_UNIT_BLEND_SRC ||
            state == JVR_STATE_UNIT_BLEND_DEST ||
            state == JVR_STATE_UNIT_MIRROR ||
            state == JVR_STATE_SELECT_ASSET) {

        QPointer <RoomObject> obj = r->GetRoomObject(selected[0]);
        if (obj) {
            info_text_geom.Clear();

            const ElementType t = obj->GetType();
            QString type_str = DOMNode::ElementTypeToTagName(t) + " js_id=\"" + obj->GetProperties()->GetJSID() + "\"";
            if (t == TYPE_LINK) {
                type_str += " url=\"" + obj->GetProperties()->GetURL().left(15)+ "...";
            }
            else if (t != TYPE_TEXT && t != TYPE_PARAGRAPH){
                type_str += " id=\"" + obj->GetProperties()->GetID() + "\"";
            }

            if (state == JVR_STATE_SELECT_ASSET) {
                info_text_geom.AddText(QString("<") + type_str, QColor(0,255,0));
            }
            else {
                info_text_geom.AddText(QString("<") + type_str);
            }

            if (state == JVR_STATE_UNIT_TRANSLATE) {
                info_text_geom.AddText(QString(" pos=") + MathUtil::GetVectorAsString(obj->GetProperties()->GetPos()->toQVector3D()), QColor(0,255,0));
            }
            else if (state == JVR_STATE_UNIT_ROTATE) {
                info_text_geom.AddText(QString(" xdir=") + MathUtil::GetVectorAsString(obj->GetProperties()->GetXDir()->toQVector3D()), QColor(0,255,0));
                info_text_geom.AddText(QString(" ydir=") + MathUtil::GetVectorAsString(obj->GetProperties()->GetYDir()->toQVector3D()), QColor(0,255,0));
                info_text_geom.AddText(QString(" zdir=") + MathUtil::GetVectorAsString(obj->GetProperties()->GetZDir()->toQVector3D()), QColor(0,255,0));
            }
            else if (state == JVR_STATE_UNIT_SCALE) {
                info_text_geom.AddText(QString(" scale=") + MathUtil::GetVectorAsString(obj->GetProperties()->GetScale()->toQVector3D()), QColor(0,255,0));
            }
            else if (state == JVR_STATE_UNIT_COLOUR) {
                info_text_geom.AddText(QString(" col=") + MathUtil::GetVector4AsString(obj->GetProperties()->GetColour()->toQVector4D()), QColor(0,255,0));
            }
            else if (state == JVR_STATE_UNIT_COLLISIONID) {
                info_text_geom.AddText(QString(" collision_id=\"") + obj->GetProperties()->GetCollisionID() + "\"", QColor(0,255,0));
            }
            else if (state == JVR_STATE_UNIT_COLLISIONSCALE) {
                info_text_geom.AddText(QString(" collision_scale=") + MathUtil::GetVectorAsString(obj->GetProperties()->GetCollisionScale()->toQVector3D()), QColor(0,255,0));
            }
            else if (state == JVR_STATE_UNIT_LIGHTING) {
                info_text_geom.AddText(QString(" lighting=") + MathUtil::GetBoolAsString(obj->GetProperties()->GetLighting()), QColor(0,255,0));
            }
            else if (state == JVR_STATE_UNIT_CULL_FACE) {
                info_text_geom.AddText(QString(" cull_face=\"") + obj->GetProperties()->GetCullFace() + "\"", QColor(0,255,0));
            }
            else if (state == JVR_STATE_UNIT_MIRROR) {
                info_text_geom.AddText(QString(" mirror=") + MathUtil::GetBoolAsString(obj->GetProperties()->GetMirror()), QColor(0,255,0));
            }

            if (t == TYPE_TEXT) {
                info_text_geom.AddText("  ...</text>");
            }
            else {
                info_text_geom.AddText("  .../>");
            }

            info_text_geom.AddText("");

            if (state == JVR_STATE_UNIT_TRANSLATE) {
                info_text_geom.AddText(QString("(snap: ")+QString::number(unit_translate_amount[unit_scale])+QString(")"), QColor(128,128,128));
            }
            else if (state == JVR_STATE_UNIT_ROTATE) {
                info_text_geom.AddText(QString("(degrees: ")+QString::number(unit_rotate_amount[unit_scale])+QString(")"), QColor(128,128,128));
            }
            else if (state == JVR_STATE_UNIT_SCALE) {
                info_text_geom.AddText(QString("(factor: ")+QString::number(unit_scale_amount[unit_scale])+QString(")"), QColor(128,128,128));
            }
        }
    }
    else if (show_position_mode) {
        QVector3D cursor_zdir = player->GetCursorZDir(0);

        if (fabsf(cursor_zdir.x()) < 0.005f) {
            cursor_zdir.setX(0.0f);
        }
        if (fabsf(cursor_zdir.y()) < 0.005f) {
            cursor_zdir.setY(0.0f);
        }
        if (fabsf(cursor_zdir.z()) < 0.005f) {
            cursor_zdir.setZ(0.0f);
        }
        player->SetCursorZDir(cursor_zdir, 0);

        const QVector3D ydir = player->GetProperties()->GetUpDir()->toQVector3D();
        const QVector3D zdir = player->GetProperties()->GetViewDir()->toQVector3D();
        const QVector3D xdir = QVector3D::crossProduct(zdir, ydir).normalized();

        const QString pos_text = QString("pos:") + MathUtil::GetVectorAsString(player_pos, false);
        const QString xdir_text = QString("xdir:") + MathUtil::GetVectorAsString(xdir, false);
        const QString ydir_text = QString("ydir:") + MathUtil::GetVectorAsString(ydir, false);
        const QString zdir_text = QString("zdir:") + MathUtil::GetVectorAsString(zdir, false);

        const int num_tris = r->GetRoomNumTris();
        const int num_objs = r->GetRoomObjects().size();

        QString renderer_text = QString("Renderer: ") + Renderer::m_pimpl->GetRendererName();
        double current_render_fps = 1000.0 / perf_logger.GetAverageRenderThreadCPUTime();
        QString fps_text3 = QString("Update: ") + QString::number(perf_logger.GetAverageMainThreadCPUTime(),'f', 1) + QString("ms/") + QString::number(1000.0 / perf_logger.GetAverageMainThreadCPUTime(),'f', 1) + QString("fps");
        QString thread_text = QString("Threads: ") + QString::number(QThreadPool::globalInstance()->activeThreadCount()) + "/" + QString::number(QThreadPool::globalInstance()->maxThreadCount());
        QString tricount_text = QString("Room: ") + QString::number(num_tris) + " tris, " + QString::number(num_objs) + " objects";
        QString tex_text = QString("Textures: ") + QString::number(Renderer::m_pimpl->GetNumTextures());
        QString userid_text = QString("UserID: ") + multi_players->GetUserID();

        if (player->GetSpeaking()) {
            userid_text += " (Speaking)";
        }
        if (player->GetRecording()) {
            userid_text += " (Recording)";
        }

        info2_text_geom.Clear();        

        if (!player->GetCursorObject(0).isEmpty()) {
            info2_text_geom.AddText(QString("js_id: ") + player->GetCursorObject(0));
        }
        else if (!player->GetCursorObject(1).isEmpty()) {
            info2_text_geom.AddText(QString("js_id: ") + player->GetCursorObject(1));
        }

        info2_text_geom.AddText(pos_text);
        info2_text_geom.AddText(xdir_text);
        info2_text_geom.AddText(ydir_text);
        info2_text_geom.AddText(zdir_text);                
        info2_text_geom.AddText(renderer_text);
        info2_text_geom.AddText(thread_text);

        if (current_render_fps >= 90) {
            info2_text_geom.AddText(fps_text3,QColor(0,255,0));
        }
        else if (current_render_fps >= 45) {
            info2_text_geom.AddText(fps_text3,QColor(255,255,0));
        }
        else {
            info2_text_geom.AddText(fps_text3,QColor(255,0,0));
        }

        info2_text_geom.AddText(tricount_text);
        info2_text_geom.AddText(tex_text);

        if (player->GetSpeaking() || player->GetRecording()) {
            info2_text_geom.AddText(userid_text, QColor(0,255,0));
        }
        else {
            info2_text_geom.AddText(userid_text);
        }

        info2_text_geom.AddText("");       

        const QString cur_url = r->GetProperties()->GetURL();
        QList <QPointer <ServerConnection> > & conn_list = multi_players->GetConnectionList();

        info2_text_geom.AddText("Server connections: " + QString::number(conn_list.size()));
        for (int i=0; i<conn_list.size(); ++i) {
            ServerConnection * s = conn_list[i];

            QColor col(255,0,0);
            if (s->logged_in) {
                col = QColor(0,255,0);
            }
            else if (s->logging_in) {
                col = QColor(255,255,0);
            }

            QString text = s->tcpserver + ":" + QString::number(s->tcpport) + "(";
            QString cipher = s->tcpsocket->sslConfiguration().sessionCipher().protocolString();
            if (!cipher.isEmpty()) {
                text += cipher + ",";
            }
            text += "janus) ";
            if (s->serverudpport > 0) {
                text += ":"+QString::number(s->serverudpport)+"(UDP)";
            }
            text += s->last_error_msg;
            info2_text_geom.AddText(text, col);

            const QList <QString> urls = s->rooms.keys();
            for (int j=0; j<urls.size(); ++j) {
                if (cur_url == urls[j]) {
                    info2_text_geom.AddText("*" + urls[j], QColor(178,178,255));
                }
                else if (s->rooms[urls[j]].sent_subscribe) {
                    info2_text_geom.AddText(" " + urls[j], QColor(102, 102, 255));
                }
                else {
                    info2_text_geom.AddText(" " + urls[j], QColor(51,51,255));
                }
            }
        }        
    }
}

void Game::DrawOverlaysGL()
{
    QPointer <AssetShader> shader = Room::GetTransparencyShader();
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }

    QPointer <Room> r = env->GetCurRoom();
    PerformanceLogger & perf_logger = r->GetPerformanceLogger();

    Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::DISABLED);
    Renderer::m_pimpl->SetDepthFunc(DepthFunc::ALWAYS);
    Renderer::m_pimpl->SetDepthMask(DepthMask::DEPTH_WRITES_ENABLED);

    shader->SetFogEnabled(false);
    shader->SetUseTextureAll(false);
    shader->SetUseTexture(0, true);
    shader->SetUseLighting(false);
    shader->SetConstColour(QVector4D(1,1,1,1));
    shader->SetChromaKeyColour(QVector4D(0,0,0,0));
    shader->SetUseSkelAnim(false);
    shader->SetUseClipPlane(false);

    const QVector3D ydir = player->GetProperties()->GetUpDir()->toQVector3D();
    const QVector3D zdir = player->GetProperties()->GetViewDir()->toQVector3D();
    const QVector3D xdir = QVector3D::crossProduct(zdir, ydir).normalized();

    QMatrix4x4 m;
    m.setColumn(0, xdir);
    m.setColumn(1, ydir);
    m.setColumn(2, zdir);
    m.setColumn(3, player->GetProperties()->GetEyePoint());
    m.setRow(3, QVector4D(0,0,0,1));
    m.translate(-0.5f,0.25f,1);
    m.scale(0.5f, 0.5f, 0.5f);

    if (state == JVR_STATE_UNIT_TRANSLATE ||
            state == JVR_STATE_UNIT_ROTATE ||
            state == JVR_STATE_UNIT_SCALE ||
            state == JVR_STATE_UNIT_COLOUR ||
            state == JVR_STATE_UNIT_COLLISIONID ||
            state == JVR_STATE_UNIT_COLLISIONSCALE ||
            state == JVR_STATE_UNIT_LIGHTING ||
            state == JVR_STATE_UNIT_CULL_FACE ||
            state == JVR_STATE_UNIT_BLEND_SRC ||
            state == JVR_STATE_UNIT_BLEND_DEST ||
            state == JVR_STATE_UNIT_MIRROR ||
            state == JVR_STATE_SELECT_ASSET) {

        MathUtil::PushModelMatrix();
        MathUtil::MultModelMatrix(m);
        MathUtil::MultModelMatrix(info_text_geom.GetModelMatrix());
        info_text_geom.DrawGL(shader);
        MathUtil::PopModelMatrix();
    }
    else if (show_position_mode) {

        MathUtil::PushModelMatrix();
        MathUtil::MultModelMatrix(m);
        MathUtil::MultModelMatrix(info2_text_geom.GetModelMatrix());

        info2_text_geom.DrawGL(shader);

        MathUtil::ModelMatrix().scale(perf_logger.GetNumFrameSamples()/10.0f, -8.0f, 1.0f);
        MathUtil::ModelMatrix().translate(1.0f, - 1.5f, 0.0f);

        shader->SetUseCubeTextureAll(false);
        shader->SetUseTextureAll(false);
        shader->SetUseLighting(false);
        shader->SetUseTexture(0, true);
        shader->UpdateObjectUniforms();

        Renderer * renderer = Renderer::m_pimpl;

        auto shader_data = shader->GetARCData();
        renderer->BindTextureHandle(0, perf_logger.GetFrameSamplesTextureHandle());
        AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                renderer->GetPlanePrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetPlaneVAO(),
                                shader_data.m_program,
                                shader_data.m_frame,
                                shader_data.m_room,
                                shader_data.m_object,
                                shader_data.m_material,
                                renderer->GetCurrentlyBoundTextures(),
                                renderer->GetDefaultFaceCullMode(),
                                renderer->GetDepthFunc(),
                                renderer->GetDepthMask(),
                                renderer->GetStencilFunc(),
                                renderer->GetStencilOp(),
                                renderer->GetColorMask());
        renderer->PushAbstractRenderCommand(a);

        MathUtil::PopModelMatrix();
    }
    else if (player->GetSpeaking()) {
        const float s = SoundManager::GetMicLevel();

        MathUtil::PushModelMatrix();
        MathUtil::MultModelMatrix(m);
        MathUtil::ModelMatrix().scale(0.1f*(1.0f + s * 0.5f));
        shader->SetUseTexture(0, false);
        shader->UpdateObjectUniforms();
        SpinAnimation::DrawPlaneGL(shader, QColor(128,0,0));
        shader->SetUseTexture(0, true);
        MathUtil::PopModelMatrix();
    }
    else if (player->GetRecording()) {
        m.scale(0.25f);
        MathUtil::PushModelMatrix();
        MathUtil::MultModelMatrix(m);
        MathUtil::ModelMatrix().scale(0.1f*(1.0f));
        shader->SetUseTexture(0, false);
        shader->UpdateObjectUniforms();
        SpinAnimation::DrawPlaneGL(shader, QColor(192,0,0));
        shader->SetUseTexture(0, true);
        MathUtil::PopModelMatrix();
    }

    Renderer::m_pimpl->SetDepthFunc(DepthFunc::LEQUAL);
    Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::BACK);
}

QPointer <Player> Game::GetPlayer()
{
    return player;
}

void Game::StartResetPlayer()
{
    if (fadestate == FADE_NONE) {
        SoundManager::Play(SOUND_BACK, false, player->GetProperties()->GetPos()->toQVector3D(), 1.0f);
        fadestate = FADE_RESETPLAYER1;
        fade_time.start();
    }
}

void Game::StartResetPlayerForward()
{
    if (fadestate == FADE_NONE) {
        fadestate = FADE_FORWARD_PLAYER1;
        fade_time.start();
    }
}

void Game::StartTeleportPlayer()
{
    //qDebug() << "Game::StartTeleportPlayer";
    fadestate = FADE_TELEPORT1;
    fade_time.start();
}

void Game::StartReloadPortal()
{
    if (fadestate == FADE_NONE) {
        fadestate = FADE_RELOAD1;
        fade_time.start();
    }
}

void Game::SetDoExit(const bool b)
{
    //qDebug() << "Game::SetDoExit" << b;
    do_exit = b;
}

bool Game::GetDoExit() const
{
    return do_exit;
}

QString Game::GetGlobalUUID()
{
    const QString uuid = multi_players->GetUserID() + "-" + QString::number(global_uuid) + "-" + QString::number(QDateTime::currentDateTime().toTime_t());
    ++global_uuid;
    return uuid;
}

QPointer <Environment> Game::GetEnvironment()
{
    return env;
}

QPointer <VirtualMenu> Game::GetVirtualMenu()
{
    return virtualmenu;
}

QPointer <MultiPlayerManager> Game::GetMultiPlayerManager()
{
    return multi_players;
}

void Game::EditModeTranslate(QPointer <RoomObject> obj, const int x, const int y, const int z)
{
    QPointer <Room> r = env->GetCurRoom();
    if (r->GetProperties()->GetLocked()) {
        MathUtil::ErrorLog("Warning: cannot do translate, room.locked=true");
        return;
    }

    if (obj.isNull()) {
        return;
    }

    const float translate_amount = unit_translate_amount[unit_scale];
    const float theta = player->GetTheta();

    QVector3D player_xdir;
    const QVector3D player_ydir(0,1,0);
    QVector3D player_zdir;
    if (theta < 45.0f) {
        player_xdir = QVector3D(1, 0, 0);
        player_zdir = QVector3D(0, 0, -1);
    }
    else if (theta < 135.0f) {
        player_xdir = QVector3D(0, 0, 1);
        player_zdir = QVector3D(1, 0, 0);
    }
    else if (theta < 225.0f) {
        player_xdir = QVector3D(-1, 0, 0);
        player_zdir = QVector3D(0, 0, 1);
    }
    else if (theta < 315.0f) {
        player_xdir = QVector3D(0, 0, -1);
        player_zdir = QVector3D(-1, 0, 0);
    }
    else {
        player_xdir = QVector3D(1, 0, 0);
        player_zdir = QVector3D(0, 0, -1);
    }

    const QVector3D last_pos = obj->GetProperties()->GetPos()->toQVector3D();

    const float snap_x = float(int(roundf(last_pos.x() / translate_amount))) * translate_amount;
    const float snap_y = float(int(roundf(last_pos.y() / translate_amount))) * translate_amount;
    const float snap_z = float(int(roundf(last_pos.z() / translate_amount))) * translate_amount;

    obj->GetProperties()->SetInterpolate(true);
    obj->SetInterpolation();

    obj->GetProperties()->SetPos(QVector3D(snap_x, snap_y, snap_z)
                + player_xdir * x * translate_amount
                + player_ydir * y * translate_amount
                + player_zdir * z * translate_amount);
    obj->GetProperties()->SetSync(true);
}

void Game::EditModeRotate(QPointer<RoomObject> obj, const int x, const int y, const int z)
{
    QPointer <Room> r = env->GetCurRoom();
    if (r->GetProperties()->GetLocked()) {
        MathUtil::ErrorLog("Warning: cannot do rotate, room.locked=true");
        return;
    }

    if (obj.isNull()) {
        return;
    }

    const float rot_deg = unit_rotate_amount[unit_scale];
    const float theta = player->GetTheta();

    QVector3D player_xdir;
    const QVector3D player_ydir(0,1,0);
    QVector3D player_zdir;
    if (theta < 45.0f) {
        player_xdir = QVector3D(-1, 0, 0);
        player_zdir = QVector3D(0, 0, 1);
    }
    else if (theta < 135.0f) {
        player_xdir = QVector3D(0, 0, -1);
        player_zdir = QVector3D(-1, 0, 0);
    }
    else if (theta < 225.0f) {
        player_xdir = QVector3D(1, 0, 0);
        player_zdir = QVector3D(0, 0, -1);
    }
    else if (theta < 315.0f) {
        player_xdir = QVector3D(0, 0, 1);
        player_zdir = QVector3D(1, 0, 0);
    }
    else {
        player_xdir = QVector3D(-1, 0, 0);
        player_zdir = QVector3D(0, 0, 1);
    }

    QVector3D xdir = obj->GetProperties()->GetXDir()->toQVector3D();
    QVector3D ydir = obj->GetProperties()->GetYDir()->toQVector3D();
    QVector3D zdir = obj->GetProperties()->GetZDir()->toQVector3D();

    xdir.normalize();
    ydir.normalize();
    zdir.normalize();

    const float x_rotate_rad = x * rot_deg * MathUtil::_PI_OVER_180;
    const float y_rotate_rad = y * rot_deg * MathUtil::_PI_OVER_180;
    const float z_rotate_rad = z * rot_deg * MathUtil::_PI_OVER_180;

    if (x != 0) {
        xdir = MathUtil::GetRotatedAxis(x_rotate_rad, xdir, player_ydir);
        ydir = MathUtil::GetRotatedAxis(x_rotate_rad, ydir, player_ydir);
        zdir = MathUtil::GetRotatedAxis(x_rotate_rad, zdir, player_ydir);
    }

    if (y != 0) {
        xdir = MathUtil::GetRotatedAxis(y_rotate_rad, xdir, player_xdir);
        ydir = MathUtil::GetRotatedAxis(y_rotate_rad, ydir, player_xdir);
        zdir = MathUtil::GetRotatedAxis(y_rotate_rad, zdir, player_xdir);
    }

    if (z != 0) {
        xdir = MathUtil::GetRotatedAxis(z_rotate_rad, xdir, player_zdir);
        ydir = MathUtil::GetRotatedAxis(z_rotate_rad, ydir, player_zdir);
        zdir = MathUtil::GetRotatedAxis(z_rotate_rad, zdir, player_zdir);
    }

    obj->GetProperties()->SetInterpolate(false);
    obj->SetInterpolation();

    obj->GetProperties()->SetXDirs(xdir, ydir, zdir);
    if (unit_scale == 0) {
        obj->SnapXDirsToMajorAxis();
    }
    obj->GetProperties()->SetSync(true);
    obj->GetProperties()->SetInterpolate(true);
}

void Game::EditModeScale(QPointer<RoomObject> obj, const int x, const int y, const int z)
{
    QPointer <Room> r = env->GetCurRoom();
    if (r->GetProperties()->GetLocked()) {
        MathUtil::ErrorLog("Warning: cannot do scale, room.locked=true");
        return;
    }

    if (obj.isNull()) {
        return;
    }

    const float scale_amount = unit_scale_amount[unit_scale];
    const QVector3D cur_scale = obj->GetProperties()->GetScale()->toQVector3D();

    const float snap_x = float(int(roundf(cur_scale.x() / scale_amount))) * scale_amount;
    const float snap_y = float(int(roundf(cur_scale.y() / scale_amount))) * scale_amount;
    const float snap_z = float(int(roundf(cur_scale.z() / scale_amount))) * scale_amount;

    const float new_x = ((snap_x + scale_amount * x) > 0.0f) ? snap_x + scale_amount * x : obj->GetScale().x();
    const float new_y = ((snap_y + scale_amount * y) > 0.0f) ? snap_y + scale_amount * y : obj->GetScale().y();
    const float new_z = ((snap_z + scale_amount * z) > 0.0f) ? snap_z + scale_amount * z : obj->GetScale().z();

    obj->GetProperties()->SetInterpolate(true);
    obj->SetInterpolation();

    obj->GetProperties()->SetScale(QVector3D(new_x, new_y, new_z));
    obj->GetProperties()->SetSync(true);
}

void Game::EditModeCollisionScale(QPointer<RoomObject> obj, const int x, const int y, const int z)
{
    QPointer <Room> r = env->GetCurRoom();
    if (r->GetProperties()->GetLocked()) {
        MathUtil::ErrorLog("Warning: cannot do collision scale, room.locked=true");
        return;
    }

    if (obj.isNull()) {
        return;
    }

    const float scale_amount = unit_scale_amount[unit_scale];
    const QVector3D cs = obj->GetProperties()->GetCollisionScale()->toQVector3D();

    const float snap_x = float(int(roundf(cs.x() / scale_amount))) * scale_amount;
    const float snap_y = float(int(roundf(cs.y() / scale_amount))) * scale_amount;
    const float snap_z = float(int(roundf(cs.z() / scale_amount))) * scale_amount;

    const float new_x = ((snap_x + scale_amount * x) > 0.0f) ? snap_x + scale_amount * x : cs.x();
    const float new_y = ((snap_y + scale_amount * y) > 0.0f) ? snap_y + scale_amount * y : cs.y();
    const float new_z = ((snap_z + scale_amount * z) > 0.0f) ? snap_z + scale_amount * z : cs.z();

    obj->GetProperties()->SetCollisionScale(QVector3D(new_x, new_y, new_z));
    obj->GetProperties()->SetSync(true);
}

void Game::ClearSelection(const int cursor_index)
{
    QPointer <Room> r = env->GetCurRoom();

    //57.1 move mouse to top left corner, to fix tooltips sticking around on defocus
    if (websurface_selected[cursor_index]) {
        websurface_selected[cursor_index].clear();
    }
    if (video_selected[cursor_index]) {
        video_selected[cursor_index].clear();
    }

    undo_object[cursor_index].clear();
    //59.6 - don't clear the copyselected string
    //copy_selected.clear();

    if (r) {
        r->SetSelected(selected[cursor_index], false);
    }
    selected[cursor_index].clear();
}

QPointer <AssetWebSurface> Game::GetWebSurfaceSelected()
{
    if (websurface_selected[0]) {
        return websurface_selected[0];
    }
    else if (websurface_selected[1]) {
        return websurface_selected[1];
    }
    return QPointer <AssetWebSurface> ();
}

void Game::SetWindowSize(const QSize s)
{
    win_size = s;
}

void Game::CreateNewWorkspace(const QString path)
{    
    //create directory structure for it
    QDir path_dir(path);
    if (!path_dir.exists()) {
        path_dir.mkpath(".");
    }

    //create new path
    const QString abs_path = path + "/index.html";
    const QUrl portal_url = QUrl::fromLocalFile(abs_path);

    QPointer <Room> r = new Room();
    r->GetProperties()->SetURL(abs_path);
    r->Create_Default_Workspace();
    r->SaveXML(abs_path);
    delete r;

    //generate blank/default template room and save
    //we always load the portal locally
    CreatePortal(portal_url, false);

    //do portal for multiplayer
    SoundManager::Play(SOUND_SAVED, false, player->GetProperties()->GetPos()->toQVector3D(), 1.0f);
    bookmarks->AddWorkspace(abs_path);
}

void Game::UpdateControllers()
{
    controller_manager->SetHapticsEnabled(SettingsManager::GetHapticsEnabled());
    controller_manager->Update(SettingsManager::GetGamepadEnabled());

    ControllerState * s = controller_manager->GetStates();

    QPointer <Room> r = env->GetCurRoom();

    //Update speaking for push-to-talk (V key or controller button)
    bool controller_held_for_speaking = false;
    if (controller_manager->GetUsingGamepad() && SettingsManager::GetGamepadEnabled()) {
        //speak
        if (s[1].b[0].proc_press) {
            s[1].b[0].proc_press = false;
        }
        if (s[1].b[0].proc_release) {
            s[1].b[0].proc_release = false;
        }
        if (s[1].b[0].pressed) {
            controller_held_for_speaking = true;
        }
    }
    else if (controller_manager->GetUsingSpatiallyTrackedControllers()) {
        //speak
        for (int i=0; i<2; ++i) {
            if (s[i].b[2].proc_press) {
                s[i].b[2].proc_press = false;
            }
            if (s[i].b[2].proc_release) {
                s[i].b[2].proc_release = false;
            }
            if (s[i].b[2].pressed) {
                controller_held_for_speaking = true;
            }
        }
    }

    const bool key_held_for_speaking = (keys[Qt::Key_V] && !keys[Qt::Key_Control]);
    const bool speaking = (key_held_for_speaking || controller_held_for_speaking);
    if (SoundManager::GetCaptureDeviceEnabled() &&
            !SettingsManager::GetMicAlwaysOn() &&
            speaking != player->GetSpeaking()) {
        player->SetSpeaking(speaking);
    }

    //59.7 - Updated controller player states
    if (controller_manager->GetUsingSpatiallyTrackedControllers()) {
        if (player->GetHMDType() == "rift") {
            QPointer <QObject> c = qvariant_cast<QObject *>(player->GetProperties()->property("touch"));
            if (c) {
                c->setProperty("left_stick_x", s[0].x);
                c->setProperty("left_stick_y", s[0].y);
                c->setProperty("left_trigger", s[0].t[0].value);
                c->setProperty("left_grip", s[0].t[1].value);
                c->setProperty("left_stick_click", s[0].b[2].value);
                c->setProperty("left_menu", s[0].b[3].value);

                c->setProperty("right_stick_x", s[1].x);
                c->setProperty("right_stick_y", s[1].y);
                c->setProperty("right_trigger", s[1].t[0].value);
                c->setProperty("right_grip", s[1].t[1].value);
                c->setProperty("right_stick_click", s[1].b[2].value);
                //note: c->right_menu is reserved by Oculus - against terms of use

                c->setProperty("button_x", s[0].b[0].value);
                c->setProperty("button_y", s[0].b[1].value);
                c->setProperty("button_a", s[1].b[0].value);
                c->setProperty("button_b", s[1].b[1].value);
            }
        }
        else if (player->GetHMDType() == "vive") {
            QPointer <QObject> c = qvariant_cast<QObject *>(player->GetProperties()->property("vive"));
            if (c) {
                c->setProperty("left_trackpad_x", s[0].x);
                c->setProperty("left_trackpad_y", s[0].y);
                c->setProperty("left_trackpad_click", s[0].b[0].value);
                c->setProperty("left_menu", s[0].b[1].value);
                c->setProperty("left_trigger", s[0].t[0].value);
                c->setProperty("left_grip", s[0].t[1].value);

                c->setProperty("right_trackpad_x", s[1].x);
                c->setProperty("right_trackpad_y", s[1].y);
                c->setProperty("right_trackpad_click", s[1].b[0].value);
                c->setProperty("right_menu", s[1].b[1].value);
                c->setProperty("right_trigger", s[1].t[0].value);
                c->setProperty("right_grip", s[1].t[1].value);
            }
        }
        else if (player->GetHMDType() == "wmxr") {
            QPointer <QObject> c = qvariant_cast<QObject *>(player->GetProperties()->property("wmxr"));
            if (c) {
                c->setProperty("left_stick_x", s[0].x);
                c->setProperty("left_stick_y", s[0].y);
                c->setProperty("left_trackpad_x", s[0].x1);
                c->setProperty("left_trackpad_y", s[0].y1);
                c->setProperty("left_trigger", s[0].t[0].value);
                c->setProperty("left_grip", s[0].t[1].value);
                c->setProperty("left_trackpad_click", s[0].b[0].value);
                c->setProperty("left_menu", s[0].b[1].value);

                c->setProperty("right_stick_x", s[1].x);
                c->setProperty("right_stick_y", s[1].y);
                c->setProperty("right_trackpad_x", s[1].x1);
                c->setProperty("right_trackpad_y", s[1].y1);
                c->setProperty("right_trigger", s[1].t[0].value);
                c->setProperty("right_grip", s[1].t[1].value);
                c->setProperty("right_trackpad_click", s[1].b[0].value);
                c->setProperty("right_menu", s[1].b[1].value);
            }
        }
    }
    
    //Note: 59.7 - currently has to be one or the other, might we want to support both at once?
    else if (controller_manager->GetUsingGamepad()) { 
        QPointer <QObject> c = qvariant_cast<QObject *>(player->GetProperties()->property("xbox"));
        if (c) {
            c->setProperty("connected", controller_manager->GetUsingGamepad());
            c->setProperty("left_stick_x", s[0].x);
            c->setProperty("left_stick_y", s[0].y);
            c->setProperty("left_trigger", s[0].t[0].value);

            c->setProperty("right_stick_x", s[1].x);
            c->setProperty("right_stick_y", s[1].y);
            c->setProperty("right_trigger", s[1].t[0].value);

            c->setProperty("button_x", s[0].b[1].value);
            c->setProperty("button_y", s[0].b[0].value);
            c->setProperty("button_select", s[0].b[2].value);
            c->setProperty("button_start", s[0].b[3].value);
            c->setProperty("left_shoulder", s[0].b[4].value);
            c->setProperty("right_shoulder", s[0].b[5].value);
            c->setProperty("left_stick_click", s[0].b[6].value);

            c->setProperty("button_b", s[1].b[0].value);
            c->setProperty("button_a", s[1].b[1].value);
            c->setProperty("dpad_left", s[1].b[2].value);
            c->setProperty("dpad_up", s[1].b[3].value);
            c->setProperty("dpad_right", s[1].b[4].value);
            c->setProperty("dpad_down", s[1].b[5].value);
            c->setProperty("right_stick_click", s[1].b[6].value);
        }
    }

    //Note: 59.7 - currently has to be one or the other, might we want to support both at once?
    if (!controller_manager->GetUsingGamepad()) { 
        QPointer <QObject> c = qvariant_cast<QObject *>(player->GetProperties()->property("xbox"));
        if (c) {
            c->setProperty("connected", false);
        }
    }
    //59.7 - End Updated controller player states

    player->GetProperties()->SetHand0Trackpad(QVector3D(s[0].x, s[0].y, 0.0f));
    player->GetProperties()->SetHand1Trackpad(QVector3D(s[1].x, s[1].y, 0.0f));

    float x0 = s[0].x;
    float y0 = s[0].y;
    float x1 = s[1].x;
    float y1 = s[1].y;

    const float snapturn_axis_threshold = 0.8f;
    const float axis_threshold = 0.2f; //state["threshold"].toFloat();

    //don't use vive controllers for moving
    if (player->GetHMDType() == "vive" && !SettingsManager::GetViveTrackpadMovement()) {
        x0 = 0.0f;
        y0 = 0.0f;
        x1 = 0.0f;
        y1 = 0.0f;
    }

    //this should do lasers thing later
    if (controller_manager->GetUsingSpatiallyTrackedControllers()) {
        const bool room_has_fn_onmousedown = r->HasJSFunctionContains("room.onMouseDown", "room.preventDefault()");
        const bool room_has_fn_onmouseup = r->HasJSFunctionContains("room.onMouseUp", "room.preventDefault()");
        const bool room_has_fn_onclick = r->HasJSFunctionContains("room.onClick", "room.preventDefault()");

        for (int i=0; i<2; ++i) {
            ControllerButtonState & b_click = s[i].GetClick();
            ControllerButtonState & b_teleport = s[i].GetTeleport();
            ControllerButtonState & b_home = s[i].GetHome();
            ControllerButtonState & b_grab = s[i].GetGrab();

            player->SetCursorActive(b_click.hover || b_teleport.hover, i);

            float dist = FLT_MAX;
            QVector3D controller_dir;

            //button hover (assumed if pressing/releasing, always hovering, so no need to do redundant raycasting)
            //raycasting is also used for grabbing (we cast when button is held fully)

            const QMatrix4x4 m = s[i].xform;
            QMatrix4x4 m2 = player->GetTransform() * m;
            //59.9 - angle the laser 15 degrees downward for spatially tracked controllers *other than* Oculus Touch
            if (player->GetHMDType() != "rift") {
                m2.rotate(-15.0f, 1, 0, 0);
            }
            m2.setColumn(2, -m2.column(2));
            controller_dir = m2.column(2).toVector3D().normalized();

            if (b_click.hover || b_teleport.hover || b_grab.pressed) {
                dist = UpdateCursorRaycast(m2, i);
            }

            if (b_click.hover || b_teleport.hover) {
                s[i].laser_length = ((dist != FLT_MAX) ? dist : 100.0f);
            }
            else {
                s[i].laser_length = 0.0f;
            }

            //qDebug() << i << dist << b_click.hover << b_teleport.hover << b_grab.hover << b_grab.pressed;

            //button press
            if (state == JVR_STATE_DEFAULT || state == JVR_STATE_INTERACT_TELEPORT) {

                //behaviour for C0
                if (b_teleport.proc_press) {
                    b_teleport.proc_press = false;
                    StartOpInteractionTeleport(i);
                }

                if (b_teleport.proc_release) {
                    b_teleport.proc_release = false;
                    EndOpInteractionTeleport(i);
                }

                if (b_click.proc_press) {
                    b_click.proc_press = false;

                    r->CallJSFunction("room.onMouseDown", player, multi_players);

                    if (!room_has_fn_onmousedown) {
                        StartOpInteractionDefault(i);
                    }
                }

                if (b_click.proc_release) {
                    b_click.proc_release = false;

                    //room onclick
                    r->CallJSFunction("room.onMouseUp", player, multi_players);
                    r->CallJSFunction("room.onClick", player, multi_players);

                    if (!room_has_fn_onmouseup && !room_has_fn_onclick) {
                        EndOpInteractionDefault(i);
                    }
                    else {
                        state = JVR_STATE_DEFAULT;
                    }
                }

                if (b_home.proc_press) {
                    b_home.proc_press = false;
                }
                if (b_home.proc_release) {
                    b_home.proc_release = false;
                    virtualmenu->MenuButtonPressed();
                }

                //let player grab stuff close by
                if (b_grab.proc_press) {
                    b_grab.proc_press = false;
                    if (dist < r->GetProperties()->GetGrabDist()) {
                        StartOpSpatialControllerEdit(i);
                    }
                }
                if (b_grab.proc_release) {
                    b_grab.proc_release = false;
                    EndOpSpatialControllerEdit(i);
                }
            }
        }
    }
    else if (controller_manager->GetUsingGamepad() && SettingsManager::GetGamepadEnabled()) {

        const bool room_has_fn_onmousedown = r->HasJSFunctionContains("room.onMouseDown", "room.preventDefault()");
        const bool room_has_fn_onmouseup = r->HasJSFunctionContains("room.onMouseUp", "room.preventDefault()");
        const bool room_has_fn_onclick = r->HasJSFunctionContains("room.onClick", "room.preventDefault()");

        //LMB
        if (s[1].t[0].proc_press) {
            s[1].t[0].proc_press = false;
            r->CallJSFunction("room.onMouseDown", player, multi_players);

            if (!room_has_fn_onmousedown) {
                StartOpInteractionDefault(0);
            }
        }
        if (s[1].t[0].proc_release) {
            s[1].t[0].proc_release = false;

            //room onclick
            r->CallJSFunction("room.onMouseUp", player, multi_players);
            r->CallJSFunction("room.onClick", player, multi_players);

            if (!room_has_fn_onmouseup && !room_has_fn_onclick) {
                EndOpInteractionDefault(0);
            }
        }

        //RMB
        if (s[0].t[0].proc_press) {
            s[0].t[0].proc_press = false;
            StartOpInteractionTeleport(0);
        }
        if (s[0].t[0].proc_release) {
            s[0].t[0].proc_release = false;
            EndOpInteractionTeleport(0);
            state = JVR_STATE_DEFAULT;
        }

        //jump
        if (s[1].b[1].proc_press) {
            s[1].b[1].proc_press = false;
            SetGamepadButtonPress(true);
            QKeyEvent e(keypress, Qt::Key_Space, 0);
            keyPressEvent(&e);
        }
        if (s[1].b[1].proc_release) {
            s[1].b[1].proc_release = false;
            SetGamepadButtonPress(true);
            QKeyEvent e(keypress, Qt::Key_Space, 0);
            keyReleaseEvent(&e);
        }

        //run
        if (s[0].b[1].proc_press) {
            s[0].b[1].proc_press = false;
            player->SetRunning(true);
        }
        if (s[0].b[1].proc_release) {
            s[0].b[1].proc_release = false;
            player->SetRunning(false);
        }
        if (s[0].b[6].proc_press) {
            s[0].b[6].proc_press = false;
            player->SetRunning(true);
        }
        if (s[0].b[6].proc_release) {
            s[0].b[6].proc_release = false;
            player->SetRunning(false);
        }

        //home button
        if (s[0].b[0].proc_release) {
            s[0].b[0].proc_release = false;
            virtualmenu->MenuButtonPressed();
        }

        //reset hmd
        if (s[0].b[2].proc_release) {
            s[0].b[2].proc_release = false;
            if (controller_manager->GetHMDManager()) {
                controller_manager->GetHMDManager()->ReCentre();
            }
        }

        //show menu
        if (s[0].b[3].proc_release) {
            s[0].b[3].proc_release = false;
        }

        //turn left
        if (s[0].b[4].proc_press) {
            s[0].b[4].proc_press = false;
            SetGamepadButtonPress(true);
            QKeyEvent e(keypress, Qt::Key_Q, 0);
            keyPressEvent(&e);
        }
        if (s[0].b[4].proc_release) {
            s[0].b[4].proc_release = false;
            SetGamepadButtonPress(true);
            QKeyEvent e(keypress, Qt::Key_Q, 0);
            keyReleaseEvent(&e);
        }

        //turn right
        if (s[0].b[5].proc_press) {
            s[0].b[5].proc_press = false;
            SetGamepadButtonPress(true);
            QKeyEvent e(keypress, Qt::Key_E, 0);
            keyPressEvent(&e);
        }
        if (s[0].b[5].proc_release) {
            s[0].b[5].proc_release = false;
            SetGamepadButtonPress(true);
            QKeyEvent e(keypress, Qt::Key_E, 0);
            keyReleaseEvent(&e);
        }

        //nav back
        if (s[1].b[2].proc_release) {
            s[1].b[2].proc_release = false;
            StartResetPlayer();
        }

        //flight
        if (s[1].b[3].proc_release) {
            s[1].b[3].proc_release = false;
            const bool b = !player->GetFlying();
            SoundManager::Play(b ? SOUND_FLIGHT : SOUND_NOFLIGHT, false, player->GetProperties()->GetPos()->toQVector3D(), 10.0f);
            player->SetFlying(b);
        }

        //refresh
        if (s[1].b[4].proc_release) {
            s[1].b[4].proc_release = false;
            env->ReloadRoom();
        }

        //chatlog
        if (s[1].b[5].proc_press) {
            s[1].b[5].proc_press = false;
            SetGamepadButtonPress(true);
            QKeyEvent e(keypress, Qt::Key_C, 0);
            keyPressEvent(&e);
        }
        if (s[1].b[5].proc_release) {
            s[1].b[5].proc_release = false;
            SetGamepadButtonPress(true);
            QKeyEvent e(keypress, Qt::Key_C, 0);
            keyReleaseEvent(&e);
        }
    }

    //WASD translation stuff
    if ( ( controller_manager->GetUsingGamepad() && SettingsManager::GetGamepadEnabled() ) || controller_manager->GetUsingSpatiallyTrackedControllers() ) {

        if (controller_x[0] >= -axis_threshold && x0 < -axis_threshold) {
            player->SetWalkLeft(true);
        }
        else if (controller_x[0] < -axis_threshold && x0 >= -axis_threshold) {
            player->SetWalkLeft(false);
        }

        if (controller_x[0] <= axis_threshold && x0 > axis_threshold) {
            player->SetWalkRight(true);
        }
        else if (controller_x[0] > axis_threshold && x0 <= axis_threshold) {
            player->SetWalkRight(false);
        }

        if (controller_y[0] <= axis_threshold && y0 > axis_threshold) {
            player->SetWalkForward(true);
        }
        else if (controller_y[0] > axis_threshold && y0 <= axis_threshold) {
            player->SetWalkForward(false);
        }

        if (controller_y[0] >= -axis_threshold && y0 < -axis_threshold) {
            player->SetWalkBack(true);
        }
        else if (controller_y[0] < -axis_threshold && y0 >= -axis_threshold) {
            player->SetWalkBack(false);
        }
    }

    const float sx = fabsf(x0) > axis_threshold ? fabsf(x0) : 1.0f; //scale speed based on stick axis position (or if its around centre, set to 1)
    const float sz = fabsf(y0) > axis_threshold ? fabsf(y0) : 1.0f;
    player->SetScaleVelX(sx);
    player->SetScaleVelY(sz);

    controller_x[0] = x0;
    controller_y[0] = y0;
    controller_x[1] = ((fabsf(x1) > axis_threshold) ? x1 : 0.0f);
    controller_y[1] = ((fabsf(y1) > axis_threshold) ? y1 : 0.0f);

    const bool state_can_walk = (state == JVR_STATE_DEFAULT || state == JVR_STATE_INTERACT_TELEPORT);

    //do player update stuff from within here (where autorepeating keys matters)
    if (state_can_walk && !player->GetEnteringText()) {
        player->DoSpinLeft((keys[Qt::Key_Q]) && !keys[Qt::Key_Control]);
        player->DoSpinRight((keys[Qt::Key_E]) && !keys[Qt::Key_Control]);
        player->SetFlyUp((keys[Qt::Key_Space]) && player->GetFlying());
        player->SetFlyDown((keys[Qt::Key_Shift]) && player->GetFlying());
        const bool do_jump = keys[Qt::Key_Space] && !player->GetFlying();
        player->SetJump(do_jump); //jumping stuff
        if (do_jump) {
            player->SetFollowMode(false);
            player->SetFollowModeUserID("");
        }
    }
    else {
        player->SetWalkForward(false);
        player->SetWalkBack(false);
        player->SetWalkLeft(false);
        player->SetWalkRight(false);
        player->DoSpinLeft(false);
        player->DoSpinRight(false);
        player->SetFlyUp(false);
        player->SetFlyDown(false);
        player->SetJump(false);
    }

    /*update controller axis stuff
    if (spatial_active[0] && spatial_active[1] && selected[0] == selected[1]) {
        QPointer <RoomObject> obj = env->GetEnvObject(env->GetPlayerRoom(), selected[0]);
        if (obj) {
            //both controllers manipulating one object
            QMatrix4x4 m, a, b0, b1, c0, c1, d0, d1, e, g0, g1;
            a = player->GetTransform();
            b0 = controller_manager->GetControllerTransform(0);
            b1 = controller_manager->GetControllerTransform(1);
            c0 = spatial_ctrltransform[0];
            c1 = spatial_ctrltransform[1];
            d0 = spatial_basetransform[0];
            const QVector3D base_scale = QVector3D(d0.column(0).toVector3D().length(),
                                             d0.column(1).toVector3D().length(),
                                             d0.column(2).toVector3D().length());
            QMatrix4x4 f0;
            f0.translate(0,0,-spatial_startf[0]);
            QMatrix4x4 f1;
            f1.translate(0,0,-spatial_startf[1]);
           const float base_dist = (((c0 * f0).column(3).toVector3D() -
                                      (c1 * f1).column(3).toVector3D())).length();
            const float cur_dist = (((b0 * f0).column(3).toVector3D() -
                                     (b1 * f1).column(3).toVector3D())).length();
            obj->SetScale(base_scale * cur_dist/base_dist);
            obj->SetSync(true);
        }
    }
    else {*/


    for (int i=0; i<2; ++i) {
        if (spatial_active[i]) {
            QPointer <RoomObject> obj = r->GetRoomObject(selected[i]);
            if (obj) {
                QMatrix4x4 m, a, b, c;
                a = player->GetTransform();
                b = controller_manager->GetStates()[i].xform;
                c = spatial_reltransform[i];
                m = a * b * c;
                //if (menu.GetHaptics()) {
                //    const int val = (obj->GetPos() - m.column(3).toVector3D()).length() * 4000.0f;
                //    controller_manager->TriggerHapticPulse(i, val);
                //}
                obj->GetProperties()->SetPos(m.column(3).toVector3D());
                obj->GetProperties()->SetXDir(m.column(0).toVector3D().normalized());
                obj->GetProperties()->SetYDir(m.column(1).toVector3D().normalized());
                obj->GetProperties()->SetZDir(m.column(2).toVector3D().normalized());
                obj->GetProperties()->SetSync(true);
            }
        }
    }
    //}

    const float rot_speed = SettingsManager::GetRotationSpeed();
    if (controller_x[1] != 0.0f) {
        if (SettingsManager::GetComfortMode()) {
            if (fabsf(controller_x[1]) > snapturn_axis_threshold && fabsf(last_controller_x[1]) < snapturn_axis_threshold) {
                if (controller_x[1] > 0.0f) {
                    player->SpinView(rot_speed, true);
                }
                else {
                    player->SpinView(-rot_speed, true);
                }
            }
        }
        else {
            player->SpinView(controller_x[1] * player->GetDeltaTime() * rot_speed, true);
        }
    }

    if (controller_y[1] != 0.0f) {
        if (GetMouseDoPitch()) {
            const float tilt_amount = controller_y[1] * player->GetDeltaTime() * rot_speed * (SettingsManager::GetInvertYEnabled() ? 1.0f : -1.0f);
            player->TiltView(tilt_amount);
        }
        else {
            if (websurface_selected[1]) {
                if (fabsf(controller_y[1]) > snapturn_axis_threshold) { // && fabsf(last_controller_y[1]) < snapturn_axis_threshold) {
                    QWheelEvent e2(QPointF(), controller_y[1] * 10.0f, 0, 0);
                    websurface_selected[1]->wheelEvent(&e2);
                }
            }
            else {
                if (controller_y[1] > snapturn_axis_threshold) { // && last_controller_y[1] < snapturn_axis_threshold) {
                    player->SetJump(true); //jumping stuff
                }
                else {
                    player->SetJump(false); //jumping stuff
                }
            }
        }
    }

    //update last controller position
    memcpy(last_controller_x, controller_x, 2 * sizeof(float));
    memcpy(last_controller_y, controller_y, 2 * sizeof(float));
}

void Game::UpdateVirtualMenu()
{    
    // Draw keyboard if HMD is in use, websurface selected, and text cursor is within a text-editable region
    //const bool text_entry = GetPlayerEnteringText();
    const bool text_entry = (websurface_selected[0] || websurface_selected[1]);
    player->SetEnteringText(text_entry);

    //update state of virtual keyboard if HMD is enabled
    if ((player->GetHMDEnabled() || controller_manager->GetUsingGamepad()) && text_entry && !virtualmenu->GetVisible()) { //show keyboard if hidden and needed
    //if (text_entry && !virtualmenu->GetVisible()) { //uncomment this and comment above for testing with mouse
        virtualmenu->SetMenuIndex(VirtualMenuIndex_KEYBOARD);
        virtualmenu->SetVisible(true);
        virtualmenu->ConstructSubmenus(); //65.11 - ensures keyboard RoomObject model matrices are updated

        if (websurface_selected[0]) { // && websurface_selected[0]->GetTextEditing()) {
            virtualmenu->SetWebSurface(websurface_selected[0]);
        }
        else if (websurface_selected[1]) { // && websurface_selected[1]->GetTextEditing()) {
            virtualmenu->SetWebSurface(websurface_selected[1]);
        }
    }

    virtualmenu->Update();

    if (virtualmenu->GetVisible()) {
        if (virtualmenu->GetDoBack()) {
            StartResetPlayer();
        }
        if (virtualmenu->GetDoForward()) {
            StartResetPlayerForward();
        }
        if (virtualmenu->GetDoReload()) {
            env->ReloadRoom();
        }
        if (virtualmenu->GetDoReset()) {            
            env->Reset();            
            SoundManager::Play(SOUND_RESET, false, player->GetProperties()->GetPos()->toQVector3D(), 10.0f);
        }
        if (virtualmenu->GetDoSetUserID()) {
            SetUserID(virtualmenu->GetEnteredUserID());
        }
        if (virtualmenu->GetDoExit()) {            
            SetDoExit(true);
        }
        if (virtualmenu->GetDoCreatePortal()) {
            CreatePortal(virtualmenu->GetDoCreatePortalURL(), true);
            virtualmenu->SetVisible(false);
        }
    }
    else {
        QVector3D z = -player->GetProperties()->GetViewDir()->toQVector3D(); //62.11 - spawn menu in view direction (important when using HMD)
        z.setY(0.0f);
        z.normalize();
        const QVector3D y(0,1,0);
        const QVector3D x = QVector3D::crossProduct(y, z).normalized();

        QVector3D p = player->GetProperties()->GetEyePoint();
        p.setY(player->GetProperties()->GetPos()->toQVector3D().y());

        QMatrix4x4 m;
        m.translate(p - z * 2.0f);
        m.setColumn(0, x);
        m.setColumn(1, y);
        m.setColumn(2, z);

        virtualmenu->SetModelMatrix(m);
    }
}

void Game::DrawVirtualMenu()
{
    if (virtualmenu->GetVisible() && !virtualmenu->GetTakingScreenshot()) {
        virtualmenu->DrawGL(Room::GetTransparencyShader());
    }

    if (GetPrivateWebsurfacesVisible()) {
        DrawPrivateWebsurfacesGL(Room::GetTransparencyShader());
    }
}

bool Game::SetSelected(QPointer <Room> room, const QString & selected, const bool b)
{
    if (b) {
        this->selected[0] = selected;
    }
    return room->SetSelected(selected, b);
}

QString Game::GetSelected(const int i)
{
    return selected[i];
}

void Game::RenameJSID(const QString & old_js_id, const QString & new_js_id)
{
    QPointer <Room> r = env->GetCurRoom();
    r->RenameJSID(old_js_id, new_js_id);

    //update thing user may be selecting
    if (GetSelected(0) == old_js_id) {
        r->SetSelected(new_js_id, true);
    }
}

void Game::SetGamepadButtonPress(const bool b)
{
    gamepad_button_press = b;
}

void Game::StartOpInteractionTeleport(const int )
{
    state = JVR_STATE_INTERACT_TELEPORT;
    teleport_held_time.start();
}

void Game::EndOpInteractionTeleport(const int i)
{
    //qDebug() << "Game::EndOpInteractionTeleport" << i << GetAllowTeleport(i);
    QPointer <Room> r = env->GetCurRoom();
    QPointer <RoomObject> cursor_obj = r->GetRoomObject(player->GetCursorObject(i));

    if (GetAllowTeleport(i) && teleport_held_time.elapsed() > teleport_hold_time_required)
    {
        if (cursor_obj && cursor_obj->GetType() == TYPE_LINK) {
            //teleport through an open portal
            teleport_portal = cursor_obj;
            teleport_p.setY(qMax(teleport_p.y(), cursor_obj->GetPos().y()));
            StartTeleportPlayer();
        }
        else {
            teleport_portal.clear();
            StartTeleportPlayer();
        }
    }

    state = JVR_STATE_DEFAULT;
}

QPointer <RoomObject> Game::CreatePortal(const QUrl url, const bool send_multi)
{
    //qDebug() << "Game::CreatePortal" << url << send_multi;
    QPointer <Room> r = env->GetCurRoom();
    QPointer <RoomObject> new_portal;
    if (r && !url.isEmpty()) {
        QVector3D d = player->GetProperties()->GetDir()->toQVector3D();
        d.setY(0.0f);
        d.normalize();

        const QVector3D p = player->GetProperties()->GetPos()->toQVector3D() + d * 1.5f;
        const QString portal_jsid = multi_players->GetUserID() + "-" + url.toString();
        ++global_uuid;

        //set up child stuff for portal
        QColor child_col;
        child_col.setHsl(int(30.0f * r->GetRoomObjects().size()), 128, 128);

        new_portal = new RoomObject();
        new_portal->SetType(TYPE_LINK);
        new_portal->SetURL("", url.toString());
        new_portal->GetProperties()->SetPos(p);
        new_portal->SetDir(-d);
        new_portal->GetProperties()->SetColour(MathUtil::GetColourAsVector4(child_col));
        new_portal->GetProperties()->SetScale(QVector3D(1.8f, 2.5f, 1.0f));
        new_portal->GetProperties()->SetCircular(true);
        new_portal->GetProperties()->SetJSID(portal_jsid);
        new_portal->GetProperties()->SetOpen(true);
        r->AddRoomObject(new_portal);
        env->AddRoom(new_portal);

        if (send_multi) {
            multi_players->SetSendPortal(url.toString(), portal_jsid);
        }

        new_portal->PlayCreatePortal();
    }
    return new_portal;
}

void Game::StartOpInteractionDefault(const int i)
{
    QPointer <Room> r = env->GetCurRoom();
    QPointer <RoomObject> o = r->GetRoomObject(player->GetCursorObject(i));

    //qDebug() << "player->getcursorobject" << player->GetCursorObject(cursor_index) << websurface_selected[cursor_index] << menu.GetWebpage();    
    if (virtualmenu->GetVisible()) {
        virtualmenu->mousePressEvent(player->GetCursorObject(i));
    }

    if (o && o->GetType() == TYPE_OBJECT && o->GetAssetWebSurface()) {
        websurface_selected[i] = o->GetAssetWebSurface();

        //if websurface is still on about:blank, load it
        //if (websurface_selected[i]->GetURL() == "about:blank") {
        //    websurface_selected[i]->SetURL(websurface_selected[i]->GetOriginalURL());
        //}

        //TODO - control modifier interaction with websurface to generate portals
        QPoint cursor_pos(float(websurface_selected[i]->GetProperties()->GetWidth())*cursor_uv[i].x(),
                          float(websurface_selected[i]->GetProperties()->GetHeight())*cursor_uv[i].y());

        QMouseEvent e2(QEvent::MouseButtonPress, cursor_pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);

        //send a click through if there is no thumb/image id, or the surface is already cursoractive
        if (websurface_selected[i]->GetFocus() || (o->GetProperties()->GetThumbID().length() == 0 && o->GetProperties()->GetImageID().length() == 0)) {
            websurface_selected[i]->mousePressEvent(&e2, i);
        }
        if (!websurface_selected[i]->GetFocus()) {
            websurface_selected[i]->SetFocus(true); //58.0 - bugfix to make websurfaces active on click for rift/vive
        }
    }
    else if (o && (o->GetType() == TYPE_VIDEO || o->GetType() == TYPE_OBJECT) && o->GetAssetVideo()) {
        video_selected[i] = o->GetAssetVideo();
        video_selected[i]->SetCursorActive(o->GetMediaContext(), true);

        QPoint cursor_pos(float(video_selected[i]->GetWidth(o->GetMediaContext()))*cursor_uv[i].x(),
                          float(video_selected[i]->GetHeight(o->GetMediaContext()))*cursor_uv[i].y());
        QMouseEvent e2(QEvent::MouseButtonPress, cursor_pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        video_selected[i]->mousePressEvent(o->GetMediaContext(), &e2);
    }
    else if (websurface_selected[i]) {
        QPoint cursor_pos(float(websurface_selected[i]->GetProperties()->GetWidth())*cursor_uv[i].x(),
                          float(websurface_selected[i]->GetProperties()->GetHeight())*cursor_uv[i].y());
        QMouseEvent e2(QEvent::MouseButtonPress, cursor_pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        websurface_selected[i]->mousePressEvent(&e2, i);
    }

    //59.3 - make any websurfaces in the room cursor active false (60.0 - only if we didn't click keyboard) (62.0 - refactored into VirtualMenu)
    if (!(virtualmenu->GetVisible() && virtualmenu->GetMenuIndex() == VirtualMenuIndex_KEYBOARD)) {
        for (QPointer <AssetWebSurface> & aw : r->GetAssetWebSurfaces()) {
            if (aw && aw != websurface_selected[i]) {
                aw->SetFocus(false);
            }
        }
    }
}

void Game::EndOpInteractionDefault(const int i)
{
    QPointer <Room> r = env->GetCurRoom();
    const QString curs = player->GetCursorObject(i);
    QPointer <RoomObject> o = r->GetRoomObject(curs);

    if (virtualmenu->GetVisible()) {
        virtualmenu->mouseReleaseEvent(player->GetCursorObject(i));
    }

    if (o) {
        //55.2 - onclick is on mouse release, and should only happen once per mouse click
        QString click_code = o->GetProperties()->GetOnClick();
        //qDebug() << "onclick code" << click_code;
        if (click_code.length() > 0) { //special javascript onclick code to run
            r->CallJSFunction(click_code, player, multi_players);
        }

        //default stuff
        if (o->GetType() == TYPE_LINK && o->GetProperties()->GetActive()) {
            //if portal is closed and click: open
            //hold-click: open and teleport
            if (!o->GetProperties()->GetOpen()) {
                env->AddRoom(o);
                o->GetProperties()->SetOpen(true);
            }
            else {
                if (env->ClearRoom(o)) {
                    o->GetProperties()->SetOpen(false);
                }
            }
        }
        else if (o->GetType() == TYPE_OBJECT && websurface_selected[i] && o->GetAssetWebSurface()) {

            //if websurface is still on about:blank, load it
            //if (QString::compare(websurface_selected[i]->GetURL(), "about:blank") == 0) {
            //    websurface_selected[i]->SetURL(websurface_selected[i]->GetS("src"));
            //}

            //click
            QPoint cursor_pos(float(websurface_selected[i]->GetProperties()->GetWidth())*cursor_uv[i].x(),
                              float(websurface_selected[i]->GetProperties()->GetHeight())*cursor_uv[i].y());
            QMouseEvent e2(QEvent::MouseButtonRelease, cursor_pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            websurface_selected[i]->mouseReleaseEvent(&e2, i);
        }
        else if (video_selected[i] && (o->GetType() == TYPE_VIDEO || o->GetType() == TYPE_OBJECT) && o->GetAssetVideo()) {
            QPoint cursor_pos(float(video_selected[i]->GetWidth(o->GetMediaContext()))*cursor_uv[i].x(),
                              float(video_selected[i]->GetHeight(o->GetMediaContext()))*cursor_uv[i].y());
            QMouseEvent e2(QEvent::MouseButtonRelease, cursor_pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            video_selected[i]->mouseReleaseEvent(o->GetMediaContext(), &e2);
        }
        else if (o->GetType() == TYPE_GHOST) { //clicking on ghosts
            multi_players->SetAvatarFromGhost(o); //change into ghost
            SoundManager::Play(SOUND_ASSIGNAVATAR, false, player->GetProperties()->GetPos()->toQVector3D(), 1.0f);
            //another option, play/pause it:
            //o->GetPlaying() ? o->Stop() : o->Play(); //pause/play ghost
        }
    }
    else if (websurface_selected[i]) {
        QPoint cursor_pos(float(websurface_selected[i]->GetProperties()->GetWidth())*cursor_uv[i].x(),
                          float(websurface_selected[i]->GetProperties()->GetHeight())*cursor_uv[i].y());
        QMouseEvent e2(QEvent::MouseButtonRelease, cursor_pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        websurface_selected[i]->mouseReleaseEvent(&e2, i);
    }
}

void Game::StartOpSpatialControllerEdit(const int i)
{
    //object being interacted with (if none, then we return without changing state)
    selected[i] = player->GetCursorObject(i);

    QPointer <Room> r = env->GetCurRoom();
    if (r->GetProperties()->GetLocked()) {
        MathUtil::ErrorLog("Warning: cannot do edit, object or room.locked=true");
        return;
    }

    //qDebug() << "Game::StartSpatialControllerEdit" << i << selected[i]; // << websurface_selected[i];
    bool did_select = r->SetSelected(selected[i], true);
    if (!did_select) {
        ClearSelection(i);
        return;
    }

    QPointer <RoomObject> obj = r->GetRoomObject(selected[i]);
    if (obj.isNull() || obj->GetProperties()->GetLocked()) {
        return;
    }

    //temporarily disable collision for this object until manipulation is complete
    r->GetPhysics()->RemoveRigidBody(obj);
    r->GetPhysics()->AddRigidBody(obj, COL_WALL, COL_WALL);

    // 53.13 fix drag and drop
    obj->Update(player->GetDeltaTime());
    obj->SetGrabbed(true);
    obj->SetSelected(true);

    const QMatrix4x4 m = (player->GetTransform() * controller_manager->GetStates()[i].xform).inverted() * obj->GetModelMatrixLocal();

    spatial_active[i] = true;
    spatial_reltransform[i] = m;
    spatial_basetransform[i] = obj->GetModelMatrixLocal();
    spatial_ctrltransform[i] = controller_manager->GetStates()[i].xform;
    spatial_startf[i] = (((player->GetTransform() * controller_manager->GetStates()[i].xform)).column(3).toVector3D() - player->GetCursorPos(i)).length();

    //allow objects to be passed between hands
    const int j = (i+1)%2;
    if (selected[i] == selected[j]) {
        EndOpSpatialControllerEdit(j);
    }
}

void Game::EndOpSpatialControllerEdit(const int i)
{
    //qDebug() << "Game::EndSpatialControllerEdit" << i;
    QPointer <Room> r = env->GetCurRoom();

    const int j = (i+1)%2;
    if (!(selected[i] == selected[j] && spatial_active[j])) { //we are not passing object between hands
        //reset the collision id
        QPointer <RoomObject> obj = r->GetRoomObject(selected[i]);
        if (obj) {
            r->GetPhysics()->RemoveRigidBody(obj);
            r->GetPhysics()->AddRigidBody(obj, COL_WALL, COL_PLAYER | COL_WALL);

            QVector3D v = (controller_manager->GetStates()[i].xform.column(3).toVector3D() -
                           controller_manager->GetStates()[i].xform_prev.column(3).toVector3D()) / player->GetDeltaTime();
            v = player->GetTransform().mapVector(v);

            r->GetPhysics()->SetLinearVelocity(obj, v);
            obj->GetProperties()->SetVel(v);
            obj->SetGrabbed(false);
            obj->SetSelected(false);
        }
    }

    spatial_active[i] = false;
    state = JVR_STATE_DEFAULT;
    selected[i].clear();
}


void Game::SaveScreenThumb(const QString out_filename)
{
    const int w = win_size.width();
    const int h = win_size.height();

    uchar * data = new uchar[w*h*4];
    int img_dimen = qMin(w,h);

    QImage img(data, img_dimen, img_dimen, QImage::Format_RGBA8888);
    if (w >= h) {
        MathUtil::glFuncs->glReadPixels(w/2-img_dimen/2,0, img_dimen, img_dimen, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }
    else {
        MathUtil::glFuncs->glReadPixels(0, h/2-img_dimen/2,img_dimen, img_dimen, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    img = img.mirrored();
    img = img.scaledToWidth(256, Qt::SmoothTransformation);
    img.save(out_filename, "jpg", 90);

    delete [] data;

    qDebug() << "Game::SaveScreenThumb() - image" << out_filename << "saved";
}

QString Game::GetRoomScreenshotPath(QPointer <Room> r)
{
    if (r) {
        return MathUtil::GetScreenshotPath() + MathUtil::MD5Hash(r->GetProperties()->GetURL()) + ".jpg";
    }
    return MathUtil::GetScreenshotPath() + "null.jpg";
}

void Game::SaveBookmark()
{
    QPointer <Room> r = env->GetCurRoom();

    const QString url = r->GetProperties()->GetURL();
    const QString title = r->GetProperties()->GetTitle();
    const QVector3D pos = player->GetProperties()->GetPos()->toQVector3D();

    if (bookmarks->GetBookmarked(url)) {
        bookmarks->RemoveBookmark(url);
        SoundManager::Play(SOUND_DELETING, false, pos, 1.0f);
    }
    else {
        const QString thumb_url = QUrl::fromLocalFile(GetRoomScreenshotPath(r)).toString();
        bookmarks->AddBookmark(url, title, thumb_url);
        SoundManager::Play(SOUND_SAVED, false, pos, 1.0f);
    }

    bookmarks->SaveBookmarks();
}

QPointer <Asset> Game::CreateAssetFromURL(const QString url_str)
{
    //infer asset type from filename extension
    //60.0 - be sure to use just the filename only, as this filters out blah.txt?v=2311283 and so on
    const ElementType t = MathUtil::AssetTypeFromFilename(QUrl(url_str).fileName());
    //qDebug() << "Game::CreateAssetFromURL" << url_str << AssetTypeToString(t);

    QPointer <Asset> new_asset;
    if (t == TYPE_ASSETGHOST) {
        new_asset = new AssetGhost();
    }
    else if (t == TYPE_ASSETIMAGE) {
        new_asset = new AssetImage();
    }
    else if (t == TYPE_ASSETOBJECT) {
        new_asset = new AssetObject();
    }
    else if (t == TYPE_ASSETRECORDING) {
        new_asset = new AssetRecording();
    }
    else if (t == TYPE_ASSETSCRIPT) {
        new_asset = new AssetScript(env->GetCurRoom());
    }
    else if (t == TYPE_ASSETSHADER) {
        new_asset = new AssetShader();
    }
    else if (t == TYPE_ASSETSOUND) {
        new_asset = new AssetSound();
    }
    else if (t == TYPE_ASSETVIDEO) {
        new_asset = new AssetVideo();
    }
    else if (t == TYPE_ASSETWEBSURFACE) {
        new_asset = new AssetWebSurface();
    }

    if (new_asset) {
        if (QUrl(url_str).isLocalFile() && QUrl(player->GetProperties()->GetURL()).isLocalFile()) {
            const QString file_path = QUrl(url_str).toLocalFile();
            const QString base_path = QDir(QUrl(player->GetProperties()->GetURL()).toLocalFile()).path();
            const QDir dir = QFileInfo(base_path).absoluteDir();
            const QString src = dir.relativeFilePath(file_path);
            new_asset->SetSrc(QUrl::fromLocalFile(base_path).toString(), src);
        }
        else {
            new_asset->SetSrc(url_str, url_str);
        }
        new_asset->GetProperties()->SetID(url_str);
    }

    return new_asset;
}

void Game::UpdateDragAndDropPosition(QPointer <RoomObject> o, const int cursor_index)
{
    if (o.isNull()) {
        return;
    }

    QMatrix4x4 xform;

    if (o->GetType() == TYPE_OBJECT && o->GetProperties()->GetID() == "plane") {
        xform.setColumn(3, player->GetCursorPos(cursor_index) + player->GetCursorZDir(cursor_index) * 0.1f);
    }
    else {
        xform.setColumn(3, player->GetCursorPos(cursor_index));
    }

    if (o->GetType() == TYPE_OBJECT && o->GetProperties()->GetID() != "plane") { //make objects upright
        xform.setColumn(0, player->GetCursorYDir(cursor_index));
        xform.setColumn(1, player->GetCursorZDir(cursor_index));
        xform.setColumn(2, -player->GetCursorXDir(cursor_index));
    }
    else if (o->GetType() == TYPE_LINK) {
        xform.setColumn(0, player->GetCursorXDir(cursor_index));
        xform.setColumn(1, QVector3D(0,1,0));
        xform.setColumn(2, QVector3D::crossProduct(player->GetCursorXDir(cursor_index), QVector3D(0,1,0)).normalized());
    }
    else { //but make other planar surfaces conform to the surface
        xform.setColumn(0, player->GetCursorXDir(cursor_index));
        xform.setColumn(1, player->GetCursorYDir(cursor_index));
        xform.setColumn(2, player->GetCursorZDir(cursor_index));
    }

    xform.setRow(3, QVector4D(0,0,0,1));
    xform *= dragdrop_xform;

    o->GetProperties()->SetPos(xform.column(3).toVector3D());
    o->GetProperties()->SetXDirs(xform.column(0).toVector3D(),
                xform.column(1).toVector3D(),
                xform.column(2).toVector3D());
    o->GetProperties()->SetSync(true);
}

//drag and drop an already-defined asset in the scene
void Game::DragAndDropAssetObject(const QString id, const int i)
{
    //qDebug() << "Game::DragAndDrop" << url_str << drop_or_pin << i;
    QPointer <Room> r = env->GetCurRoom();
    //drag+drop
    if (r->GetProperties()->GetLocked()) {
        MathUtil::ErrorLog("Warning: cannot do drag and drop, room.locked=true");
        return;
    }

    QPointer <AssetObject> a = r->GetAssetObject(id);
    if (a.isNull()) {
        MathUtil::ErrorLog("Warning: cannot do drag and drop, assetobject with id does not exist: " + id);
        return;
    }

    ClearSelection(i);

    QPointer <RoomObject> new_object = new RoomObject();
    new_object->GetProperties()->SetJSID(GetGlobalUUID());

    new_object->SetType(TYPE_OBJECT);
    new_object->GetProperties()->SetID(id);
    new_object->GetProperties()->SetCollisionID(id);
    new_object->SetRescaleOnLoad(false); //rescale object to unit diam on load
    selected[i] = r->AddRoomObject(new_object);

    UpdateDragAndDropPosition(new_object, i);

    //qDebug() << "drag" << new_object->GetJSID() << new_object->GetOriginalURL() << new_object->GetWebSurfaceID() << new_object->GetUUID();

    //remove focus on websurface and initiate drag and drop
    if (controller_manager->GetUsingSpatiallyTrackedControllers()) {
        player->SetCursorObject(selected[i], i);
        StartOpSpatialControllerEdit(i);
    }
    else {
        mouse_move_accum = QPointF(0,0);

        state = JVR_STATE_DRAGDROP; //59.0 - @alu requested cursor raycasting for initial placement
        dragdrop_xform.setToIdentity();

        r->SetSelected(selected[i], true);
        player->SetCursorObject(selected[i], i);

        r->GetPhysics()->RemoveRigidBody(new_object);
        r->GetPhysics()->AddRigidBody(new_object, COL_WALL, COL_WALL);

        new_object->Update(player->GetDeltaTime());

        websurface_selected[i] = new_object->GetAssetWebSurface();
        if (websurface_selected[i]) {
            websurface_selected[i]->SetURL(websurface_selected[i]->GetProperties()->GetSrc());
        }
        video_selected[i] = new_object->GetAssetVideo();

        //qDebug() << video_selected[i]->GetFullURL();
        //qDebug() << websurface_selected[i]->GetFullURL();
    }

    new_object->GetProperties()->SetSync(true);

    if (new_object->GetAssetSound()){
        //new_object->GetAssetSound()->SetSrc(url_str, url_str);
        new_object->GetAssetSound()->Play(new_object->GetMediaContext());
    }
    if (new_object->GetAssetVideo()){
        //new_object->GetAssetVideo()->SetSrc(url_str, url_str);
        new_object->GetAssetVideo()->Play(new_object->GetMediaContext());
    }
}

void Game::DragAndDrop(const QString url_str, const QString drop_or_pin, const int i)
{
    //qDebug() << "Game::DragAndDrop" << url_str << drop_or_pin << i;
    QPointer <Room> r = env->GetCurRoom();
    //drag+drop
    if (r->GetProperties()->GetLocked()) {
        MathUtil::ErrorLog("Warning: cannot do drag and drop, room.locked=true");
        return;
    }

    QString asset_id;
    QPointer <Asset> new_asset = CreateAssetFromURL(url_str);

    if (new_asset) {
        asset_id = new_asset->GetProperties()->GetID();
        new_asset->GetProperties()->SetSync(true);        
    }

    ClearSelection(i);

    QPointer <RoomObject> new_object = new RoomObject();
    new_object->GetProperties()->SetJSID(GetGlobalUUID());

    //qDebug() << i << hit_result_rect << url_str;
    if (new_asset) {
        const ElementType t = new_asset->GetProperties()->GetType();
        if (t == TYPE_ASSETIMAGE) {
            r->AddAssetImage(dynamic_cast<AssetImage *>(new_asset.data()));
            dynamic_cast<AssetImage *>(new_asset.data())->Load();
            new_object->SetType(TYPE_OBJECT);
            new_object->GetProperties()->SetID("plane");
            new_object->GetProperties()->SetCollisionID("plane");
            new_object->GetProperties()->SetImageID(asset_id);
            new_object->GetProperties()->SetCullFace("none");
            new_object->GetProperties()->SetTexClamp(true);
            new_object->GetProperties()->SetLighting(false);
        }
        else if (t == TYPE_ASSETVIDEO) {
            r->AddAssetVideo(dynamic_cast<AssetVideo *>(new_asset.data()));
            new_object->SetAssetVideo(dynamic_cast<AssetVideo *>(new_asset.data()));
            new_object->SetType(TYPE_VIDEO);
            new_object->GetProperties()->SetID(asset_id);            
            new_object->GetProperties()->SetLighting(false);
        }
        else if (t == TYPE_ASSETSOUND) {
            r->AddAssetSound(dynamic_cast<AssetSound *>(new_asset.data()));
            new_object->SetAssetSound(dynamic_cast<AssetSound *>(new_asset.data()));
            new_object->SetType(TYPE_SOUND);
            new_object->GetProperties()->SetID(asset_id);
        }
        else if (t == TYPE_ASSETOBJECT) {
            r->AddAssetObject(dynamic_cast<AssetObject*>(new_asset.data()));
            dynamic_cast<AssetObject *>(new_asset.data())->Load();
            new_object->SetType(TYPE_OBJECT);
            new_object->GetProperties()->SetID(asset_id);
            new_object->GetProperties()->SetCollisionID(asset_id);
            new_object->SetRescaleOnLoad(false); //rescale object to unit diam on load
        }
        else if (t == TYPE_ASSETRECORDING) {
            r->AddAssetRecording(dynamic_cast<AssetRecording *>(new_asset.data()));
            new_asset->GetProperties()->SetAutoPlay(true);
        }
        else {
            //unknown, show it on a websurface
            r->AddAssetWebSurface(dynamic_cast<AssetWebSurface *>(new_asset.data()));
            new_object->SetType(TYPE_OBJECT);
            new_object->GetProperties()->SetID("plane");
            new_object->GetProperties()->SetCollisionID("plane");
            new_object->GetProperties()->SetWebsurfaceID(url_str);
            new_object->GetProperties()->SetCullFace("none");
            new_object->GetProperties()->SetLighting(false);
        }

        if (drop_or_pin == "Drag+Drop") {
            new_object->GetProperties()->SetCollisionStatic(false);
        }

        selected[i] = r->AddRoomObject(new_object);
    }
    else {
        //unassociated with asset, do it as a portal
        new_object->SetType(TYPE_LINK);
        new_object->SetURL(url_str, url_str);
        r->AddRoomObject(new_object);
        selected[i] = new_object->GetProperties()->GetJSID();
    }

    UpdateDragAndDropPosition(new_object, i);

    //qDebug() << "drag" << new_object->GetJSID() << new_object->GetOriginalURL() << new_object->GetWebSurfaceID() << new_object->GetUUID();

    //remove focus on websurface and initiate drag and drop
    if (controller_manager->GetUsingSpatiallyTrackedControllers()) {
        player->SetCursorObject(selected[i], i);
        StartOpSpatialControllerEdit(i);
    }
    else {
        mouse_move_accum = QPointF(0,0);
        if (new_asset && new_asset->GetProperties()->GetType() == TYPE_ASSETRECORDING) {
            state = JVR_STATE_DEFAULT;
        }
        else {
            state = JVR_STATE_DRAGDROP; //59.0 - @alu requested cursor raycasting for initial placement
            dragdrop_xform.setToIdentity();
        }

        r->SetSelected(selected[i], true);
        player->SetCursorObject(selected[i], i);

        r->GetPhysics()->RemoveRigidBody(new_object);
        r->GetPhysics()->AddRigidBody(new_object, COL_WALL, COL_WALL);

        new_object->Update(player->GetDeltaTime());

        websurface_selected[i] = new_object->GetAssetWebSurface();
        if (websurface_selected[i]) {
            websurface_selected[i]->SetURL(websurface_selected[i]->GetProperties()->GetSrc());
        }
        video_selected[i] = new_object->GetAssetVideo();

        //qDebug() << video_selected[i]->GetFullURL();
        //qDebug() << websurface_selected[i]->GetFullURL();
    }

    new_object->GetProperties()->SetSync(true);

    if (new_object->GetAssetSound()){
        //new_object->GetAssetSound()->SetSrc(url_str, url_str);
        new_object->GetAssetSound()->Play(new_object->GetMediaContext());
    }
    if (new_object->GetAssetVideo()){
        //new_object->GetAssetVideo()->SetSrc(url_str, url_str);
        new_object->GetAssetVideo()->Play(new_object->GetMediaContext());
    }
}

bool Game::GetRecording() const
{
    return multi_players->GetRecording();
}

void Game::StartRecording(const bool record_everyone)
{
    multi_players->SetRecording(true, record_everyone);
    SoundManager::Play(SOUND_RECORDGHOST, false, player->GetProperties()->GetPos()->toQVector3D(), 1.0f);
}

void Game::StopRecording()
{
    multi_players->SetRecording(false, true);
    SoundManager::Play(SOUND_GHOSTSAVED, false, player->GetProperties()->GetPos()->toQVector3D(), 1.0f);
}

void Game::DragAndDropFromWebsurface(const QString drop_or_pin, const int i)
{
    //QHash <QString, QPointer <RoomObject> > & envobjects = env->GetPlayerRoom()->GetEnvObjects();
    QPointer <Room> r = env->GetCurRoom();
    //drag+drop
    const bool is_room_locked = r->GetProperties()->GetLocked();
    const bool is_websurface_not_null = !websurface_selected[i].isNull();

    if (!is_room_locked && is_websurface_not_null) {

        QString url_str = websurface_selected[i]->GetLinkClicked(i).toString().trimmed();
        if (url_str == websurface_selected[i]->GetURL() || url_str == "") {
            return;
        }

        //remove stuff after ? (59.0 - but not if it's a Google link - contains /url?q=) (62.9 - versioning in Vesta)
        if (url_str.contains("?") && !url_str.contains("/url?q=") && !url_str.contains("?v=")) {
            url_str = url_str.left(url_str.indexOf("?"));
        }
        DragAndDrop(url_str, drop_or_pin, i);
    }
    else if (is_room_locked) {
        MathUtil::ErrorLog("Warning: cannot do drag and drop, room.locked=true");
    }
}

void Game::SetRoomDeleteCode(const QString s)
{
    multi_players->SetRoomDeleteCode(s);
}

void Game::SetUserID(const QString s)
{
    //qDebug() << "Game::SetUserID" << s;
    player->GetProperties()->SetUserID(s);

    if (!multi_players.isNull() && !env.isNull()) {
        QPointer <RoomObject> user_ghost = multi_players->GetPlayer();
        if (user_ghost) {
            user_ghost->GetProperties()->SetID(s);

            multi_players->DoUpdateAvatar();

            if (multi_players->GetEnabled()) {
                multi_players->SetEnabled(false);
                multi_players->SetEnabled(true);
            }
        }
    }
}

void Game::ResetAvatar()
{
    if (multi_players) {
        multi_players->LoadAvatarData(false);

        if (multi_players->GetEnabled()) {
            multi_players->SetEnabled(false);
            multi_players->SetEnabled(true);
        }
    }
}

void Game::ReloadAvatar()
{
    if (multi_players) {
        multi_players->LoadAvatarData(true);

        if (multi_players->GetEnabled()) {
            multi_players->SetEnabled(false);
            multi_players->SetEnabled(true);
        }
    }
}

void Game::SendChatMessage(const QString s)
{
    if (multi_players->SetChatMessage(player, s) && SoundManager::GetEnabled()) { //53.8 - spyduck doesn't like noise for commands
        SoundManager::Play(SOUND_POP2, false, player->GetProperties()->GetPos()->toQVector3D()+QVector3D(0,1,0), 1.0f);
    }

    //59.6 - restore #sync functionality
    if (s.toLower() == "#sync") {
        env->GetCurRoom()->SyncAll();
    }
}

void Game::UpdateAssetRecordings()
{
    QPointer <Room> r = env->GetCurRoom();
    for (QPointer <AssetRecording> & a : r->GetAssetRecordings()) {
        if (a && a->GetRoomID().isEmpty()) {
            const QString s = MathUtil::MD5Hash(r->GetProperties()->GetURL());
            a->SetRoomID(s);
            multi_players->AddAssetRecording(a);
        }
    }
}

void Game::DoImport(const QString url)
{
    QPointer <WebAsset> w = new WebAsset();
    w->Load(url);
    import_list.push_back(w);
}

void Game::UpdateImportList()
{
    for (int i=0; i<import_list.size(); ++i) {
        if (import_list[i].isNull()) {
            import_list.removeAt(i);
            --i;
        }
        else if (import_list[i] && import_list[i]->GetLoaded()) {
            env->GetCurRoom()->ImportCode(import_list[i]->GetData(), import_list[i]->GetURL().toString());
            delete import_list[i];
            import_list.removeAt(i);
            --i;
        }
    }
}

void Game::SetDrawCursor(const bool b)
{
    draw_cursor = b;
}

void Game::UpdateAudio()
{
    SoundManager::SetEnabled(SettingsManager::GetSoundsEnabled());
    SoundManager::SetGainMic(SettingsManager::GetVolumeMic());
    SoundManager::SetThresholdVolume(SettingsManager::GetMicSensitivity());

    SoundManager::Update(player);

    //add any input mic buffers
    if (!SoundManager::GetMicBuffers().isEmpty()) {
        multi_players->AddMicBuffers(SoundManager::GetMicBuffers());
        SoundManager::ClearMicBuffers();
    }

    //update threshold-based speaking
    if (SoundManager::GetCaptureDeviceEnabled() && SettingsManager::GetMicAlwaysOn()) {
        if (SoundManager::GetThresholdPast() && !player->GetSpeaking()) {
            //start if over threhsold and we are not speaking
            player->SetSpeaking(true);
        }
        else if (!SoundManager::GetThresholdPast() && player->GetSpeaking()) {
            //stop if under threshold and we are speaking (and for over 500 msec)
            player->SetSpeaking(false);
        }
    }
}

void Game::UpdateMultiplayer()
{
    multi_players->SetEnabled(SettingsManager::GetMultiplayerEnabled() && !do_exit);    
    multi_players->SetPartyMode(SettingsManager::GetPartyModeEnabled());

    QPointer <Room> r = env->GetCurRoom();
    if (r && multi_players) {
        //determine URL adjacency
        const QString room_url = r->GetProperties()->GetURL();
        const QString room_name = r->GetProperties()->GetTitle();
        const bool room_allows_party_mode = r->GetProperties()->GetPartyMode();
        QList <QString> adjacent_urls;
        QMap <QString, QPointer <Room> > visible_rooms = r->GetVisibleRooms();
        for (QPointer <Room> & r2 : visible_rooms) {
            if (r2 && r2 != r) {
                adjacent_urls.push_back(r2->GetProperties()->GetID());
            }
        }

        //59.4 - strip out URL anchor stuff from URL before passing to multiplayermanager
        //(so e.g. two people in room, one with anchor one without can still see each other - the MD5 hash will be the same)
        QString room_url_no_anchor = room_url;
        if (room_url_no_anchor.contains("#")) {
            room_url_no_anchor = room_url_no_anchor.left(room_url_no_anchor.indexOf("#"));
        }

        multi_players->Update(player, room_url_no_anchor, adjacent_urls, room_name, room_allows_party_mode, delta_time);
        QPointer <RoomObject> g = multi_players->GetPlayer();
        if (g) {
            g->Update(delta_time);
        }

        //any reset to make?
        if (multi_players->GetResetPlayer()) {
            if (fadestate == FADE_NONE) {
                fadestate = FADE_RESETPLAYER1;
                fade_time.restart();
            }
            else if (fadestate == FADE_RESETPLAYER2) {
                multi_players->SetResetPlayer(false);
            }
        }
    }   
}

void Game::UpdateAssets()
{
    env->UpdateAssets();
    SpinAnimation::UpdateAssets();
    RoomObject::UpdateAssets();   
}

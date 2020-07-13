#include "domnode.h"

int DOMNode::next_uuid = 0;

DOMNode::DOMNode(QObject *parent) : QObject(parent)
{
    uuid = NextUUID();
    js_id = QString::number(uuid);    
    pos = new ScriptableVector(0, 0, 0, this);
    xdir = new ScriptableVector(1, 0, 0, this);
    ydir = new ScriptableVector(0, 1, 0, this);
    zdir = new ScriptableVector(0, 0, 1, this);
    rotation = new ScriptableVector(0, 0, 0, this);
    rotation_order = "xyz";
    vel = new ScriptableVector(0, 0, 0, this);
    accel = new ScriptableVector(0, 0, 0, this);
    col = new ScriptableVector(1, 1, 1, 1, this);
    chromakey_col = new ScriptableVector(0, 0, 0, 0, this);
    text_col = new ScriptableVector(0, 0, 0, 1, this);
    back_col = new ScriptableVector(1, 1, 1, 1, this);
    scale = new ScriptableVector(1, 1, 1, this);
    tiling = new ScriptableVector(1, 1, 0, 0, this);
    rand_pos = new ScriptableVector(0, 0, 0, this);
    rand_vel = new ScriptableVector(0, 0, 0, this);
    rand_accel = new ScriptableVector(0, 0, 0, this);
    rand_col = new ScriptableVector(0, 0, 0, this);
    rand_scale = new ScriptableVector(0, 0, 0, this);
    lightmap_scale = new ScriptableVector(1, 1, 0, 0, this);
    emitter_pos = new ScriptableVector(0, 0, 0, this);
    blend0 = 0.0f;
    blend1 = 0.0f;
    blend2 = 0.0f;
    blend3 = 0.0f;
    loop = false;
    pitch = 1.0f;
    gain = 1.0f;
    doppler_factor = 1.0f;
    outer_gain = 0.0f;
    inner_angle = 360.0f;
    outer_angle = 360.0f;    
    font_size = 32;
    collision_pos = new ScriptableVector(0, 0, 0, this);
    collision_scale = new ScriptableVector(1, 1, 1, this);
    collision_radius = 0.0f;
    collision_response = true;
    collision_friction = 0.5f; // Roughly concrete
    collision_rollingfriction = 0.01f; // Roughly concrete
    collision_restitution = 0.85f; // Roughly concrete
    collision_angulardamping = 0.1f; // Emulate Air friction
    collision_lineardamping = 0.15f; // Emulate Air friction
    collision_static = true;
    collision_trigger = false;
    collision_ccdmotionthreshold = 1.0f;
    collision_ccdsweptsphereradius = 0.0f;
    text_changed = false;
    lighting = true;
    visible = true;
    blend_src = "one";
    blend_dest = "one_minus_src_alpha";
    cull_face = "back";
    emit_local = false;
    sync = false;
    anim_speed = 1.0f; //multiplier for speed
    auto_load = false;
    draw_text = true;
    open = false;
    mirror = false;
    active = true;
    count = 0;
    rate = 1;
    duration = 1.0f;
    fade_in = 1.0f;
    fade_out = 1.0f;
    dirty = false;
    current_time = 0.0f;
    total_time = 0.0f;
    spinaxis = new ScriptableVector(0, 1, 0, this);
    spinval = 0.0f;
    m_light_intensity = 1.0f;
    m_light_cone_angle = 0.0f;
    m_light_cone_exponent = 1.0f;
    m_light_range = 0.5f;
    m_draw_layer = 0;
    save_to_markup = true;
    primitive = false;
    locked = false;
    interpolate = false; //60.1 - false by default (can push user around otherwise, e.g. room loading geometry)
    circular = false;
    highlighted = false;    
    back_alpha = 1.0f;
    auto_load_triggered = false;
    triggered = false;
    play_once = false;
    started_auto_play = false;
    translator_busy = false;
    ready = false;
    ready_for_screenshot = false;
    teleport_override = false;
    reloaded = false;
    cur_mount = 0;
    url_changed = false;
    swallow = false;

    //assets
    tex.resize(ASSETSHADER_MAX_TEXTURE_UNITS);
    tex_alpha = "undefined";
    tex_colorspace = "sRGB";
    tex_clamp = false;
    tex_compress = false;
    tex_linear = true;
    tex_premultiply = true;
    tex_mipmap = true;
    width = 1000;
    height = 800;
    sample_rate = 44100;
    auto_play = false;
    sbs3d = false;
    ou3d = false;
    reverse3d = false;

    //room
    party_mode = true;
    cursor_visible = true;
    near_dist = 0.01f;
    far_dist = 1000.0f;
    grab_dist = 0.5f;
    fog = false;
    fog_mode = 1;
    fog_density = 1.0f;
    fog_start = 0.0f;
    fog_end = 1.0f;
    fog_col = new ScriptableVector(0, 0, 0, 1, this);
    gravity = -9.8f;
    jump_velocity = 5.0f;
    walk_speed = 1.8f;
    run_speed = 5.4f;
    reset_volume.first = QVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    reset_volume.second = QVector3D(FLT_MAX, -100.0f, FLT_MAX);
    teleport_min_dist = 0.0f;
    teleport_max_dist = 100.0f;
    server = SettingsManager::GetServer();
    port = SettingsManager::GetPort();

    //player
    view_dir = new ScriptableVector(0, 0, 1, this);
    up_dir = new ScriptableVector(0, 1, 0, this);
    cursor0_active = false;
    cursor0_pos = new ScriptableVector(0, 0, 0, this);
    cursor0_xdir = new ScriptableVector(1, 0, 0, this);
    cursor0_ydir = new ScriptableVector(0, 1, 0, this);
    cursor0_zdir = new ScriptableVector(0, 0, -1, this);
    cursor1_active = false;
    cursor1_pos = new ScriptableVector(0, 0, 0, this);
    cursor1_xdir = new ScriptableVector(1, 0, 0, this);
    cursor1_ydir = new ScriptableVector(0, 1, 0, this);
    cursor1_zdir = new ScriptableVector(0, 0, -1, this);
    local_head_pos = new ScriptableVector(0, 0, 0, this);
    head_pos = new ScriptableVector(0, 0, 0, this);
    eye_pos = new ScriptableVector(0, 1.6f, 0, this);
    hand0_active = false;
    hand0_trackpad = new ScriptableVector(0,0,0,this);
    hand0_pos = new ScriptableVector(0,0,0,this);
    hand0_vel = new ScriptableVector(0,0,0,this);
    hand0_xdir = new ScriptableVector(1,0,0,this);
    hand0_ydir = new ScriptableVector(0,1,0,this);
    hand0_zdir = new ScriptableVector(0,0,1,this);
    hand1_active = false;
    hand1_trackpad = new ScriptableVector(0,0,0,this);
    hand1_pos = new ScriptableVector(0,0,0,this);
    hand1_vel = new ScriptableVector(0,0,0,this);
    hand1_xdir = new ScriptableVector(1,0,0,this);
    hand1_ydir = new ScriptableVector(0,1,0,this);
    hand1_zdir = new ScriptableVector(0,0,1,this);
    hmd_type = "2d";    
}

DOMNode::~DOMNode()
{
    //qDebug() << "DOMNode::~DOMNode()" << this;
    for (QList <QPointer <DOMNode> >::iterator it = children_nodes.begin(); it!=children_nodes.end(); ++it) {
        if (!it->isNull()) {
            delete *it;
        }
    }
    children_nodes.clear();
}

void DOMNode::SetProperties(const QVariantMap & d)
{
    //asset
    if (d.contains("src")) {
        SetSrc(d["src"].toString());
    }
    if (d.contains("vertex_src")) {
        SetVertexSrc(d["vertex_src"].toString());
    }
    if (d.contains("mtl")) {
        SetMTL(d["mtl"].toString());
    }
    if (d.contains("sbs3d")) {
        SetSBS3D(d["sbs3d"].toBool());
    }
    if (d.contains("ou3d")) {
        SetOU3D(d["ou3d"].toBool());
    }
    if (d.contains("reverse3d")) {
        SetReverse3D(d["reverse3d"].toBool());
    }
    if (d.contains("tex_alpha")) {
        SetTexAlpha(d["tex_alpha"].toString());
    }
    if (d.contains("tex_colorspace")) {
        SetTexColorspace(d["tex_colorspace"].toString());
    }
    if (d.contains("tex_clamp")) {
        SetTexClamp(d["tex_clamp"].toBool());
    }
    if (d.contains("tex_linear")) {
        SetTexLinear(d["tex_linear"].toBool());
    }
    if (d.contains("tex_compress")) {
        SetTexCompress(d["tex_compress"].toBool());
    }
    if (d.contains("tex_premultiply")) {
        SetTexPremultiply(d["tex_premultiply"].toBool());
    }
    if (d.contains("tex_mipmap")) {
        SetTexMipmap(d["tex_mipmap"].toBool());
    }

    //room
    if (d.contains("party_mode")) {
        SetPartyMode(d["party_mode"].toBool());
    }
    if (d.contains("locked")) {
        SetLocked(d["locked"].toBool());
    }
    if (d.contains("visible")) {
        SetVisible(d["visible"].toBool());
    }
    if (d.contains("cursor_visible")) {
        SetCursorVisible(d["cursor_visible"].toBool());
    }
    if (d.contains("gravity")) {
        SetGravity(d["gravity"].toFloat());
    }
    if (d.contains("jump_velocity")) {
        SetJumpVelocity(d["jump_velocity"].toFloat());
    }
    if (d.contains("walk_speed")) {
        SetWalkSpeed(d["walk_speed"].toFloat());
    }
    if (d.contains("run_speed")) {
        SetRunSpeed(d["run_speed"].toFloat());
    }
    if (d.contains("teleport_max_dist")) {
        SetTeleportMaxDist(d["teleport_max_dist"].toFloat());
    }
    if (d.contains("teleport_min_dist")) {
        SetTeleportMinDist(d["teleport_min_dist"].toFloat());
    }
    if (d.contains("shader_id")) {
        SetShaderID(d["shader_id"].toString());
    }
    if (d.contains("col")) {
        SetColour(MathUtil::GetColourFromQVariant(d["col"]));
    }
    if (d.contains("near_dist")) {
        SetNearDist(d["near_dist"].toFloat());
    }
    if (d.contains("far_dist")) {
        SetFarDist(d["far_dist"].toFloat());
    }
    if (d.contains("grab_dist")) {
        SetGrabDist(d["grab_dist"].toFloat());
    }
    if (d.contains("fog")) {        
        SetFog(d["fog"].toBool());
    }
    if (d.contains("fog_density")) {
        SetFogDensity(d["fog_density"].toFloat());
    }
    if (d.contains("fog_start")) {
        SetFogStart(d["fog_start"].toFloat());
    }
    if (d.contains("fog_end")) {
        SetFogEnd(d["fog_end"].toFloat());
    }
    if (d.contains("fog_col")) {
        SetFogCol(MathUtil::GetColourFromQVariant(d["fog_col"]));
    }
    if (d.contains("fog_mode")) {
        SetFogMode(d["fog_mode"].toString());
    }
    if (d.contains("server")) {
        SetServer(d["server"].toString());
    }
    if (d.contains("port")) {
        SetServerPort(d["port"].toInt());
    }
    if (d.contains("reset_volume")) {
        SetResetVolume(MathUtil::GetStringAsAABB(d["reset_volume"].toString()));
    }
    if (d.contains("use_local_asset")) {
        SetUseLocalAsset(d["use_local_asset"].toString());
    }

    //roomobject
    if (d.contains("onclick")) {
        SetOnClick(d["onclick"].toString());
    }
    if (d.contains("oncollision")) {
        SetOnCollision(d["oncollision"].toString());
    }
    if (d.contains("id")) {
        SetID(d["id"].toString());
    }
    if (d.contains("cubemap_radiance_id")) {
        SetCubemapRadianceID(d["cubemap_radiance_id"].toString());
    }
    if (d.contains("cubemap_irradiance_id")) {
        SetCubemapIrradianceID(d["cubemap_irradiance_id"].toString());
    }
    if (d.contains("js_id")) {
        if (!d["js_id"].toString().isEmpty()) {
            SetJSID(d["js_id"].toString());
        }
    }
    if (d.contains("loop")) {
        SetLoop(d["loop"].toBool());
    }
    if (d.contains("auto_play")) {
        SetAutoPlay(d["auto_play"].toBool());
    }
    if (d.contains("auto_load")) {
        SetAutoLoad(d["auto_load"].toBool());
    }
    if (d.contains("title")) {
        SetTitle(d["title"].toString());
    }
    if (d.contains("draw_text")) {
        SetDrawText(d["draw_text"].toBool());
    }
    if (d.contains("pos")) {
        SetPos(MathUtil::GetVectorFromQVariant(d["pos"]));
    }
    if (d.contains("vel")) {
        SetVel(MathUtil::GetVectorFromQVariant(d["vel"]));
    }
    if (d.contains("accel")) {
        SetAccel(MathUtil::GetVectorFromQVariant(d["accel"]));
    }
    if (d.contains("rotation_order")) {
        SetRotationOrder(d["rotation_order"].toString());
    }
    if (d.contains("rotation")) {
        SetRotation(MathUtil::GetVectorFromQVariant(d["rotation"]));
    }
    else if (d.contains("fwd")) {
        SetDir(MathUtil::GetVectorFromQVariant(d["fwd"]));
    }
    else {
        QVector3D x = GetXDir()->toQVector3D();
        QVector3D y = GetYDir()->toQVector3D();
        QVector3D z = GetZDir()->toQVector3D();
        if (d.contains("xdir")) {
            x = MathUtil::GetVectorFromQVariant(d["xdir"]);
        }
        if (d.contains("ydir")) {
            y = MathUtil::GetVectorFromQVariant(d["ydir"]);
        }
        if (d.contains("zdir")) {
            z = MathUtil::GetVectorFromQVariant(d["zdir"]);
        }
        SetXDirs(x,y,z);
    }
    if (d.contains("chromakey_col")) {
        SetChromaKeyColour(MathUtil::GetColourFromQVariant(d["chromakey_col"]));
    }
    if (d.contains("scale")) {
        SetScale(MathUtil::GetVectorFromQVariant(d["scale"]));
    }
    if (d.contains("rand_pos")) {
        SetRandPos(MathUtil::GetVectorFromQVariant(d["rand_pos"]));
    }
    if (d.contains("rand_vel")) {
        SetRandVel(MathUtil::GetVectorFromQVariant(d["rand_vel"]));
    }
    if (d.contains("rand_accel")) {
        SetRandAccel(MathUtil::GetVectorFromQVariant(d["rand_accel"]));
    }
    if (d.contains("rand_col")) {
        SetRandColour(MathUtil::GetVectorFromQVariant(d["rand_col"]));
    }
    if (d.contains("rand_scale")) {
        SetRandScale(MathUtil::GetVectorFromQVariant(d["rand_scale"]));
    }
    if (d.contains("locked")) {
        SetLocked(d["locked"].toBool());
    }
    if (d.contains("lighting")) {
        SetLighting(d["lighting"].toBool());
    }
    if (d.contains("collision_static")) {
        SetCollisionStatic(d["collision_static"].toBool());
    }
    if (d.contains("collision_ccdmotionthreshold")) {
        SetCollisionCcdMotionThreshold(d["collision_ccdmotionthreshold"].toFloat());
    }
    if (d.contains("collision_ccdsweptsphereradius")) {
        SetCollisionCcdSweptSphereRadius(d["collision_ccdsweptsphereradius"].toFloat());
    }
    if (d.contains("collision_trigger")) {
        SetCollisionTrigger(d["collision_trigger"].toBool());
    }
    if (d.contains("visible")) {
        SetVisible(d["visible"].toBool());
    }
    if (d.contains("mirror")) {
        SetMirror(d["mirror"].toBool());
    }
    if (d.contains("active")) {
        SetActive(d["active"].toBool());
    }
    if (d.contains("shader_id")) {
        SetShaderID(d["shader_id"].toString());
    }
    if (d.contains("websurface_id")) {
        SetWebsurfaceID(d["websurface_id"].toString());
    }
    if (d.contains("teleport_id")) {
        SetTeleportID(d["teleport_id"].toString());
    }
    if (d.contains("video_id")) {
        SetVideoID(d["video_id"].toString());
    }
    if (d.contains("image_id")) {
        SetImageID(d["image_id"].toString());
    }
    if (d.contains("bone_id")) {
        SetBoneID(d["bone_id"].toString());
    }
    if (d.contains("lmap_id")) {
        SetLightmapID(d["lmap_id"].toString());
    }
    if (d.contains("tile")) {
        SetTiling(MathUtil::GetVector4FromQVariant(d["tile"]));
    }
    if (d.contains("lmap_sca")) {
        SetLightmapScale(MathUtil::GetVector4FromQVariant(d["lmap_sca"]));
    }
    if (d.contains("rotate_axis")) {
        SetSpinAxis(MathUtil::GetVectorFromQVariant(d["rotate_axis"]));
    }
    if (d.contains("rotate_deg_per_sec")) {
        SetSpinVal(d["rotate_deg_per_sec"].toFloat());
    }
    if (d.contains("emitter_id")) {
        SetEmitterID(d["emitter_id"].toString());
    }
    if (d.contains("emit_local")) {
        SetEmitLocal(d["emit_local"].toBool());
    }
    if (d.contains("collision_id")) {
        SetCollisionID(d["collision_id"].toString());
    }
    if (d.contains("collision_radius")) {
        SetCollisionRadius(d["collision_radius"].toFloat());
    }
    if (d.contains("cull_face")) {
        SetCullFace(d["cull_face"].toString());
    }
    if (d.contains("eye_pos")) {
        SetEyePos(MathUtil::GetVectorFromQVariant(d["eye_pos"]));
    }
    if (d.contains("body_id")) {
        SetBodyID(d["body_id"].toString());
    }
    if (d.contains("head_id")) {
        SetHeadID(d["head_id"].toString());
    }
    if (d.contains("anim_id")) {
        SetAnimID(d["anim_id"].toString());
    }
    if (d.contains("anim_speed")) {
        SetAnimSpeed(d["anim_speed"].toFloat());
    }
    if (d.contains("text")) {
        SetText(d["text"].toString());
    }
    if (d.contains("innertext")) {
        SetText(d["innertext"].toString());
    }
    if (d.contains("font_size")) {
        SetFontSize(d["font_size"].toFloat());
    }
    if (d.contains("back_col")) {
        SetBackCol(MathUtil::GetColourFromQVariant(d["back_col"]));
    }
    if (d.contains("text_col")) {
        SetTextCol(MathUtil::GetColourFromQVariant(d["text_col"]));
    }
    if (d.contains("back_alpha")) {
        SetBackAlpha(d["back_alpha"].toFloat());
    }
    if (d.contains("rect")) {
        SetTriggerRect(MathUtil::GetStringAsRect(d["rect"].toString()));
    }
    if (d.contains("play_once")) {
        SetPlayOnce(d["play_once"].toBool());
    }
    if (d.contains("thumb_id")) {
        SetThumbID(d["thumb_id"].toString());
    }
    if (d.contains("blend0_id")) {
        SetBlend0ID(d["blend0_id"].toString());
    }
    if (d.contains("blend1_id")) {
        SetBlend1ID(d["blend1_id"].toString());
    }
    if (d.contains("blend2_id")) {
        SetBlend2ID(d["blend2_id"].toString());
    }
    if (d.contains("blend3_id")) {
        SetBlend3ID(d["blend3_id"].toString());
    }
    if (d.contains("blend0")) {
        SetBlend0(d["blend0"].toFloat());
    }
    if (d.contains("blend1")) {
        SetBlend1(d["blend1"].toFloat());
    }
    if (d.contains("blend2")) {
        SetBlend2(d["blend2"].toFloat());
    }
    if (d.contains("blend3")) {
        SetBlend3(d["blend3"].toFloat());
    }
    if (d.contains("pitch")) {
        SetPitch(d["pitch"].toFloat());
    }
    if (d.contains("gain")) {
        SetGain(d["gain"].toFloat());
    }
    if (d.contains("doppler_factor")) {
        SetDopplerFactor(d["doppler_factor"].toFloat());
    }
    if (d.contains("outer_gain")) {
        SetOuterGain(d["outer_gain"].toFloat());
    }
    if (d.contains("inner_angle")) {
        SetInnerAngle(d["inner_angle"].toFloat());
    }
    if (d.contains("outer_angle")) {
        SetOuterAngle(d["outer_angle"].toFloat());
    }
    if (d.contains("rate")) {
        SetRate(d["rate"].toInt());
    }
    if (d.contains("count")) {
        SetCount(d["count"].toInt());
    }
    if (d.contains("duration")) {
        SetDuration(d["duration"].toFloat());
    }
    if (d.contains("fade_duration")) {
        SetFadeOut(d["fade_duration"].toFloat());
    }
    if (d.contains("fade_out")) {
        SetFadeOut(d["fade_out"].toFloat());
    }
    if (d.contains("fade_in")) {
        SetFadeIn(d["fade_in"].toFloat());
    }
    if (d.contains("light_intensity")) {
        SetLightIntensity(d["light_intensity"].toFloat()*light_intensity_multiplier);
    }
    if (d.contains("light_cone_angle")) {
        SetLightConeAngle(d["light_cone_angle"].toFloat());
    }
    if (d.contains("light_cone_exponent")) {
        SetLightConeExponent(d["light_cone_exponent"].toFloat());
    }
    if (d.contains("light_range")) {
        SetLightRange(d["light_range"].toFloat());
    }
    if (d.contains("collision_pos")) {
        SetCollisionPos(MathUtil::GetVectorFromQVariant(d["collision_pos"]));
    }
    if (d.contains("collision_scale")) {
        SetCollisionScale(MathUtil::GetVectorFromQVariant(d["collision_scale"]));
    }
    if (d.contains("collision_friction")) {
        SetCollisionFriction(d["collision_friction"].toFloat());
    }
    if (d.contains("collision_rollingfriction")) {
        SetCollisionRollingFriction(d["collision_rollingfriction"].toFloat());
    }
    if (d.contains("collision_restitution")) {
        SetCollisionRestitution(d["collision_restitution"].toFloat());
    }
    if (d.contains("collision_angulardamping")) {
        SetCollisionAngularDamping(d["collision_angulardamping"].toFloat());
    }
    if (d.contains("collision_lineardamping")) {
        SetCollisionLinearDamping(d["collision_lineardamping"].toFloat());
    }
    if (d.contains("draw_layer")) {
        SetDrawLayer(d["draw_layer"].toInt());
    }
    if (d.contains("url")) {
        SetURL(d["url"].toString());
    }
    if (d.contains("text") || d.contains("innertext")) {
        SetTextChanged(true);
    }
    if (d.contains("swallow")) {
        SetSwallow(d["swallow"].toBool());
    }
}

void DOMNode::SetFwd(const ScriptableVector * v)
{
    if (v) {
        QVector3D d(v->GetX(), v->GetY(), v->GetZ());

        const QVector3D z = d.normalized();
        const QVector3D x = QVector3D::crossProduct(QVector3D(0, 1, 0), z).normalized();
        const QVector3D y = QVector3D::crossProduct(z, x).normalized();

        SetXDir(x);
        SetYDir(y);
        SetZDir(z);
    }
}

ScriptableVector * DOMNode::GetFwd() const
{
    return zdir;
}

void DOMNode::Copy(QPointer <DOMNode> p)
{
    pos->Copy(p->pos);
    vel->Copy(p->vel);
    xdir->Copy(p->xdir);
    ydir->Copy(p->ydir);
    zdir->Copy(p->zdir);
    col->Copy(p->col);
    chromakey_col->Copy(p->chromakey_col);
    scale->Copy(p->scale);

    rand_pos->Copy(p->rand_pos);
    rand_vel->Copy(p->rand_vel);
    rand_accel->Copy(p->rand_accel);
    rand_col->Copy(p->rand_col);
    rand_scale->Copy(p->rand_scale);

    type = p->type;
    //p->uuid = uuid; //do not copy js_id or uuid
    //p->js_id = js_id;
    id = p->id;
    m_cubemap_radiance_id = p->m_cubemap_radiance_id;
    m_cubemap_irradiance_id = p->m_cubemap_irradiance_id;
    image_id = p->image_id;
    text =p-> text;
    font_size = p->font_size;
    onclick_code = p->onclick_code;
    oncollision = p->oncollision;
    websurface_id = p->websurface_id;
    teleport_id = p->teleport_id;
    video_id = p->video_id;
    shader_id = p->shader_id;
    thumb_id = p->thumb_id;
    bone_id = p->bone_id;
    sync = p->sync;
    lighting = p->lighting;
    body_id = p->body_id;
    head_id = p->head_id;
    lightmap_id = p->lightmap_id;
    lightmap_scale->Copy(p->lightmap_scale);

    collision_id = p->collision_id;
    collision_radius = p->collision_radius;
    collision_friction = p->collision_friction;
    collision_rollingfriction = p->collision_rollingfriction;
    collision_restitution = p->collision_restitution;
    collision_angulardamping = p->collision_angulardamping;
    collision_lineardamping = p->collision_lineardamping;
    collision_pos->Copy(p->collision_pos);
    collision_scale->Copy(p->collision_scale);
    collision_static = p->collision_static;
    collision_trigger = p->collision_trigger;
    collision_ccdmotionthreshold = p->collision_ccdmotionthreshold;
    collision_ccdsweptsphereradius = p->collision_ccdsweptsphereradius;

    spinaxis->Copy(p->spinaxis);
    spinval = p->spinval;

    visible = p->visible;

    m_light_intensity = p->m_light_intensity;
    m_light_cone_angle = p->m_light_cone_angle;
    m_light_cone_exponent = p->m_light_cone_exponent;
    m_light_range = p->m_light_range;

    m_draw_layer = p->m_draw_layer;
}

int DOMNode::NextUUID()
{
    return next_uuid++;
}

void DOMNode::AppendChild(QPointer<DOMNode> child)
{
    if (child) {
        if (child->GetParent()) {
            child->GetParent()->RemoveChildByJSID(child->GetJSID());
        }

        if (!children_nodes.empty() && children_nodes.last()) {
            children_nodes.last()->SetRightSibling(child);
            child->SetLeftSibling(children_nodes.last());
        }
        children_nodes.append(child);
        child->SetParent(this);
    }
}

void DOMNode::appendChild(DOMNode *child)
{
    QString oldParentJSID = "__null";
    if (child->GetParent())
        oldParentJSID = child->GetParent()->GetJSID();
    AppendChild(QPointer <DOMNode>(child));
    if (engine() && oldParentJSID!=QString("__null"))
    {
        QScriptValue parentChanged = engine()->globalObject().property("__parent_changed");
        if (!parentChanged.isValid()) {
            parentChanged = engine()->newObject();
            engine()->globalObject().setProperty("__parent_changed", parentChanged);
        }
        QScriptValue map = engine()->newObject();
        QScriptValue existingMap = parentChanged.property(child->GetJSID());
        if (existingMap.isValid()) {
            map.setProperty("old_parent_id", existingMap.property("old_parent_id"));
        }
        else {
            map.setProperty("old_parent_id", oldParentJSID);
        }
        //map.setProperty("new_parent_id", QScriptValue(QString("__null")));
        map.setProperty("new_parent_id", GetJSID());

        parentChanged.setProperty(child->GetJSID(), map);
    }
}

void DOMNode::RemoveChild(QPointer <DOMNode> n)
{
    RemoveChildAt(children_nodes.indexOf(n));
}

void DOMNode::PrependChild(QPointer<DOMNode> child)
{
    if (!children_nodes.empty())
    {
        children_nodes.first()->SetLeftSibling(child);
        child->SetRightSibling(children_nodes.first());
    }
    children_nodes.prepend(child);
    child->SetParent(this);
}

void DOMNode::SetChildren(QList<DOMNode* > children)
{
    if (engine())
    {
        context()->throwError("Unable to set the children property. Use appendChild() instead.");
        return;
    }
    children_nodes.clear();

    for (int i=0; i<children.size(); ++i)
        children_nodes.append(QPointer <DOMNode>(children[i]));
}

void DOMNode::SetParent(DOMNode *parent)
{
    if (engine())
    {
        context()->throwError("Unable to set the parent property. Use appendChild() on the intended parent instead.");
        return;
    }
    parent_node = QPointer <DOMNode>(parent);
}

void DOMNode::SetLeftSibling(DOMNode *node)
{
    //if (engine())
    //{
    //    context()->throwError("The leftSibling property is read-only.");
    //    return;
    //}

    left_sibling = QPointer <DOMNode>(node);
}

void DOMNode::SetRightSibling(DOMNode *node)
{
    //if (engine())
    //{
    //    context()->throwError("The rightSibling property is read-only.");
    //    return;
    //}
    right_sibling = QPointer <DOMNode>(node);
}

DOMNode *DOMNode::GetParent() const
{
    if (parent_node && parent_node->IsDirty())
        return NULL;
    return parent_node.data();
}

DOMNode * DOMNode::GetLeftSibling() const
{
    if (left_sibling && left_sibling->IsDirty())
        return left_sibling->GetLeftSibling();
    return left_sibling.data();
}

DOMNode* DOMNode::GetRightSibling() const
{
    if (right_sibling && right_sibling->IsDirty())
        return right_sibling->GetRightSibling();
    return right_sibling.data();
}

QList <DOMNode *> DOMNode::GetChildren() const
{
    QList <DOMNode *> unguarded_children;
    for (int i=0; i<children_nodes.size(); ++i)
    {
        //trick to ensure that deleted objects currently in "dirty" state are not returned.
        //qDebug()<<children_nodes[i]->GetJSID()<<children_nodes[i]->IsDirty();
        if (children_nodes[i] && !children_nodes[i]->IsDirty()) {
            unguarded_children.append(children_nodes[i].data());
        }
    }
    //qDebug() << "DOMNode::GetChildren()" << children_nodes.size() << unguarded_children.size();
    return unguarded_children;
}

void DOMNode::ClearChildren()
{
    children_nodes.clear();
}

void DOMNode::RemoveChildAt(int pos)
{
    if (pos>=0 && pos<children_nodes.size())
    {
        if (pos-1>=0 && children_nodes[pos-1])
            children_nodes[pos-1]->SetRightSibling(children_nodes[pos]->GetRightSibling());
        if (pos+1<children_nodes.size() && children_nodes[pos+1])
            children_nodes[pos+1]->SetLeftSibling(children_nodes[pos]->GetLeftSibling());

        children_nodes[pos]->SetParent(NULL);
        children_nodes.removeAt(pos);
    }
}

QPointer <DOMNode> DOMNode::RemoveChildByJSID(QString jsid)
{
    QPointer <DOMNode> child;
    for (int i=0; i<children_nodes.size(); ++i)
    {
        if (children_nodes[i]->GetJSID()==jsid)
        {
            child = children_nodes[i];
            RemoveChildAt(i);
            return child;
        }
    }
    return child;
}

//removeChild() is the user faceing JS function

DOMNode * DOMNode::removeChild(QString jsid)
{
    QPointer <DOMNode> child = RemoveChildByJSID(jsid);
    if (child)
    {
        if (engine())
        {
            QScriptValue parentChanged = engine()->globalObject().property("__parent_changed");
            if (!parentChanged.isValid()) {
                parentChanged = engine()->newObject();
                engine()->globalObject().setProperty("__parent_changed", parentChanged);
            }
            QScriptValue map = engine()->newObject();
            QScriptValue existingMap = parentChanged.property(jsid);
            if (existingMap.isValid())
                map.setProperty("old_parent_id", existingMap.property("old_parent_id"));
            else
                map.setProperty("old_parent_id", GetJSID());
            map.setProperty("new_parent_id", QScriptValue(QString("__null")));
            parentChanged.setProperty(child->GetJSID(), map);

            QScriptValue roomObject = engine()->globalObject().property("__room");
            roomObject.property("appendChild").call(roomObject, engine()->toScriptValue <DOMNode *>(child.data()));
        }

        //qDebug()<<jsid<<"detached from parent"<<GetJSID();
        return child.data();
    }
    else
        return NULL;
}

DOMNode * DOMNode::removeChild(DOMNode *node)
{
    QPointer <DOMNode> child = RemoveChildByJSID(node->GetJSID());
    if (child) {
        if (engine()) {
            QScriptValue parentChanged = engine()->globalObject().property("__parent_changed");
            if (!parentChanged.isValid()) {
                parentChanged = engine()->newObject();
                engine()->globalObject().setProperty("__parent_changed", parentChanged);
            }
            QScriptValue map = engine()->newObject();
            QScriptValue existingMap = parentChanged.property(child->GetJSID());
            if (existingMap.isValid()) {
                map.setProperty("old_parent_id", existingMap.property("old_parent_id"));
            }
            else {
                map.setProperty("old_parent_id", GetJSID());
            }
            map.setProperty("new_parent_id", QScriptValue(QString("__null")));
            parentChanged.setProperty(child->GetJSID(), map);

            QScriptValue roomObject = engine()->globalObject().property("__room");
            roomObject.property("appendChild").call(roomObject, engine()->toScriptValue <DOMNode *>(child.data()));
        }
        return child.data();
    }
    else {
        return NULL;
    }
}

void DOMNode::SetDirty(const bool b)
{
    dirty = b;
}

void DOMNode::SetType(QString & t)
{
    SetType(StringToElementType(t));
}

void DOMNode::SetType(const ElementType & t)
{
    type = t;
}

QString DOMNode::GetTypeAsString()
{
    return ElementTypeToString(type);
}

void DOMNode::SetInterpolate(const bool b)
{
    interpolate = b;
}

bool DOMNode::GetInterpolate() const
{
    return interpolate;
}

void DOMNode::SetPos(ScriptableVector *&p)
{
    if (p) {
        *pos = *p;
    }
}

void DOMNode::SetPos(const QVector3D & p)
{
    pos->SetFromOther(p);
}

void DOMNode::SetXDir(ScriptableVector * & x)
{
    if (x) {
        *xdir = *x;
    }
}

void DOMNode::SetXDir(const QVector3D & x)
{
    xdir->SetFromOther(x);
}

void DOMNode::SetYDir(ScriptableVector * & y)
{
    if (y) {
        *ydir = *y;
    }
}

void DOMNode::SetYDir(const QVector3D & y)
{
    ydir->SetFromOther(y);
}

void DOMNode::SetZDir(ScriptableVector * & z)
{
    if (z) {
        *zdir = *z;
    }
}

void DOMNode::SetZDir(const QVector3D & z)
{
    zdir->SetFromOther(z);
}

void DOMNode::SetDir(ScriptableVector * & d)
{
    if (d) {
        SetDir(d->toQVector3D());
    }
}

void DOMNode::SetDir(const QVector3D & d)
{
    const QVector3D z = d.normalized();
    const QVector3D x = QVector3D::crossProduct(QVector3D(0, 1, 0), z).normalized();
    const QVector3D y = QVector3D::crossProduct(z, x).normalized();

    SetXDir(x);
    SetYDir(y);
    SetZDir(z);
}

void DOMNode::SetRotation(const QVector3D & v)
{
    //qDebug() << "DOMNode::SetRotation rotation found!" << v;
    rotation->SetFromOther(v);

    const QMatrix4x4 m = MathUtil::GetRotationMatrixFromEuler(rotation->toQVector3D(), rotation_order);
    SetXDir(m.column(0).toVector3D());
    SetYDir(m.column(1).toVector3D());
    SetZDir(m.column(2).toVector3D());
}

void DOMNode::SetRotation(ScriptableVector *&v)
{
    if (v) {
        //qDebug() << "DOMNode::SetRotation" << v->GetX() << v->GetY() << v->GetZ();
        *rotation = *v;

        const QMatrix4x4 m = MathUtil::GetRotationMatrixFromEuler(rotation->toQVector3D(), rotation_order);
        SetXDir(m.column(0).toVector3D());
        SetYDir(m.column(1).toVector3D());
        SetZDir(m.column(2).toVector3D());
    }
}

ScriptableVector * DOMNode::GetRotation()
{
    return rotation;
}

void DOMNode::SetRotationOrder(const QString & s)
{
    rotation_order = s;
}

QString DOMNode::GetRotationOrder() const
{
    return rotation_order;
}

void DOMNode::SetVel(ScriptableVector * & v)
{
    if (v) {
        *vel = *v;
    }
}

void DOMNode::SetVel(const QVector3D & v)
{
    vel->SetFromOther(v);
}

void DOMNode::SetAccel(ScriptableVector * & v)
{
    if (v) {
        *accel = *v;
    }
}

void DOMNode::SetAccel(const QVector3D & v)
{
    accel->SetFromOther(v);
}

void DOMNode::SetColour(ScriptableVector * c)
{
    if (c) {
        //qDebug() << "DOMNode::SetColour" << c->toString();
        col->SetX(qMax(qMin(c->GetX(), 1.0f), 0.0f));
        col->SetY(qMax(qMin(c->GetY(), 1.0f), 0.0f));
        col->SetZ(qMax(qMin(c->GetZ(), 1.0f), 0.0f));
        col->SetW(qMax(qMin(c->GetW(), 1.0f), 0.0f));
    }
}

void DOMNode::SetColour(const QVector4D & c)
{
    col->SetFromOther(c);
    col->SetX(qMax(qMin(col->GetX(), 1.0f), 0.0f));
    col->SetY(qMax(qMin(col->GetY(), 1.0f), 0.0f));
    col->SetZ(qMax(qMin(col->GetZ(), 1.0f), 0.0f));
    col->SetW(qMax(qMin(col->GetW(), 1.0f), 0.0f));
}

void DOMNode::SetColour(const QVector3D & c)
{
    col->SetFromOther(c);
    col->SetX(qMax(qMin(col->GetX(), 1.0f), 0.0f));
    col->SetY(qMax(qMin(col->GetY(), 1.0f), 0.0f));
    col->SetZ(qMax(qMin(col->GetZ(), 1.0f), 0.0f));
    col->SetW(1.0f);
}

void DOMNode::SetColour(const QColor & c)
{
    col->SetX(qMax(qMin(float(c.redF()), 1.0f), 0.0f));
    col->SetY(qMax(qMin(float(c.greenF()), 1.0f), 0.0f));
    col->SetZ(qMax(qMin(float(c.blueF()), 1.0f), 0.0f));
    col->SetW(qMax(qMin(float(c.alphaF()), 1.0f), 0.0f));
}

void DOMNode::SetChromaKeyColour(ScriptableVector * & c)
{
    if (c) {
        chromakey_col->SetX(c->GetX());
        chromakey_col->SetY(c->GetY());
        chromakey_col->SetZ(c->GetZ());
        chromakey_col->SetW(c->GetW());
    }
}

void DOMNode::SetChromaKeyColour(const QVector4D & c)
{
    chromakey_col->SetFromOther(c);
}

void DOMNode::SetScale(const QVector3D & s)
{
    scale->SetFromOther(s);
}

void DOMNode::SetScale(ScriptableVector * s)
{
    if (s) {
        *scale = *s;
    }
}

void DOMNode::SetRandPos(ScriptableVector * & v)
{
    if (v) {
        *rand_pos = *v;
    }
}

void DOMNode::SetRandPos(const QVector3D & p)
{
    rand_pos->SetFromOther(p);
}

void DOMNode::SetRandVel(ScriptableVector * & v)
{
    if (v) {
        *rand_vel = *v;
    }
}

void DOMNode::SetRandVel(const QVector3D & v)
{
    rand_vel->SetFromOther(v);
}

void DOMNode::SetRandAccel(ScriptableVector * & v)
{
    if (v) {
        *rand_accel = *v;
    }
}

void DOMNode::SetRandAccel(const QVector3D & v)
{
    rand_accel->SetFromOther(v);
}

void DOMNode::SetRandColour(ScriptableVector * & v)
{
    if (v) {
        *rand_col = *v;
    }
}

void DOMNode::SetRandColour(const QVector3D & c)
{
    rand_col->SetFromOther(c);
}

void DOMNode::SetRandScale(ScriptableVector * & v)
{
    if (v) {
        *rand_scale = *v;
    }
}

void DOMNode::SetRandScale(const QVector3D & s)
{
    rand_scale->SetFromOther(s);
}

void DOMNode::SetSync(const bool b)
{
    sync = b;
}

void DOMNode::SetJSID(const QString & id)
{
    js_id = id;
}

void DOMNode::SetID(const QString & id)
{
    this->id = id;
}

void DOMNode::SetCubemapRadianceID(const QString & id)
{
    this->m_cubemap_radiance_id = id;
}

void DOMNode::SetCubemapIrradianceID(const QString & id)
{
    this->m_cubemap_irradiance_id = id;
}

void DOMNode::SetBlend0ID(const QString & id)
{
    this->blend0_id = id;
}

void DOMNode::SetBlend1ID(const QString & id)
{
    blend1_id = id;
}

void DOMNode::SetBlend2ID(const QString & id)
{
    blend2_id = id;
}

void DOMNode::SetBlend3ID(const QString & id)
{
    blend3_id = id;
}

void DOMNode::SetBlend0(const float f)
{
    blend0 = f;
}

void DOMNode::SetBlend1(const float f)
{
    blend1 = f;
}

void DOMNode::SetBlend2(const float f)
{
    blend2 = f;
}

void DOMNode::SetBlend3(const float f)
{
    blend3 = f;
}

void DOMNode::SetLoop(const bool b)
{
    loop = b;
}

void DOMNode::SetGain(const float f)
{
    gain = f;
}

void DOMNode::SetDopplerFactor(const float f)
{
    doppler_factor = f;
}

void DOMNode::SetOuterGain(const float f)
{
    outer_gain = f;
}

void DOMNode::SetInnerAngle(const float f)
{
    inner_angle = f;
}

void DOMNode::SetOuterAngle(const float f)
{
    outer_angle = f;
}

void DOMNode::SetPitch(const float f)
{
    pitch = f;
}

void DOMNode::SetCollisionID(const QString & id)
{
    collision_id = id;
}

void DOMNode::SetEmitterID(const QString & id)
{
    emitter_id = id;
}

void DOMNode::SetEmitLocal(const bool b)
{
    emit_local = b;
}

void DOMNode::SetCollisionRadius(const float r)
{
    collision_radius = r;
}

void DOMNode::SetText(const QString & text)
{
    this->text = text;
    text_changed = true;
}

void DOMNode::SetTextChanged(const bool & b)
{
    text_changed = b;
}

void DOMNode::SetFontSize(const int & newSize)
{
    font_size = newSize;
}

int DOMNode::valueOf() const
{
    return this->GetUUID();
}

void DOMNode::SetLocalHeadPos(const QVector3D &p)
{
    local_head_pos->SetFromOther(p);
}

void DOMNode::SetLocalHeadPos(ScriptableVector *&p)
{
    //qDebug() << "player headpos set2" << p->toString();
    if (p)
        *local_head_pos = *p;
}

ScriptableVector * DOMNode::GetLocalHeadPos()
{
    return local_head_pos;
}

void DOMNode::SetGlobalHeadPos(const QVector3D &p)
{
    head_pos->SetFromOther(p);
}

void DOMNode::SetGlobalHeadPos(ScriptableVector *&p)
{
    if (p) {
        *head_pos = *p;
    }
}

ScriptableVector * DOMNode::GetGlobalHeadPos()
{
    return head_pos;
}

void DOMNode::SetEyePos(const QVector3D &p)
{
    eye_pos->SetFromOther(p);
}

void DOMNode::SetEyePos(ScriptableVector *&p)
{
    if (p)
        *eye_pos = *p;
}

ScriptableVector * DOMNode::GetEyePos()
{
    return eye_pos;
}

int DOMNode::GetRoomObjectUUID() const
{
    return room_object_uuid;
}

void DOMNode::SetRoomObjectUUID(int value)
{
    room_object_uuid = value;
}

QString DOMNode::GetUseLocalAsset() const
{
    return use_local_asset;
}

void DOMNode::SetUseLocalAsset(const QString &value)
{
    use_local_asset = value;
}

bool DOMNode::GetFog() const
{
    return fog;
}

void DOMNode::SetFog(bool value)
{
    fog = value;
}

QString DOMNode::GetFogMode() const
{
    return fog_mode;
}

void DOMNode::SetFogMode(const QString &value)
{
    fog_mode = value;
}

float DOMNode::GetFogDensity() const
{
    return fog_density;
}

void DOMNode::SetFogDensity(float value)
{
    fog_density = value;
}

float DOMNode::GetFogStart() const
{
    return fog_start;
}

void DOMNode::SetFogStart(float value)
{
    fog_start = value;
}

float DOMNode::GetFogEnd() const
{
    return fog_end;
}

void DOMNode::SetFogEnd(float value)
{
    fog_end = value;
}

ScriptableVector *DOMNode::GetFogCol() const
{
    return fog_col;
}

void DOMNode::SetFogCol(const QVector4D & c)
{
    fog_col->SetFromOther(c);
    fog_col->SetX(qMax(qMin(col->GetX(), 1.0f), 0.0f));
    fog_col->SetY(qMax(qMin(col->GetY(), 1.0f), 0.0f));
    fog_col->SetZ(qMax(qMin(col->GetZ(), 1.0f), 0.0f));
    fog_col->SetW(qMax(qMin(col->GetW(), 1.0f), 0.0f));
}


void DOMNode::SetFogCol(ScriptableVector *value)
{
    if (value) {
        *fog_col = *value;
    }
}

float DOMNode::GetGrabDist() const
{
    return grab_dist;
}

void DOMNode::SetGrabDist(float value)
{
    grab_dist = value;
}

bool DOMNode::GetPartyMode() const
{
    return party_mode;
}

void DOMNode::SetPartyMode(bool value)
{
    party_mode = value;
}

bool DOMNode::GetTexCompress() const
{
    return tex_compress;
}

void DOMNode::SetTexCompress(bool value)
{
    tex_compress = value;
}

bool DOMNode::GetTriggered() const
{
    return triggered;
}

void DOMNode::SetTriggered(bool value)
{
    triggered = value;
}

bool DOMNode::GetHighlighted() const
{
    return highlighted;
}

void DOMNode::SetHighlighted(bool value)
{
    highlighted = value;
}

bool DOMNode::GetPrimitive() const
{
    return primitive;
}

void DOMNode::SetPrimitive(bool value)
{
    primitive = value;
}

float DOMNode::GetGravity() const
{
    return gravity;
}

void DOMNode::SetGravity(float value)
{
    gravity = value;
}

float DOMNode::GetJumpVelocity() const
{
    return jump_velocity;
}

void DOMNode::SetJumpVelocity(float value)
{
    jump_velocity = value;
}

QPair<QVector3D, QVector3D> DOMNode::GetResetVolume() const
{
    return reset_volume;
}

void DOMNode::SetResetVolume(const QPair<QVector3D, QVector3D> &value)
{
    reset_volume = value;
}

ScriptableVector *DOMNode::GetBackCol() const
{
    return back_col;
}

void DOMNode::SetBackCol(const QVector4D & c)
{
    back_col->SetFromOther(c);
    back_col->SetX(qMax(qMin(col->GetX(), 1.0f), 0.0f));
    back_col->SetY(qMax(qMin(col->GetY(), 1.0f), 0.0f));
    back_col->SetZ(qMax(qMin(col->GetZ(), 1.0f), 0.0f));
    back_col->SetW(qMax(qMin(col->GetW(), 1.0f), 0.0f));
}

void DOMNode::SetBackCol(ScriptableVector *value)
{
    back_col = value;
}

ScriptableVector *DOMNode::GetTextCol() const
{
    return text_col;
}

void DOMNode::SetTextCol(const QVector4D & c)
{
    text_col->SetFromOther(c);
    text_col->SetX(qMax(qMin(col->GetX(), 1.0f), 0.0f));
    text_col->SetY(qMax(qMin(col->GetY(), 1.0f), 0.0f));
    text_col->SetZ(qMax(qMin(col->GetZ(), 1.0f), 0.0f));
    text_col->SetW(qMax(qMin(col->GetW(), 1.0f), 0.0f));
}

void DOMNode::SetTextCol(ScriptableVector *value)
{
    text_col = value;
}

QRectF DOMNode::GetTriggerRect() const
{
    return rect;
}

void DOMNode::SetTriggerRect(const QRectF &value)
{
    rect = value;
}

bool DOMNode::GetPlayOnce() const
{
    return play_once;
}

void DOMNode::SetPlayOnce(bool value)
{
    play_once = value;
}

int DOMNode::GetCurMount() const
{
    return cur_mount;
}

void DOMNode::SetCurMount(int value)
{
    cur_mount = value;
}

bool DOMNode::GetReady() const
{
    return ready;
}

void DOMNode::SetReady(bool value)
{
    ready = value;
}

float DOMNode::GetProgress() const
{
    return progress;
}

void DOMNode::SetProgress(float value)
{
    progress = value;
}

bool DOMNode::GetReadyForScreenshot() const
{
    return ready_for_screenshot;
}

void DOMNode::SetReadyForScreenshot(bool value)
{
    ready_for_screenshot = value;
}

bool DOMNode::GetStartedAutoPlay() const
{
    return started_auto_play;
}

void DOMNode::SetStartedAutoPlay(bool value)
{
    started_auto_play = value;
}

void DOMNode::SetWebsurfaceID(const QString & s)
{
    websurface_id = s;
}

void DOMNode::SetTeleportID(const QString & s)
{
    teleport_id = s;
}

void DOMNode::SetVideoID(const QString & s)
{
    video_id = s;
}

void DOMNode::SetThumbID(const QString & s)
{
    thumb_id = s;
}

void DOMNode::SetShaderID(const QString & s)
{
    shader_id = s;
}

void DOMNode::SetImageID(const QString & s)
{
    image_id = s;
}

void DOMNode::SetLightmapID(const QString & s)
{
    lightmap_id = s;
}

void DOMNode::SetTiling(ScriptableVector * & v)
{
    *tiling = *v;
}

void DOMNode::SetTiling(const QVector4D & v)
{
    tiling->SetFromOther(v);
}


void DOMNode::SetBoneID(const QString & s)
{
    bone_id = s;
}

void DOMNode::SetBodyID(const QString & s)
{
    body_id = s;
}

void DOMNode::SetHeadID(const QString & s)
{
    head_id = s;
}

void DOMNode::SetLightmapScale(ScriptableVector * & v)
{
    *lightmap_scale = *v;
}

void DOMNode::SetLightmapScale(const QVector4D & c)
{
    lightmap_scale->SetFromOther(c);
}

void DOMNode::SetEmitterPos(ScriptableVector * & v)
{
    *emitter_pos = *v;
}

void DOMNode::SetEmitterPos(const QVector4D & c)
{
    emitter_pos->SetFromOther(c);
}

void DOMNode::SetAnimID(const QString & s)
{
    anim_id = s;
}

void DOMNode::SetAnimSpeed(const float f)
{
    anim_speed = f;
}

void DOMNode::SetCollisionResponse(const bool b)
{
    collision_response = b;
}

void DOMNode::SetCollisionFriction(const float f)
{
    collision_friction = f;
}

void DOMNode::SetCollisionRollingFriction(const float f)
{
    collision_rollingfriction = f;
}

void DOMNode::SetCollisionRestitution(const float f)
{
    collision_restitution = f;
}

void DOMNode::SetCollisionAngularDamping(const float f)
{
    collision_angulardamping = f;
}

void DOMNode::SetCollisionLinearDamping(const float f)
{
    collision_lineardamping = f;
}

void DOMNode::SetLighting(const bool b)
{
    lighting = b;
}

void DOMNode::SetCollisionStatic(const bool b)
{
    collision_static = b;
}

void DOMNode::SetCollisionTrigger(const bool b)
{
    collision_trigger = b;
}

void DOMNode::SetCollisionCcdMotionThreshold(const float param)
{
    collision_ccdmotionthreshold = param;
}

void DOMNode::SetCollisionCcdSweptSphereRadius(const float param)
{
    collision_ccdsweptsphereradius = param;
}

void DOMNode::SetVisible(const bool b)
{
    visible = b;
}

void DOMNode::SetURL(const QString & s)
{
    //qDebug() << "DOMNode::SetURL" << s;
    if (!url_changed && url != s) {
        url_changed = true;
    }
    url = s;

}

void DOMNode::SetURLChanged(const bool b)
{
    url_changed = b;
}

void DOMNode::SetSwallow(const bool b)
{
    swallow = b;
}

void DOMNode::SetBaseURL(const QString & s)
{
    base_url = s;
}

void DOMNode::SetSrc(const QString & s)
{
    src = s;
}

void DOMNode::SetVertexSrc(const QString & s)
{
    vertex_src = s;
}

void DOMNode::SetSrcURL(const QString & s)
{
    src_url = s;
}

void DOMNode::SetOriginalURL(const QString & s)
{
    url_orig = s;
}

void DOMNode::SetTitle(const QString & s)
{
    title = s;
}

void DOMNode::SetAutoLoad(const bool b)
{
    auto_load = b;
}

void DOMNode::SetDrawText(const bool b)
{
    draw_text = b;
}

void DOMNode::SetOpen(const bool b)
{
    open = b;
}

void DOMNode::SetMirror(const bool b)
{
    mirror = b;
}

void DOMNode::SetActive(const bool b)
{
    active = b;
}

void DOMNode::SetCount(const int i)
{
    count = i;
}

void DOMNode::SetRate(const int i)
{
    rate = i;
}

void DOMNode::SetDuration(const float f)
{
    duration = f;
}

void DOMNode::SetCurTime(const float f)
{
    current_time = f;
}

void DOMNode::SetTotalTime(const float f)
{
    total_time = f;
}

void DOMNode::SetFadeIn(const float f)
{
    fade_in = f;
}

void DOMNode::SetFadeOut(const float f)
{
    fade_out = f;
}

void DOMNode::SetOnClick(const QString &str)
{
    onclick_code = str;
}

void DOMNode::SetOnCollision(const QString &str)
{
    oncollision = str;
}

void DOMNode::SetOnEnter(const QString &str)
{
    onenter = str;
}

void DOMNode::SetOnExit(const QString &str)
{
    onexit = str;
}

void DOMNode::SetLightIntensity(float p_light_intensity)
{
    m_light_intensity = p_light_intensity;
}

float DOMNode::GetLightIntensity()
{
    return m_light_intensity;
}

void DOMNode::SetLightConeAngle(float p_light_cone_angle)
{
    m_light_cone_angle = p_light_cone_angle;
}

float DOMNode::GetLightConeAngle()
{
    return m_light_cone_angle;
}

void DOMNode::SetLightConeExponent(float p_light_cone_exponent)
{
    m_light_cone_exponent = p_light_cone_exponent;
}

float DOMNode::GetLightConeExponent()
{
    return m_light_cone_exponent;
}

void DOMNode::SetLightRange(float p_light_range)
{
    m_light_range = p_light_range;
}

float DOMNode::GetLightRange()
{
    return m_light_range;
}

void DOMNode::SetDrawLayer(int p_Draw_Layer)
{
    m_draw_layer = p_Draw_Layer;
}

void DOMNode::SetBackAlpha(const float f)
{
    back_alpha = f;
}

int DOMNode::GetDrawLayer()
{
    return m_draw_layer;
}

void DOMNode::SetSpinAxis(ScriptableVector *&p)
{
    if (p) {
        *spinaxis = *p;
    }
}

void DOMNode::SetSpinAxis(const QVector3D & p)
{
    spinaxis->SetFromOther(p);
}

void DOMNode::SetSpinVal(const float f)
{
    spinval = f;
}

void DOMNode::SetCollisionPos(ScriptableVector *&p)
{
    if (p) {
        *collision_pos = *p;
    }
}

void DOMNode::SetCollisionPos(const QVector3D & p)
{
    collision_pos->SetFromOther(p);
}

void DOMNode::SetCollisionScale(ScriptableVector *&p)
{
    if (p) {
        *collision_scale = *p;
    }
}

void DOMNode::SetCollisionScale(const QVector3D & p) {
    collision_scale->SetFromOther(p);
}

void DOMNode::SetSBS3D(const bool b) {
    sbs3d = b;
}

void DOMNode::SetReverse3D(const bool b) {
    reverse3d = b;
}

void DOMNode::SetOU3D(const bool b) {
    ou3d = b;
}

void DOMNode::SetTexLinear(const bool b)
{
    tex_linear = b;
}

void DOMNode::SetTexMipmap(const bool b)
{
    tex_mipmap = b;
}

void DOMNode::SetTexClamp(const bool b)
{
    tex_clamp = b;
}

void DOMNode::SetTexPremultiply(const bool b)
{
    tex_premultiply = b;
}

void DOMNode::SetTexAlpha(const QString s)
{
    tex_alpha = s;
}

void DOMNode::SetTexColorspace(const QString s)
{
    tex_colorspace = s;
}

void DOMNode::SetTex(const QString s, int i)
{
    tex[i] = s;
}

void DOMNode::SetMTL(const QString s)
{
    mtl = s;
}

void DOMNode::SetSampleRate(const int i)
{
    sample_rate = i;
}

void DOMNode::SetAutoPlay(const bool b)
{
    auto_play = b;
}

void DOMNode::SetWidth(const int i)
{
    width = i;
}

void DOMNode::SetLocked(const bool b)
{
    locked = b;
}

void DOMNode::SetHeight(const int i)
{
    height = i;
}

void DOMNode::SetNearDist(const float f)
{
    near_dist = f;
}

void DOMNode::SetFarDist(const float f)
{
    far_dist = f;
}

void DOMNode::SetTeleportMinDist(const float f)
{
    teleport_min_dist = f;
}

void DOMNode::SetTeleportMaxDist(const float f)
{
    teleport_max_dist = f;
}

void DOMNode::SetCullFace(const QString s)
{
    cull_face = s;
}

void DOMNode::SetReloaded(const bool b)
{
    reloaded = b;
}

void DOMNode::SetCircular(const bool b)
{
    circular = b;
}

void DOMNode::SetUserID(const QString s)
{
    userid = s;
}

void DOMNode::SetEyePoint(const QVector3D v)
{
    eye_point = v;
}

void DOMNode::SetServer(const QString s)
{
    server = s;
}

void DOMNode::SetServerPort(const int i)
{
    port = i;
}

void DOMNode::SetSaveToMarkup(const bool b)
{
    save_to_markup = b;
}

void DOMNode::SetWalkSpeed(const float f)
{
    walk_speed = f;
}

void DOMNode::SetRunSpeed(const float f)
{
    run_speed = f;
}

void DOMNode::SetAutoLoadTriggered(const bool b)
{
    auto_load_triggered = b;
}

void DOMNode::SetTeleportOverride(const bool b)
{
    teleport_override = b;
}

void DOMNode::SetCursorVisible(const bool b)
{
    cursor_visible = b;
}

void DOMNode::SetViewDir(const QVector3D & v)
{
    view_dir->SetFromOther(v);
}

void DOMNode::SetViewDir(ScriptableVector * v)
{
    if (v)
        *view_dir = *v;
}

ScriptableVector * DOMNode::GetViewDir()
{
    return view_dir;
}

void DOMNode::SetUpDir(const QVector3D & v)
{
    up_dir->SetFromOther(v);
}

void DOMNode::SetUpDir(ScriptableVector * v)
{
    if (v)
        *up_dir = *v;
}

ScriptableVector * DOMNode::GetUpDir()
{
    return up_dir;
}

void DOMNode::SetCursor0Active(const bool b)
{
    cursor0_active = b;
}

bool DOMNode::GetCursor0Active() const
{
    return cursor0_active;
}

void DOMNode::SetCursor0Object(const QString & s)
{
    cursor0_object = s;
}

QString DOMNode::GetCursor0Object() const
{
    return cursor0_object;
}

void DOMNode::SetCursor0Pos(const QVector3D &p)
{
    cursor0_pos->SetFromOther(p);
}

void DOMNode::SetCursor0Pos(ScriptableVector *&p)
{
    if (p)
        *cursor0_pos = *p;
}

ScriptableVector * DOMNode::GetCursor0Pos()
{
    return cursor0_pos;
}

void DOMNode::SetCursor0XDir(const QVector3D &p)
{
    cursor0_xdir->SetFromOther(p);
}

void DOMNode::SetCursor0XDir(ScriptableVector *&p)
{
    if (p)
        *cursor0_xdir = *p;
}

ScriptableVector * DOMNode::GetCursor0XDir()
{
    return cursor0_xdir;
}

void DOMNode::SetCursor0YDir(const QVector3D &p)
{
    cursor0_ydir->SetFromOther(p);
}

void DOMNode::SetCursor0YDir(ScriptableVector *&p)
{
    if (p)
        *cursor0_ydir = *p;
}

ScriptableVector * DOMNode::GetCursor0YDir()
{
    return cursor0_ydir;
}

void DOMNode::SetCursor0ZDir(const QVector3D &p)
{
    cursor0_zdir->SetFromOther(p);
}

void DOMNode::SetCursor0ZDir(ScriptableVector *&p)
{
    if (p)
        *cursor0_zdir = *p;
}

ScriptableVector * DOMNode::GetCursor0ZDir()
{
    return cursor0_zdir;
}

void DOMNode::SetCursor1Active(const bool b)
{
    cursor1_active = b;
}

bool DOMNode::GetCursor1Active() const
{
    return cursor1_active;
}

void DOMNode::SetCursor1Object(const QString & s)
{
    cursor1_object = s;
}

QString DOMNode::GetCursor1Object() const
{
    return cursor1_object;
}

void DOMNode::SetCursor1Pos(const QVector3D &p)
{
    cursor1_pos->SetFromOther(p);
}

void DOMNode::SetCursor1Pos(ScriptableVector *&p)
{
    if (p)
        *cursor1_pos = *p;
}

ScriptableVector * DOMNode::GetCursor1Pos()
{
    return cursor1_pos;
}

void DOMNode::SetCursor1XDir(const QVector3D &p)
{
    cursor1_xdir->SetFromOther(p);
}

void DOMNode::SetCursor1XDir(ScriptableVector *&p)
{
    if (p)
        *cursor1_xdir = *p;
}

ScriptableVector * DOMNode::GetCursor1XDir()
{
    return cursor1_xdir;
}

void DOMNode::SetCursor1YDir(const QVector3D &p)
{
    cursor1_ydir->SetFromOther(p);
}

void DOMNode::SetCursor1YDir(ScriptableVector *&p)
{
    if (p)
        *cursor1_ydir = *p;
}

ScriptableVector * DOMNode::GetCursor1YDir()
{
    return cursor1_ydir;
}

void DOMNode::SetCursor1ZDir(const QVector3D &p)
{
    cursor1_zdir->SetFromOther(p);
}

void DOMNode::SetCursor1ZDir(ScriptableVector *&p)
{
    if (p)
        *cursor1_zdir = *p;
}

ScriptableVector * DOMNode::GetCursor1ZDir()
{
    return cursor1_zdir;
}

void DOMNode::SetHand0Active(const bool b)
{
    hand0_active = b;
}

bool DOMNode::GetHand0Active()
{
    return hand0_active;
}

void DOMNode::SetHand0Trackpad(const QVector3D &p)
{
    hand0_trackpad->SetFromOther(p);
}

void DOMNode::SetHand0Trackpad(ScriptableVector *&p)
{
    if (p)
        *hand0_trackpad = *p;
}

ScriptableVector * DOMNode::GetHand0Trackpad()
{
    return hand0_trackpad;
}

void DOMNode::SetHand1Trackpad(const QVector3D &p)
{
    hand1_trackpad->SetFromOther(p);
}

void DOMNode::SetHand1Trackpad(ScriptableVector *&p)
{
    if (p)
        *hand1_trackpad = *p;
}

ScriptableVector * DOMNode::GetHand1Trackpad()
{
    return hand1_trackpad;
}

void DOMNode::SetHand0Pos(const QVector3D &p)
{
    hand0_pos->SetFromOther(p);
}

void DOMNode::SetHand0Pos(ScriptableVector *&p)
{
    if (p)
        *hand0_pos = *p;
}

ScriptableVector * DOMNode::GetHand0Pos()
{
    return hand0_pos;
}

void DOMNode::SetHand0Vel(const QVector3D &p)
{
    hand0_vel->SetFromOther(p);
}

void DOMNode::SetHand0Vel(ScriptableVector *&p)
{
    if (p)
        *hand0_vel = *p;
}

ScriptableVector * DOMNode::GetHand0Vel()
{
    return hand0_vel;
}

void DOMNode::SetHand0XDir(const QVector3D &p)
{
    hand0_xdir->SetFromOther(p);
}

void DOMNode::SetHand0XDir(ScriptableVector *&p)
{
    if (p)
        *hand0_xdir = *p;
}

ScriptableVector * DOMNode::GetHand0XDir()
{
    return hand0_xdir;
}

void DOMNode::SetHand0YDir(const QVector3D &p)
{
    hand0_ydir->SetFromOther(p);
}

void DOMNode::SetHand0YDir(ScriptableVector *&p)
{
    if (p)
        *hand0_ydir = *p;
}

ScriptableVector * DOMNode::GetHand0YDir()
{
    return hand0_ydir;
}

void DOMNode::SetHand0ZDir(const QVector3D &p)
{
    hand0_zdir->SetFromOther(p);
}

void DOMNode::SetHand0ZDir(ScriptableVector *&p)
{
    if (p)
        *hand0_zdir = *p;
}

ScriptableVector * DOMNode::GetHand0ZDir()
{
    return hand0_zdir;
}

void DOMNode::SetHand1Active(const bool b)
{
    hand1_active = b;
}

bool DOMNode::GetHand1Active()
{
    return hand1_active;
}

void DOMNode::SetHand1Pos(const QVector3D &p)
{
    hand1_pos->SetFromOther(p);
}

void DOMNode::SetHand1Pos(ScriptableVector *&p)
{
    if (p)
        *hand1_pos = *p;
}

ScriptableVector * DOMNode::GetHand1Pos()
{
    return hand1_pos;
}

void DOMNode::SetHand1Vel(const QVector3D &p)
{
    hand1_vel->SetFromOther(p);
}

void DOMNode::SetHand1Vel(ScriptableVector *&p)
{
    if (p)
        *hand1_vel = *p;
}

ScriptableVector * DOMNode::GetHand1Vel()
{
    return hand1_vel;
}

void DOMNode::SetHand1XDir(const QVector3D &p)
{
    hand1_xdir->SetFromOther(p);
}

void DOMNode::SetHand1XDir(ScriptableVector *&p)
{
    if (p)
        *hand1_xdir = *p;
}

ScriptableVector * DOMNode::GetHand1XDir()
{
    return hand1_xdir;
}

void DOMNode::SetHand1YDir(const QVector3D &p)
{
    hand1_ydir->SetFromOther(p);
}

void DOMNode::SetHand1YDir(ScriptableVector *&p)
{
    if (p)
        *hand1_ydir = *p;
}

ScriptableVector * DOMNode::GetHand1YDir()
{
    return hand1_ydir;
}

void DOMNode::SetHand1ZDir(const QVector3D &p)
{
    hand1_zdir->SetFromOther(p);
}

void DOMNode::SetHand1ZDir(ScriptableVector *&p)
{
    if (p)
        *hand1_zdir = *p;
}

ScriptableVector * DOMNode::GetHand1ZDir()
{
    return hand1_zdir;
}

QString DOMNode::GetHMDType()
{
    return hmd_type;
}

void DOMNode::SetHMDType(const QString &s)
{
    hmd_type = s;
}

QString DOMNode::GetDeviceType()
{
    return device_type;
}

void DOMNode::SetDeviceType(const QString &s)
{
    device_type = s;
}

QString DOMNode::ElementTypeToString(const ElementType t)
{
    switch (t) {
    case TYPE_GHOST:
        return "ghost";
    case TYPE_IMAGE:
        return "image";
    case TYPE_LIGHT:
        return "light";
    case TYPE_LINK:
        return "link";
    case TYPE_OBJECT:
        return "object";
    case TYPE_PARAGRAPH:
        return "paragraph";
    case TYPE_PARTICLE:
        return "particle";
    case TYPE_SOUND:
        return "sound";
    case TYPE_TEXT:
        return "text";
    case TYPE_VIDEO:
        return "video";
    case TYPE_ROOM:
        return "room";
    case TYPE_PLAYER:
        return "player";
    case TYPE_ASSET:
        return "asset";
    case TYPE_ASSETGHOST:
        return "assetghost";
    case TYPE_ASSETIMAGE:
        return "assetimage";
    case TYPE_ASSETOBJECT:
        return "assetobject";
    case TYPE_ASSETRECORDING:
        return "assetrecording";
    case TYPE_ASSETSCRIPT:
        return "assetscript";
    case TYPE_ASSETSHADER:
        return "assetshader";
    case TYPE_ASSETSOUND:
        return "assetsound";
    case TYPE_ASSETVIDEO:
        return "assetvideo";
    case TYPE_ASSETWEBSURFACE:
        return "assetwebsurface";
    case TYPE_ERROR:
    default:
        return "error";
    }
}

QString DOMNode::ElementTypeToTagName(const ElementType t)
{
    switch (t) {
    case TYPE_GHOST:
        return "Ghost";
    case TYPE_IMAGE:
        return "Image";
    case TYPE_LIGHT:
        return "Light";
    case TYPE_LINK:
        return "Link";
    case TYPE_OBJECT:
        return "Object";
    case TYPE_PARAGRAPH:
        return "Paragraph";
    case TYPE_PARTICLE:
        return "Particle";
    case TYPE_SOUND:
        return "Sound";
    case TYPE_TEXT:
        return "Text";
    case TYPE_VIDEO:
        return "Video";
    case TYPE_ROOM:
        return "Room";
    case TYPE_PLAYER:
        return "Player";
    case TYPE_ASSET:
        return "Asset";
    case TYPE_ASSETGHOST:
        return "AssetGhost";
    case TYPE_ASSETIMAGE:
        return "AssetImage";
    case TYPE_ASSETOBJECT:
        return "AssetObject";
    case TYPE_ASSETRECORDING:
        return "AssetRecording";
    case TYPE_ASSETSCRIPT:
        return "AssetScript";
    case TYPE_ASSETSHADER:
        return "AssetShader";
    case TYPE_ASSETSOUND:
        return "AssetSound";
    case TYPE_ASSETVIDEO:
        return "AssetVideo";
    case TYPE_ASSETWEBSURFACE:
        return "AssetWebSurface";
    case TYPE_ERROR:
    default:
        return "error";
    }
}

ElementType DOMNode::StringToElementType(const QString name)
{
    const QString s = name.toLower().trimmed();
    if (s == "ghost") {
        return TYPE_GHOST;
    }
    else if (s == "image") {
        return TYPE_IMAGE;
    }
    else if (s == "light") {
        return TYPE_LIGHT;
    }
    else if (s == "link") {
        return TYPE_LINK;
    }    
    else if (s == "paragraph") {
        return TYPE_PARAGRAPH;
    }
    else if (s == "particle") {
        return TYPE_PARTICLE;
    }
    else if (s == "sound") {
        return TYPE_SOUND;
    }
    else if (s == "text") {
        return TYPE_TEXT;
    }    
    else if (s == "room") {
        return TYPE_ROOM;
    }
    else if (s == "player") {
        return TYPE_PLAYER;
    }
    else if (s == "asset") {
        return TYPE_ASSET;
    }
    else if (s == "assetghost") {
        return TYPE_ASSETGHOST;
    }
    else if (s == "assetimage") {
        return TYPE_ASSETIMAGE;
    }
    else if (s == "assetobject") {
        return TYPE_ASSETOBJECT;
    }
    else if (s == "assetrecording") {
        return TYPE_ASSETRECORDING;
    }
    else if (s == "assetscript") {
        return TYPE_ASSETSCRIPT;
    }
    else if (s == "assetshader") {
        return TYPE_ASSETSHADER;
    }
    else if (s == "assetsound") {
        return TYPE_ASSETSOUND;
    }
    else if (s == "assetvideo") {
        return TYPE_ASSETVIDEO;
    }
    else if (s == "assetwebsurface") {
        return TYPE_ASSETWEBSURFACE;
    }
    else if (s.left(5) == "video") {
        return TYPE_VIDEO;
    }
    else { //60.1 - error tolerance, all unknown tags assumed to be Object
        return TYPE_OBJECT;
    }
}

void DOMNode::SetXDirs(const QString & x, const QString & y, const QString & z)
{
    if (!x.isEmpty()) {
        SetXDir(MathUtil::GetStringAsVector(x));
    }
    if (!y.isEmpty()) {
        SetYDir(MathUtil::GetStringAsVector(y));
    }
    if (!z.isEmpty()) {
        SetZDir(MathUtil::GetStringAsVector(z).normalized());
    }

    const QVector3D dy = GetYDir()->toQVector3D();
    const QVector3D dz = GetZDir()->toQVector3D();

    SetYDir((dy - dz * QVector3D::dotProduct(dy, dz)).normalized());
    SetXDir(QVector3D::crossProduct(dy, dz));
}

void DOMNode::SetXDirs(const QVector3D & x, const QVector3D & y, const QVector3D & z)
{
    SetXDir(x);
    SetYDir(y);
    SetZDir(z.normalized());

    const QVector3D dy = GetYDir()->toQVector3D();
    const QVector3D dz = GetZDir()->toQVector3D();

    //re-orthgonalize/ortho-normalize
    SetYDir((dy - dz * QVector3D::dotProduct(dy, dz)).normalized());
    SetXDir(QVector3D::crossProduct(dy, dz));
    //qDebug() << "setting xdirs" << x << y << z << GetXDir()->toQVector3D() << GetYDir()->toQVector3D() << GetZDir()->toQVector3D();
}

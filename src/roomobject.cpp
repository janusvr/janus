#include "roomobject.h"

QPointer <AssetImage> RoomObject::linear_gradient_img;
QPointer <AssetImage> RoomObject::sound_img;
QPointer <AssetImage> RoomObject::light_img;
QPointer <AssetImage> RoomObject::object_img;
QPointer <AssetImage> RoomObject::particle_img;
QPointer <AssetObject> RoomObject::cursor_arrow_obj;
QPointer <AssetObject> RoomObject::cursor_crosshair_obj;
QPointer <AssetObject> RoomObject::cursor_hand_obj;
QPointer <AssetObject> RoomObject::avatar_obj;
QPointer <AssetObject> RoomObject::avatar_head_obj;

int RoomObject::objects_allocated = 0;
float RoomObject::portal_spacing = 0.1f;
bool RoomObject::draw_assetobject_teleport = false;

//Network-related rates, in seconds
float RoomObject::rate = 0.2f;
float RoomObject::logoff_rate = 3.0f;

RoomObject::RoomObject() :
    props(new DOMNode()),
    save_to_markup(true),
    grabbed(false),
    draw_back(false),
    interface_object(false)
{
    ++objects_allocated;

    SetType(TYPE_OBJECT);
    last_rotation = QVector3D(0,0,0);
    selected = false;    
    tex_width = 1024;
    tex_height = 1024;    
    playing = false;    
    do_multiplayer_timeout = false;    
    play_sounds = true;        
    time_elapsed = 0.0f;
    head_avatar_pos = QVector3D(0,1,0);

    eye_ipd = 0.0f;  
    error_reported = false;
    openal_stream_source = 0;

    assetghost = 0;
    assetimage = 0;
    assetsound = 0;
    assetobject_anim = 0;
    assetobject = 0;
    assetobject_collision = 0;    
    assetobject_emitter = 0;
    assetshader = 0;
    assetvideo = 0;
    assetwebsurface = 0;
    assetobject_teleport = 0;    
    ghost_frame_index = -1; //49.50 ensures ghosts always load, even for single-frame animations where the frameindex = 0

    interp_val = 1.0f;
    interp_time = 0.1f;
    interp_timer.start();

    particle_system = new ParticleSystem();

    assetobject_blendshapes = QVector<QPointer <AssetObject> >(4);

    textgeom = QPointer <TextGeom> (new TextGeom());
    textgeom_player_id = QPointer <TextGeom> (new TextGeom());
    textgeom_player_id->SetFixedSize(true, 0.075f);

    textgeom_url = QPointer <TextGeom> (new TextGeom());
    textgeom_title = QPointer <TextGeom> (new TextGeom());

    player_in_room = false;
    player_in_adjacent_room = false;

    m_cubemap_radiance = 0;
    m_cubemap_irradiance = 0;
    assetimage_lmap = 0;

    rescale_on_load = false;
    rescale_on_load_aspect = false;
    rescale_on_load_done = false;

    sound_buffers_sample_rate = 44100;

    swallow_time = 0;
    swallow_state = 0;
}

RoomObject::~RoomObject()
{
    //qDebug() << "RoomObject::~RoomObject()" << this;
    --objects_allocated;
    Clear();

    if (particle_system) {
        delete particle_system;
    }

    if (portal_text) {
        delete portal_text;
    }

    if (props) {
        delete props;
    }
}

QPointer <DOMNode> RoomObject::GetProperties()
{
    return props;
}

void RoomObject::Clear()
{
    //Note: envobject does not do any deallocation - it only holds pointers to things
    collision_set.clear();

    if (assetsound) {
        AssetSound::ClearOutput(&media_ctx);
    }

    if (assetvideo) {
        AssetVideo::ClearOutput(&media_ctx);
    }

    if (!sound_buffers.isEmpty()) {
        sound_buffers.clear();
    }

    if (openal_stream_source > 0) {
        ALint is_playing;
        alGetSourcei(openal_stream_source, AL_SOURCE_STATE, &is_playing);
        if (is_playing == AL_PLAYING) {
            alSourceStop(openal_stream_source);
        }

        //Dequeue buffers
        int buffers_to_dequeue = 0;
        alGetSourcei(openal_stream_source, AL_BUFFERS_QUEUED, &buffers_to_dequeue);
        if (buffers_to_dequeue > 0)
        {
            QVector<ALuint> buffHolder;
            buffHolder.resize(buffers_to_dequeue);
            alSourceUnqueueBuffers(openal_stream_source, buffers_to_dequeue, buffHolder.data());
            for (int i=0;i<buffers_to_dequeue;++i) {
                // Push the recovered buffers back on the queue
                SoundManager::buffer_input_queue.push_back(buffHolder[i]);
            }
        }

        alSourcei(openal_stream_source, AL_BUFFER, 0);

        alDeleteSources(1, &openal_stream_source);
        openal_stream_source = 0;
    }

    assetshader.clear();
    // TYPE_PARAGRAPH creates it's own assetimage separate from the usual asset creation pipeline
    // so in this edge-case we need to delete the assetimage to stop it leaking.
    if (GetType() == TYPE_PARAGRAPH && assetimage) {
        delete assetimage;
    }
    assetimage.clear();
    assetsound.clear();
    assetvideo.clear();
    assetwebsurface.clear();

    assetvideo.clear();
    assetobject.clear();
    assetobject_collision.clear();
    assetobject_emitter.clear();
    assetobject_teleport.clear();
    assetobject_anim.clear();
    assetghost.clear();
    assetobject_blendshapes.clear();

    ghost_assetobjs.clear();
    ghost_asset_shaders.clear();

    for (int i=0; i<child_objects.size(); ++i) {
        if (child_objects[i]) {
            delete child_objects[i];
        }
    }
    child_objects.clear();

    if (textgeom) {
        delete textgeom;
    }
    if (textgeom_player_id) {
        delete textgeom_player_id;
    }
    if (textgeom_url) {
        delete textgeom_url;
    }
    if (textgeom_title) {
        delete textgeom_title;
    }

    player_in_room = false;
    player_in_adjacent_room = false;
}

void RoomObject::initializeGL()
{
    if (linear_gradient_img.isNull()) {
        linear_gradient_img = QPointer<AssetImage>(new AssetImage());
        linear_gradient_img->GetProperties()->SetTexClamp(true);
        linear_gradient_img->SetSrc(MathUtil::GetApplicationURL(), "assets/linear_gradient.png");
        linear_gradient_img->Load();
    }

    if (sound_img.isNull()) {
        sound_img = new AssetImage();
        sound_img = QPointer<AssetImage>(new AssetImage());
        sound_img->SetSrc(MathUtil::GetApplicationURL(), "assets/sound.png");
        sound_img->Load();
    }

    if (light_img.isNull()) {
        light_img = new AssetImage();
        light_img = QPointer<AssetImage>(new AssetImage());
        light_img->SetSrc(MathUtil::GetApplicationURL(), "assets/light.png");
        light_img->Load();
    }

    if (object_img.isNull()) {
        object_img = new AssetImage();
        object_img = QPointer<AssetImage>(new AssetImage());
        object_img->SetSrc(MathUtil::GetApplicationURL(), "assets/object.png");
        object_img->Load();
    }

    if (particle_img.isNull()) {
        particle_img = new AssetImage();
        particle_img = QPointer<AssetImage>(new AssetImage());
        particle_img->SetSrc(MathUtil::GetApplicationURL(), "assets/particle.png");
        particle_img->Load();
    }

    if (cursor_arrow_obj.isNull()) {
        cursor_arrow_obj = new AssetObject();
        cursor_arrow_obj->SetSrc(MathUtil::GetApplicationURL(), "assets/primitives/plane.obj");
        cursor_arrow_obj->SetTextureClamp(true);
        cursor_arrow_obj->SetTextureAlphaType("blended");
        cursor_arrow_obj->SetTextureFile("./assets/cursor/arrow.png", 0);
        cursor_arrow_obj->Load();
    }

    if (cursor_crosshair_obj.isNull()) {
        cursor_crosshair_obj = new AssetObject();
        cursor_crosshair_obj->SetSrc(MathUtil::GetApplicationURL(), "assets/primitives/plane.obj");
        cursor_crosshair_obj->SetTextureClamp(true);
        cursor_crosshair_obj->SetTextureLinear(true);
        cursor_crosshair_obj->SetTextureMipmap(true);
        cursor_crosshair_obj->SetTextureAlphaType("blended");
        cursor_crosshair_obj->SetTextureFile("./assets/cursor/crosshair.png", 0);
        cursor_crosshair_obj->Load();
    }

    if (cursor_hand_obj.isNull()) {
        cursor_hand_obj = new AssetObject();
        cursor_hand_obj->SetSrc(MathUtil::GetApplicationURL(), "assets/primitives/plane.obj");
        cursor_hand_obj->SetTextureClamp(true);
        cursor_hand_obj->SetTextureLinear(true);
        cursor_hand_obj->SetTextureMipmap(true);
        cursor_hand_obj->SetTextureAlphaType("blended");
        cursor_hand_obj->SetTextureFile("./assets/cursor/hand.png", 0);
        cursor_hand_obj->Load();
    }

    if (avatar_obj.isNull()) {
        avatar_obj = new AssetObject();
        avatar_obj->SetSrc(MathUtil::GetApplicationURL(), "assets/avatar.obj");
        avatar_obj->SetTextureFile("./assets/avatar.png", 0);
        avatar_obj->Load();
    }

    if (avatar_head_obj.isNull()) {
        avatar_head_obj = new AssetObject();
        avatar_head_obj->SetSrc(MathUtil::GetApplicationURL(), "assets/avatar_head.obj");
        avatar_head_obj->SetTextureFile("./assets/avatar_head.png", 0);
        avatar_head_obj->Load();
    }
}

void RoomObject::SetType(const ElementType t)
{
    //qDebug() << "RoomObject::SetType" << t;
    props->SetType(t);

    if (t == TYPE_LINK) {
        props->SetScale(QVector3D(1.8f, 2.5f, 1.0f));

        //portals
        if (portal_text.isNull()) {
            portal_text = new RoomObject();
            portal_text->SetType(TYPE_PARAGRAPH);
            portal_text->GetProperties()->SetBackAlpha(0.0f);
            portal_text->GetProperties()->SetFontSize(32);
            portal_text->GetProperties()->SetPos(QVector3D(0.0f, 0.0f, 0.0f));
            portal_text->GetProperties()->SetDir(QVector3D(0.0f, 0.0f, 1.0f));
            portal_text->GetProperties()->SetLighting(false);
        }
    }
    //qDebug() << "RoomObject::SetType"  << GetJSID() << GetID() << "allocated" << objects_allocated << "type" << GetType();
}

ElementType RoomObject::GetType() const
{
    return props->GetType();
}

void RoomObject::SetInterfaceObject(const bool b)
{
    interface_object = b;
}

bool RoomObject::GetInterfaceObject() const
{
    return interface_object;
}

void RoomObject::SetProperties(QPointer<DOMNode> props)
{
    this->props = props;
}

void RoomObject::SetProperties(QVariantMap d)
{
    if (d.contains("url")) {
        SetURL(props->GetURL(), d["url"].toString());
    }
    if (d.contains("head_pos")) {
        SetHeadAvatarPos(MathUtil::GetVectorFromQVariant(d["head_pos"])); //56.0 - bugfix @attention joseph, head_pos in markup set's avatar_head_pos (confusing, I know)
    }
    if (d.contains("userid_pos")) {
        SetUserIDPos(MathUtil::GetVectorFromQVariant(d["userid_pos"]));
    }

    props->SetProperties(d);
}

bool RoomObject::GetRaycastIntersection(const QMatrix4x4 transform, QList <QVector3D> & int_verts, QList <QVector3D> & int_normals, QList <QVector2D> & int_texcoords)
{
    //qDebug() << "RoomObject::GetRaycastIntersection" << this->GetID() << this->GetCollisionID();
    const bool visible = props->GetVisible();
    const bool edit_mode_enabled = SettingsManager::GetEditModeEnabled();
    const bool edit_mode_icons_enabled = SettingsManager::GetEditModeIconsEnabled();
    const QVector3D ray_p = transform.column(3).toVector3D();
    const QVector3D ray_d = transform.column(2).toVector3D();

    QList <QVector3D> int_pts;

    const ElementType t = GetType();
    switch (t) {
    case TYPE_SOUND:
    case TYPE_LIGHT:
    case TYPE_PARTICLE:
        if (edit_mode_enabled && edit_mode_icons_enabled) {
            //manip handle sphere test
            Sphere3D s;
            s.cent = GetPos();
            s.rad = SpinAnimation::GetIconScale() * 0.5f;

            int nbInter;
            float inter1, inter2;
            if (MathUtil::testIntersectionSphereLine(s, ray_p, ray_p + ray_d, nbInter, inter1, inter2)) {

                if (nbInter >= 1) {
                    int_verts.push_back(ray_p + ray_d * inter1);
                    int_normals.push_back(-ray_d);
                    int_texcoords.push_back(QVector2D(0,0));
                }
                if (nbInter >= 2) {
                    int_verts.push_back(ray_p + ray_d * inter2);
                    int_normals.push_back(-ray_d);
                    int_texcoords.push_back(QVector2D(0,0));
                }
            }
        }
        break;

    case TYPE_OBJECT:
    {
        //other tests
        QPointer <AssetObject> obj;

        bool do_collider_transform = false;
        //use the teleport assetobject if it exists, otherwise use the geometry itself
        if (assetobject_teleport) {
            obj = assetobject_teleport;
        }
        else if (assetobject && visible && (props->GetWebsurfaceID().length() > 0 || props->GetVideoID().length() > 0)) {
            obj = assetobject;
        }
        else if (assetobject_collision) {
            obj = assetobject_collision;
            do_collider_transform = true;
        }

        if (obj && obj->GetFinished())
        {
            const QVector3D bbox_min = obj->GetBBoxMin();
            const QVector3D bbox_max = obj->GetBBoxMax();

            QMatrix4x4 model_matrix = model_matrix_global;

            if (do_collider_transform)
            {
                const QVector3D cp = props->GetCollisionPos()->toQVector3D();
                const QVector3D cs = props->GetCollisionScale()->toQVector3D();
                model_matrix.translate(cp.x(), cp.y(), cp.z());
                model_matrix.scale(cs.x(), cs.y(), cs.z());
            }

            const QMatrix4x4 model_matrix_inv = model_matrix.inverted();
            const QVector3D ray_pos_object_space = model_matrix_inv.map(ray_p);
            const QVector3D ray_dir_object_space = model_matrix_inv.mapVector(ray_d);

            //determine if the ray crosses the bounding sphere which encapsulates this bounding box
            Sphere3D s;
            s.cent = (bbox_min + bbox_max) * 0.5f;
            s.rad = (bbox_max - bbox_min).length() * 0.5f;

            int nbInter;
            float inter1, inter2;

            // If the ray intersects the bounding sphere for this AssetObject
            if (MathUtil::testIntersectionSphereLine(s, ray_pos_object_space, ray_pos_object_space + ray_dir_object_space, nbInter, inter1, inter2))
            {
                const QList <QString> materials = obj->GetGeom()->GetData().GetMaterialNames();

                // For each material used in this AssetObject
                for (int k=0; k<materials.size(); ++k)
                {
                    // Get the unordered_map of Meshes(QVector <GeomTriangle>) that use this material
                    GeomMaterial & material = obj->GetGeom()->GetData().GetMaterial(materials[k]);
                    QVector<QVector<GeomTriangle>>& triangles_map = material.triangles;
                    QVector<GeomVBOData>& vbo_map = material.vbo_data;
                    QVector<QPair<uint32_t, int>>& mesh_keys = material.mesh_keys;
                    const int mesh_count = mesh_keys.size();

                    // For each mesh that uses this material of this AssetObject
                    for (int mesh_index = 0; mesh_index < mesh_count; ++mesh_index)
                    {
                        const QVector<GeomTriangle> & triangles = triangles_map[mesh_keys[mesh_index].second];
                        const GeomVBOData & vbo_data = vbo_map[mesh_keys[mesh_index].second];
                        const QVector <QMatrix4x4>& mesh_instance_transforms = vbo_data.m_instance_transforms;

                        // For each instance of this mesh
                        QVector3D ray_pos_instance_space;
                        QVector3D ray_dir_instance_space;
                        QMatrix4x4 inv_instance_position_transform;
                        QMatrix4x4 instance_normal_transform;
                        QMatrix4x4 inv_instance_normal_transform;

                        const int instance_count = mesh_instance_transforms.size();
                        for (int instance_index = 0; instance_index < instance_count; ++instance_index)
                        {
                            // Transform ray into instance-local space
                            inv_instance_position_transform = mesh_instance_transforms[instance_index].inverted();
                            instance_normal_transform = mesh_instance_transforms[instance_index].inverted().transposed();
                            inv_instance_normal_transform = instance_normal_transform.inverted();
                            ray_pos_instance_space = inv_instance_position_transform.map(ray_pos_object_space);
                            ray_dir_instance_space = inv_instance_position_transform.mapVector(ray_dir_object_space);
                            ray_dir_instance_space.normalize();

                            // Raycast against the first 16k triangles of a mesh
                            const int triangle_loop_count = qMin(triangles.size(), 16384);
                            for (int i = 0; i < triangle_loop_count; ++i)
                            {
                                // per-triangle test
                                QVector3D p0 = QVector3D(triangles[i].p[0][0],
                                                         triangles[i].p[0][1],
                                                         triangles[i].p[0][2]);

                                QVector3D p1 = QVector3D(triangles[i].p[1][0],
                                                         triangles[i].p[1][1],
                                                         triangles[i].p[1][2]);

                                QVector3D p2 = QVector3D(triangles[i].p[2][0],
                                                         triangles[i].p[2][1],
                                                         triangles[i].p[2][2]);

                                QVector3D int_pt;
                                if (MathUtil::GetRayTriIntersect(ray_pos_instance_space, ray_dir_instance_space, p0, p1, p2, int_pt)
                                    && QVector3D::dotProduct(int_pt - ray_pos_instance_space, ray_dir_instance_space) > 0.0f)
                                {
                                    // If sucess, compute barycentric to get interpolated pos, normal, and UV for the intersection point.
                                    float f0, f1, f2;
                                    MathUtil::ComputeBarycentric3D(int_pt, p0, p1, p2, f0, f1, f2);

                                    // Map the barycentric position back into instance-space then back into room-space
                                    QVector4D room_space_intersection_pos = model_matrix.map(mesh_instance_transforms[instance_index].map(int_pt));
                                    int_verts.push_back(room_space_intersection_pos.toVector3D());

                                    // Map the barycentric normal back into instance-space then back into room-space
                                    QVector3D barycentric_normal = QVector3D(triangles[i].n[0][0], triangles[i].n[0][1], triangles[i].n[0][2]) * f0 +
                                                                   QVector3D(triangles[i].n[1][0], triangles[i].n[1][1], triangles[i].n[1][2]) * f1 +
                                                                   QVector3D(triangles[i].n[2][0], triangles[i].n[2][1], triangles[i].n[2][2]) * f2;
                                    barycentric_normal.normalize();
                                    QVector3D room_space_intersection_normal = model_matrix.mapVector(instance_normal_transform.mapVector(barycentric_normal).normalized());
                                    room_space_intersection_normal.normalize();
                                    int_normals.push_back(room_space_intersection_normal);

                                    // UV
                                    QVector2D int_uv = QVector2D(triangles[i].t[0][0], triangles[i].t[0][1]) * f0 +
                                                       QVector2D(triangles[i].t[1][0], triangles[i].t[1][1]) * f1 +
                                                       QVector2D(triangles[i].t[2][0], triangles[i].t[2][1]) * f2;
                                    int_uv.setY(1.0f - int_uv.y()); //UVs are flipped vertically
                                    int_texcoords.push_back(int_uv);
                                }
                            }
                        }
                    }
                }
            }
        }

        //manip handle sphere test 57.1 - note, it should not get in way of normal interaction
        if (edit_mode_enabled && edit_mode_icons_enabled && int_verts.empty() && !interface_object) {
            Sphere3D s;
            s.cent = GetPos();
            s.rad = SpinAnimation::GetIconScale() * 0.5f;

            int nbInter;
            float inter1, inter2;
            if (MathUtil::testIntersectionSphereLine(s, ray_p, ray_p + ray_d, nbInter, inter1, inter2)) {

                if (nbInter >= 1) {
                    int_verts.push_back(ray_p + ray_d * inter1);
                    int_normals.push_back(-ray_d);
                    int_texcoords.push_back(QVector2D(0,0));
                }
                if (nbInter >= 2) {
                    int_verts.push_back(ray_p + ray_d * inter2);
                    int_normals.push_back(-ray_d);
                    int_texcoords.push_back(QVector2D(0,0));
                }
            }
        }
    }
        break;

    case TYPE_GHOST:
        //compute cursor intersection for ghost
        //billboard plane
        if (visible) {
            QVector3D p = GetPos();
            QVector3D z = ray_p - p;
            z.setY(0.0f);
            z.normalize();
            QVector3D y(0,1,0);
            QVector3D x = QVector3D::crossProduct(y,z).normalized();

            QVector3D int_pt;
            if (MathUtil::LinePlaneIntersection(p, z, ray_p, ray_p + ray_d, int_pt) && QVector3D::dotProduct(int_pt - ray_p, ray_d) > 0.0f) {
                const float x_dot = fabsf(QVector3D::dotProduct(int_pt - p, x));
                const float y_dot = QVector3D::dotProduct(int_pt - p, y);

                if (x_dot < 0.5f && y_dot > 0.0f && y_dot < 1.8f) {
                    int_verts.push_back(int_pt);
                    int_normals.push_back(z);
                    int_texcoords.push_back(QVector2D(x_dot, y_dot));
                }
            }
        }
        break;

    case TYPE_IMAGE:
    case TYPE_VIDEO:
    case TYPE_LINK:
    case TYPE_TEXT:
    case TYPE_PARAGRAPH:
        if (visible) {
            QVector4D bbox_min(-1,-1,0,1);
            QVector4D bbox_max(1,1,0.1f,1);
            QMatrix4x4 m = model_matrix_local;

            switch (t) {
            case TYPE_IMAGE:
                if (assetimage) {
                    m.scale(1.0f, assetimage->GetAspectRatio(), 1.0f);
                }
                break;
            case TYPE_VIDEO:
                if (assetimage) {
                    m.scale(1.0f, assetimage->GetAspectRatio(), 1.0f);
                }
                else if (assetvideo) {
                    m.scale(1.0f, assetvideo->GetAspectRatio(&media_ctx), 1.0f);
                }
                break;
            case TYPE_TEXT:
                if (textgeom) {
                    const float f = float(textgeom->GetText().length());
                    m.scale(f * 0.5f, 1, 1);
                    m.translate(1, 0, 0);
                }
                break;
            default:
            {
                const float w = GetScale().x() * 0.5f;
                const float h = GetScale().y() * 0.5f;
                m.translate(0, h, 0);
                m.scale(w, h, 1.0f);
            }
                break;
            }

            const QVector4D transform_bbox_min = (m * bbox_min);
            const QVector4D transform_bbox_max = (m * bbox_max);
            const QVector3D cent = (transform_bbox_min.toVector3D() + transform_bbox_max.toVector3D()) * 0.5f;

            QVector3D int_pt;
            if (MathUtil::LinePlaneIntersection(cent, GetZDir(), ray_p, ray_p + ray_d, int_pt) && QVector3D::dotProduct(int_pt - ray_p, ray_d) > 0.0f) {

                const QVector3D x = m.column(0).toVector3D();
                const QVector3D y = m.column(1).toVector3D();

                const float x_dot = fabsf(QVector3D::dotProduct(int_pt - cent, x.normalized()));
                const float y_dot = fabsf(QVector3D::dotProduct(int_pt - cent, y.normalized()));

                if (x_dot < x.length() && y_dot < y.length()) {
                    //qDebug() << "DOT PRODS ARE LESS THAN 1!" << x << y << x_dot << y_dot;
                    int_verts.push_back(int_pt);
                    int_normals.push_back(m.column(2).toVector3D().normalized());
                    int_texcoords.push_back(QVector2D(x_dot, y_dot));
                }
            }
        }
        break;

    default:
        break;
    }

    return !(int_verts.isEmpty());
}

void RoomObject::SetAttributeModelMatrix(const QMatrix4x4 & m)
{
    props->SetXDir(m.column(0).toVector3D().normalized());
    props->SetYDir(m.column(1).toVector3D().normalized());
    props->SetZDir(m.column(2).toVector3D().normalized());
    props->SetPos(m.column(3).toVector3D());
    props->SetScale(QVector3D(m.column(0).toVector3D().length(),
                              m.column(1).toVector3D().length(),
                              m.column(2).toVector3D().length()));
}

QMatrix4x4 RoomObject::GetAttributeModelMatrix() const
{
    const QVector3D s = props->GetScale()->toQVector3D();
    QMatrix4x4 m;
    m.setColumn(0, props->GetXDir()->toQVector3D() * s.x());
    m.setColumn(1, props->GetYDir()->toQVector3D() * s.y());
    m.setColumn(2, props->GetZDir()->toQVector3D() * s.z());
    m.setColumn(3, props->GetPos()->toQVector3D());
    m.setRow(3, QVector4D(0,0,0,1));
    return m;
}

void RoomObject::SetAttributeModelMatrixNoScaling(const QMatrix4x4 & m)
{
    props->SetXDir(m.column(0).toVector3D().normalized());
    props->SetYDir(m.column(1).toVector3D().normalized());
    props->SetZDir(m.column(2).toVector3D().normalized());
    props->SetPos(m.column(3).toVector3D());
}

QMatrix4x4 RoomObject::GetAttributeModelMatrixNoScaling() const
{
    QMatrix4x4 m;
    m.setColumn(0, props->GetXDir()->toQVector3D());
    m.setColumn(1, props->GetYDir()->toQVector3D());
    m.setColumn(2, props->GetZDir()->toQVector3D());
    m.setColumn(3, props->GetPos()->toQVector3D()) ;
    m.setRow(3, QVector4D(0,0,0,1));
    return m;
}

QVector3D RoomObject::GetPos() const
{
    QVector3D pos = props->GetPos()->toQVector3D();
    if (props->GetInterpolate() && interp_val < 1.0f) {
        return interp_pos * (1.0f - interp_val) + pos * interp_val;
    }
    else {
        return pos;
    }
}

QVector3D RoomObject::GetVel() const
{
    return props->GetVel()->toQVector3D();
}

QVector3D RoomObject::GetAccel() const
{
    return props->GetAccel()->toQVector3D();
}

void RoomObject::SetDir(const QVector3D & p)
{
    const QVector3D z = p.normalized();
    const QVector3D x = QVector3D::crossProduct(QVector3D(0, 1, 0), z).normalized();
    const QVector3D y = QVector3D::crossProduct(z, x).normalized();

    props->SetXDir(x);
    props->SetYDir(y);
    props->SetZDir(z);
}

QVector3D RoomObject::GetDir() const
{
    return props->GetZDir()->toQVector3D();
}

void RoomObject::SetSaveToMarkup(const bool b)
{
    save_to_markup = b;
}

bool RoomObject::GetSaveToMarkup() const
{
    return save_to_markup;
}

void RoomObject::SnapXDirsToMajorAxis()
{
    QVector3D x[3];
    x[0] = props->GetXDir()->toQVector3D();
    x[1] = props->GetYDir()->toQVector3D();
    x[2] = props->GetZDir()->toQVector3D();

    QList <QVector3D> a;
    a.push_back(QVector3D(1,0,0));
    a.push_back(QVector3D(-1,0,0));
    a.push_back(QVector3D(0,1,0));
    a.push_back(QVector3D(0,-1,0));
    a.push_back(QVector3D(0,0,1));
    a.push_back(QVector3D(0,0,-1));

    //get closest major axes for x and y
    for (int j=0; j<2; ++j) {
        int best_index = 0;
        float best_prod = -FLT_MAX;
        for (int i=0; i<a.size(); ++i) {
            const float each_prod = QVector3D::dotProduct(x[j], a[i]);
            if (each_prod > best_prod) {
                best_prod = each_prod;
                best_index = i;
            }
        }
        x[j] = a[best_index];
        a.removeAt(best_index);
    }

    x[2] = QVector3D::crossProduct(x[0], x[1]);

    props->SetXDir(x[0]);
    props->SetYDir(x[1]);
    props->SetZDir(x[2]);
}

void RoomObject::Update(const double dt_sec)
{
    const ElementType obj_type = GetType();

    if (assetimage) {
        assetimage->UpdateGL();
    }
    for (QPointer <AssetObject> a : ghost_assetobjs) {
        if (a) {
            a->Update();
            a->UpdateGL();
        }
    }

    if (obj_type != TYPE_GHOST || playing) {
        time_elapsed += dt_sec;
    }

    if (!props->GetInterpolate()) {
        interp_pos = props->GetPos()->toQVector3D();
        interp_xdir = props->GetXDir()->toQVector3D();
        interp_ydir = props->GetYDir()->toQVector3D();
        interp_zdir = props->GetZDir()->toQVector3D();
        interp_scale = props->GetScale()->toQVector3D();
    }

    UpdateMatrices();   

    if (obj_type == TYPE_PARTICLE) {
        if (assetobject_emitter) {
            particle_system->SetEmitterMesh(assetobject_emitter);
        }
        else {
            particle_system->SetEmitterMesh(NULL);
        }

        particle_system->Update(props, dt_sec);
    }

    //update child objects
    for (int i=0; i<child_objects.size(); ++i) {
        if (child_objects[i]) {
            child_objects[i]->model_matrix_parent = model_matrix_parent * model_matrix_local;
            child_objects[i]->Update(dt_sec);
        }
    }

    //rescale objects (generally used for drag and drop where scale can be crazy)
    if (rescale_on_load && !rescale_on_load_done && assetobject && assetobject->GetFinished()) {
        QVector3D bmin, bmax;
        QPointer <AssetObject> a = assetobject;
        bmin = a->GetBBoxMin();
        bmax = a->GetBBoxMax();

        const float d  = 1.0f / (bmax - bmin).length();
        props->SetScale(QVector3D(d,d,d));

        props->SetCollisionPos((bmin+bmax)*0.5f);
        props->SetCollisionScale(bmax - bmin);
        //SetCollisionStatic(false);

        rescale_on_load_done = true;
    }

    if (rescale_on_load_aspect && !rescale_on_load_done && assetobject && assetobject->GetFinished()) {
        float aspect_ratio = 1.0;
        if (assetimage && assetimage->GetFinished()){
            aspect_ratio = assetimage->GetAspectRatio();
            props->SetScale(QVector3D(1.0f,aspect_ratio,0.1f));
            rescale_on_load_done = true;
        }
        else if (assetvideo){
            aspect_ratio = assetvideo->GetAspectRatio(&media_ctx);
            props->SetScale(QVector3D(1.0f,aspect_ratio,0.1f));
            rescale_on_load_done = true;
        }
    }

    //update the object's current_time and total_time members for sound/video playback for JS reads
    if (assetsound) {
        props->SetCurTime(assetsound->GetCurTime(&media_ctx));
        props->SetTotalTime(assetsound->GetTotalTime(&media_ctx));
    }
    else if (assetvideo) {
        props->SetCurTime(assetvideo->GetCurTime(&media_ctx));
        props->SetTotalTime(assetvideo->GetTotalTime(&media_ctx));
    }

    if (assetsound && !media_ctx.setup) {
        assetsound->SetupOutput(&media_ctx, props->GetLoop());
    }

    if (assetvideo && !media_ctx.setup) {
        assetvideo->SetupOutput(&media_ctx);
    }

    // HACKS: Trigger side-effects on setters.
    // will be fixed when logic is ported over to properties.
    if (obj_type == TYPE_TEXT && textgeom) { //60.0 - bugfix update text changed by JS
        if (props->GetText() != textgeom->GetText()) {
            props->SetTextChanged(true);
        }
    }
    if ((obj_type == TYPE_TEXT || obj_type == TYPE_PARAGRAPH) && props->GetTextChanged()) {
        SetText(props->GetText());
        props->SetTextChanged(false);
    }

    if (props->GetInterpolate()  && interp_time > 0.0f && obj_type != TYPE_LINK && obj_type != TYPE_SOUND) {
        interp_val = float(interp_timer.elapsed()) / (interp_time * 1000.0f);
        interp_val = qMax(0.0f, qMin(interp_val, 1.0f));
    }
    else {
        interp_val = 1.0f;
    }

    switch (obj_type) {
    case TYPE_GHOST:
    {
        const QString id = props->GetID();
        if (id != textgeom_player_id->GetText()) {
            textgeom_player_id->SetText(id);
        }

        //update active animation for avatar
        const QString body_id = props->GetBodyID();
        QPointer <AssetObject> assetobj = ghost_assetobjs[body_id];
        if (assetobj) {
            QPointer <Geom> body = assetobj->GetGeom();
            const QString anim_id = props->GetAnimID();

            if (body && body->GetReady() &&
                    ghost_assetobjs.contains(anim_id) &&
                    ghost_assetobjs[anim_id] &&
                    ghost_assetobjs[anim_id]->GetFinished() &&
                    ghost_assetobjs[anim_id]->GetGeom()->GetReady()) {
                body->SetLinkAnimation(ghost_assetobjs[anim_id]->GetGeom());
            }

            if (assetobj->GetGeom()) {
                assetobj->GetGeom()->SetAnimSpeed(props->GetAnimSpeed());
                assetobj->GetGeom()->SetLoop(true);
            }
        }

        QPointer <AssetGhost> g = assetghost;
        if (g && g->GetProcessed()) { //56.0 - ensure ghost data is processed before doing this

            ghost_frame = GhostFrame();

            const int cur_ghost_frame_index = g->GetFrameIndex(time_elapsed);
            const bool sequence_end = g->GetGhostFrame(time_elapsed, ghost_frame);

            g->ClearEditsDeletes();

            //54.8 - auto start/stop only for non-multiplayer avatars
            if (playing && sequence_end && !do_multiplayer_timeout) {
                if (props->GetLoop()) {
                    time_elapsed = 0.0f;
                }
                else {
                    playing = false;
                }
            }

            if (do_multiplayer_timeout) {
                if (SettingsManager::GetUpdateVOIP()) {
                    for (int i=0; i<g->GetNumFrames(); ++i) {
                        GhostFrame & f2 = g->GetFrameByIndex(i);
                        if (!f2.sound_buffers.isEmpty()) {
                            //qDebug() << "Room::Update voip1" << this->GetS("id") << f2.sound_buffers.size();
                            sound_buffers += f2.sound_buffers;
                            f2.sound_buffers.clear();
                            //59.0 - commented out intentionally, play VOIP packets even if player is in another room
                            if (!player_in_room && !player_in_adjacent_room) {
                                sound_buffers.clear();
                            }
                        }
                    }
                }
            }

            //do all the things we want to do only ONCE for each new frame
            if (cur_ghost_frame_index != ghost_frame_index) {
                if (SettingsManager::GetUpdateVOIP()) {
                    if (cur_ghost_frame_index+1 < g->GetNumFrames()) {
                        GhostFrame & f2 = g->GetFrameByIndex(cur_ghost_frame_index+1);
                        if (!f2.sound_buffers.isEmpty()) {
                            //qDebug() << "Room::Update voip2" << this->GetS("id")<< f2.sound_buffers.size();
                            sound_buffers += f2.sound_buffers;
                            //59.0 - commented out intentionally, play VOIP packets even if player is in another room
                            if (!player_in_room && !player_in_adjacent_room) {
                                sound_buffers.clear();
                            }
                        }
                    }
                }
                //qDebug() << "Got sound buffers" << sound_buffers.size();

                //make the ghost chat
                SetChatMessage(ghost_frame.chat_message);

                //change the ghost's avatar
                //if (SettingsManager::GetUpdateCustomAvatars()) {
                    if (!ghost_frame.avatar_data.isEmpty()
                            && QString::compare(ghost_frame.avatar_data.left(13), "<FireBoxRoom>") == 0
                            && QString::compare(GetAvatarCode(), ghost_frame.avatar_data) != 0) {
                        const QString id = props->GetID();
                        const QString js_id = props->GetJSID();
                        const bool loop = props->GetLoop();
                        //qDebug() << this << "setting data:" << ghost_frame.avatar_data;
                        LoadGhost(ghost_frame.avatar_data);
                        SetAvatarCode(ghost_frame.avatar_data);

                        //49.50 - Do not let id, js_id, or loop attributes be overwritten
                        props->SetID(id);
                        props->SetJSID(js_id);
                        props->SetLoop(loop);
                    }
                //}

                //room edits/deletes
                if (!ghost_frame.room_edits.isEmpty()) {
                    RoomObjectEdit e;
                    e.data = ghost_frame.room_edits;
                    e.room = ghost_frame.roomid;
                    e.user = ghost_frame.userid;
                    room_edits_incoming += e;
                }

                if (!ghost_frame.room_deletes.isEmpty()) {
                    RoomObjectEdit e;
                    e.data = ghost_frame.room_deletes;
                    e.room = ghost_frame.roomid;
                    e.user = ghost_frame.userid;
                    room_deletes_incoming += e;
                }

                //send portals
                if (ghost_frame.send_portal_url.length() > 0) {
                    send_portal_url += ghost_frame.send_portal_url;
                    send_portal_jsid += ghost_frame.send_portal_jsid;
                    send_portal_pos += ghost_frame.send_portal_pos;
                    send_portal_fwd += ghost_frame.send_portal_fwd;
                }

                ghost_frame_index = cur_ghost_frame_index;
            }

            //change the ghost's animation and other attributes
            props->SetAnimID(ghost_frame.anim_id);           
            props->SetPos(ghost_frame.pos);
            SetDir(ghost_frame.dir);
            SetHMDType(ghost_frame.hmd_type);
        }

        //modify the transformation of the neck joint based on HMD transform
        QPointer <AssetObject> ghost_body = 0;
        if (ghost_assetobjs.contains(body_id)) {
            ghost_body = ghost_assetobjs[body_id];
        }
        if (ghost_body &&
                ghost_body->GetFinished() &&
                ghost_body->GetGeom() &&
                ghost_body->GetGeom()->GetReady()) {
            const QPointer <Geom> body = ghost_body->GetGeom();
            body->ClearTransforms();

            //65.2 - we use likely names for a head or neck joint and apply rotation based on user's view/up directions
            const QStringList head_bone_names = {"Head", "head", "Neck", "neck"};
            for (int i=0; i<head_bone_names.size(); ++i) {
                if (body->HasBone(head_bone_names[i])) {
                    body->SetRelativeTransform(head_bone_names[i], ghost_frame.head_xform);
                    break;
                }
            }
        }
    /*
            const QVector3D s = GetScale();
            const QVector3D zdir = GetZDir();

            const float angle = -(90.0f - atan2f(zdir.z(), zdir.x()) * MathUtil::_180_OVER_PI);

            QMatrix4x4 m0;
            m0.rotate(angle, 0, 1, 0);
            m0.scale(s.x(), s.y(), s.z());
            m0 *= ghost_frame.head_xform;

            m0 = ghost_frame.head_xform;
            m0.rotate(angle, 0, 1, 0);
            m0.scale(1.0f / s.x(), 1.0f / s.y(), 1.0f / s.z());
            m0.setColumn(3, QVector4D(m0.column(3).x() / s.x(),
                         m0.column(3).y() / s.y(),
                         m0.column(3).z() / s.z(),
                         1));

            qDebug() << "xform" << m0;
            m0.translate(0,5,0);
            m0.scale(1.0f / s.x(), 1.0f / s.y(), 1.0f / s.z());
            const QVector3D head_pos = ghost_frame.head_xform.column(3).toVector3D();

            if (QString::compare(hmd_type, QStringLiteral("vive")) == 0) {
                qDebug() << "global xform" << m0.column(3);
                body_fbx->SetGlobalTransform("Head", m0);

                QMatrix4x4 m1;
                m1.setToIdentity();

                m1.rotate(angle, 0, 1, 0);
                m1.translate(QVector3D(head_pos.x() / s.x(),  (head_pos.y() - 0.55f) / s.y(), head_pos.z() / s.z()));

                const float head_angle = (90.0f - atan2f(ghost_frame.head_xform.column(2).z(), ghost_frame.head_xform.column(2).x()) * MathUtil::_180_OVER_PI);
                m1.rotate(head_angle, 0, 1, 0);
                body_fbx->SetGlobalTransform("Hips", m1);
            }
            else {
                body_fbx->SetRelativeTransform("Head", m0);
            }        

            QList <QString> finger_names;
            finger_names.push_back("Thumb");
            finger_names.push_back("Index");
            finger_names.push_back("Middle");
            finger_names.push_back("Ring");
            finger_names.push_back("Pinky");

            for (int i=0; i<2; ++i) {
                LeapHand & hand = ((i == 0) ? ghost_frame.hands.first : ghost_frame.hands.second);
                if (hand.is_active) {

                    QMatrix4x4 head_basis;

                    if ((QString::compare(hmd_type, QStringLiteral("vive")) == 0 ||
                            QString::compare(hmd_type, QStringLiteral("rift")) == 0)
                            && !hand.finger_tracking) { //55.2 - override if using Leap Motion (i.e. finger tracking)
                        head_basis = m0;
                        //use identity matrix for head basis
                        //for some reason finger tracking can be true??
                    }
                    else {
                        if (!hand.finger_tracking) {
                            head_basis.setColumn(3, QVector4D(eye_pos, 1));
                        }
                        else {
                            head_basis = ghost_frame.head_xform;
                            head_basis.setColumn(3, QVector4D(head_pos + eye_pos, 1));
                            head_basis.rotate(angle, 0, 1, 0);
                        }

                        //56.0 - commented all this arm stuff out until it can be given a proper pass.... bugs out vive and rift with gamepad or non-tracked controllers
                        if (hand.finger_tracking) {

                            head_basis = QMatrix4x4();
                            if (hand.finger_tracking_leap_hmd) {
                                head_basis.setColumn(3, QVector4D(eye_pos, 1));
                            }
                            else {
                                head_basis.setColumn(3, QVector4D(head_pos + eye_pos, 1));
                            }

                            const QString prefix = ((i == 0) ? "Left" : "Right");

                            QVector3D b0, b1, b2;
                            b0 = body_fbx->GetGlobalBindPose(prefix+"Arm").column(3).toVector3D();
                            b1 = body_fbx->GetGlobalBindPose(prefix+"ForeArm").column(3).toVector3D();
                            b2 = body_fbx->GetGlobalBindPose(prefix+"Hand").column(3).toVector3D();

                            QVector3D p0, p2;
                            p0 = body_fbx->GetFinalPose(prefix+"Arm").column(3).toVector3D();
                            p2 = head_basis * hand.basis.column(3).toVector3D();
                            p2.setX(p2.x() / s.x());
                            p2.setY(p2.y() / s.y());
                            p2.setZ(p2.z() / s.z());

                            const float l0 = (b1-b0).lengthSquared();
                            const float l1 = (b2-b1).lengthSquared();

                            //set for forearm
                            double a0 = (1.0 - ((l0-l1)) / (p2-p0).lengthSquared()) * 0.5;
                            QVector3D c0 = p0 * a0 + p2 * (1.0 - a0); //circle centre point
                            const float r = sqrtf(l0 - (c0-p0).lengthSquared()); //circle radius of intersecting spheres on p0,p2 with rads l0,l1

                            QVector3D p1;
                            if (sqrtf(l0) + sqrtf(l1) < (p2-p0).length()) { //in case hands are further than the arms' reaech
                                p1 = (p0 + p2) * 0.5;
                            }
                            else {
                                p1 = c0 - MathUtil::GetOrthoVec((p2-p0).normalized()) * r;
                            }

                            //setup arm joint
                            QVector3D m00 = (p1 - p0).normalized();
                            QVector3D m01 = MathUtil::GetOrthoVec(m00);
                            QVector3D m02 = QVector3D::crossProduct(m00, m01).normalized();
                            QVector3D m03 = p0;

                            if (i == 1) {
                                m00 *= -1.0f;
                                m02 *= -1.0f;
                            }

                            QMatrix4x4 m0;
                            m0.setColumn(0, m00);
                            m0.setColumn(1, m01);
                            m0.setColumn(2, m02);
                            m0.setColumn(3, m03);
                            m0.setRow(3, QVector4D(0,0,0,1));

                            body_fbx->SetGlobalTransform(prefix + "Arm", m0);

                            //setup forearm joint
                            QVector3D m10 = (p2 - p1).normalized();
                            QVector3D m11 = MathUtil::GetOrthoVec(m10);
                            QVector3D m12 = QVector3D::crossProduct(m10, m11).normalized();
                            QVector3D m13 = p1;

                            if (i == 1) {
                                m10 *= -1.0f;
                                m11 *= -1.0f;
                            }

                            QMatrix4x4 m1;
                            m1.setColumn(0, m10);
                            m1.setColumn(1, m11);
                            m1.setColumn(2, m12);
                            m1.setColumn(3, m13);
                            m1.setRow(3, QVector4D(0,0,0,1));

                            body_fbx->SetGlobalTransform(prefix + "ForeArm", m1);

                            //set for hand
                            QMatrix4x4 m = head_basis * hand.basis;
                            m.data()[12] /= s.x();
                            m.data()[13] /= s.y();
                            m.data()[14] /= s.z();
                            body_fbx->SetGlobalTransform(prefix + "Hand", m);

                            //do fingers only if it's leap motion
                            for (int j=0; j<finger_names.size(); ++j) {
                                for (int k=0; k<4; ++k) {
                                    QMatrix4x4 m = head_basis * hand.fingers[j][k];
                                    m.data()[12] /= s.x();
                                    m.data()[13] /= s.y();
                                    m.data()[14] /= s.z();
                                    body_fbx->SetGlobalTransform(prefix+"Hand"+finger_names[j]+QString::number(k), m);
                                }
                            }
                        }
                    }
                }
            }
        }
    */
    }
        break;

    case TYPE_OBJECT:
        if (assetobject && assetobject_anim) {
            QPointer <Geom> body = assetobject->GetGeom();
            QPointer <Geom> body_anim = assetobject_anim->GetGeom();
            if (body && body->GetReady() && body_anim && body_anim->GetReady()) {
                body->SetLinkAnimation(body_anim);
            }
        }

        if (assetobject && assetobject->GetGeom()) {
            assetobject->GetGeom()->SetAnimSpeed(props->GetAnimSpeed());
            assetobject->GetGeom()->SetLoop(props->GetLoop());
        }
        break;

    case TYPE_LINK:
        if (props->GetDrawText() && (GetURL() != portal_last_url || GetTitle() != portal_last_title)) {
            //qDebug() << "Portal::DrawDecorationsGL() updating text" << object->GetURL() << last_url;
            if (portal_text) {
                portal_text->SetText(QString("<p align=\"center\">") + GetTitle() + QString("<br><font color=\"#0000ff\">") + GetURL() + QString("</font></p>"), false);
            }
            portal_last_url = GetURL();
            portal_last_title = GetTitle();
        }
        break;
    default:
        break;

    }

    UpdateMedia();
}

void RoomObject::UpdateMedia()
{
    //are we playing a streaming sound?  generate a source
    if (!sound_buffers.isEmpty() && openal_stream_source == 0) {
        alGenSources(1, &openal_stream_source);
    }

    //const ElementType obj_type = GetType();

    const bool positional_env = SettingsManager::GetPositionalEnvEnabled();
    const bool positional_voip = SettingsManager::GetPositionalVOIPEnabled();
    const float gain_env = SettingsManager::GetVolumeEnv()/100.0f;
    //const float gain_voip = SettingsManager::GetVolumeVOIP()/100.0f;

    if (openal_stream_source > 0) {
        const QVector3D pos = GetPos() + QVector3D(0, 1.6f, 0);
        const QVector3D vel = GetVel();
        const QVector3D dir = GetZDir();
        const QVector3D scale = GetScale();
        //const float dist = qMax(scale.x(), qMax(scale.y(), scale.z()));

        if (positional_voip) {
            alSourcei(openal_stream_source, AL_SOURCE_RELATIVE, AL_FALSE);
            alSource3f(openal_stream_source, AL_POSITION, pos.x(), pos.y(), pos.z()); //set source position
            alSource3f(openal_stream_source, AL_VELOCITY, vel.x(), vel.y(), vel.z()); //set source velocity
        }
        else{
            alSourcei(openal_stream_source, AL_SOURCE_RELATIVE, AL_TRUE);
            alSource3f(openal_stream_source, AL_POSITION, 0, 0, 0); //set source position
            alSource3f(openal_stream_source, AL_VELOCITY, 0, 0, 0); //set source velocity
        }

        alSourcei(openal_stream_source, AL_LOOPING, props->GetLoop() ? AL_TRUE : AL_FALSE);
        //60.0 - commented these out as they are problematic for VOIP
        //alSourcef(openal_stream_source, AL_PITCH, GetF("pitch"));
        //alSourcef(openal_stream_source, AL_GAIN, 2.0f * gain_voip * GetF("gain"));
        //alSourcef(openal_stream_source, AL_ROLLOFF_FACTOR, 3.0f);
        //alSourcef(openal_stream_source, AL_REFERENCE_DISTANCE, dist);

        if (dir != QVector3D(0,0,1) && positional_voip) {
            alSource3f(openal_stream_source, AL_DIRECTION, dir.x(), dir.y(), dir.z());
            alSourcef(openal_stream_source, AL_CONE_INNER_ANGLE, props->GetInnerAngle());
            alSourcef(openal_stream_source, AL_CONE_OUTER_ANGLE, props->GetOuterAngle());
            alSourcef(openal_stream_source, AL_CONE_OUTER_GAIN, props->GetOuterGain());
        }
        else if (dir != QVector3D(0,0,1) && !positional_voip){
            alSource3f(openal_stream_source, AL_DIRECTION, 0, 0, 0);
            alSourcef(openal_stream_source, AL_CONE_INNER_ANGLE, 360);
            alSourcef(openal_stream_source, AL_CONE_OUTER_ANGLE, 360);
            alSourcef(openal_stream_source, AL_CONE_OUTER_GAIN, 0);
        }

        // Poll for recoverable buffers
        ALint availBuffers=0; // Buffers to be recovered
        alGetSourcei(openal_stream_source, AL_BUFFERS_PROCESSED, &availBuffers);
        //qDebug() << availBuffers;
        if (availBuffers > 0) {
            QVector<ALuint> buffHolder;
            //buffHolder.reserve(SoundManager::buffer_input_pool.size()); // An array to hold catch the unqueued buffers
            buffHolder.resize(availBuffers); //49.50 crash with audio on Windows
            alSourceUnqueueBuffers(openal_stream_source, availBuffers, buffHolder.data());
            for (int i=0;i<availBuffers;++i) {
                // Push the recovered buffers back on the queue
                SoundManager::buffer_input_queue.push_back(buffHolder[i]);
            }
        }

        if (!sound_buffers.isEmpty()) {

            if (!SoundManager::buffer_input_queue.isEmpty()) { // We just drop the data if no buffers are available

                ALuint myBuff = SoundManager::buffer_input_queue.front();

                SoundManager::buffer_input_queue.pop_front();
                //49.8 bugfix - use the buffer size (which can vary), not a fixed size causing portions of audio not to play
                QByteArray b = AudioUtil::decode(QByteArray::fromBase64(sound_buffers.front()));
                //ghost_frame.current_sound_level = MathUtil::GetSoundLevel(b);

                alBufferData(myBuff, AL_FORMAT_MONO16, b.data(), b.size(), sound_buffers_sample_rate);

                // Queue the buffer
                alSourceQueueBuffers(openal_stream_source, 1, &myBuff);

                // Restart the source if needed
                ALint sState=0;
                alGetSourcei(openal_stream_source, AL_SOURCE_STATE, &sState);
                if (sState != AL_PLAYING) {
                    alSourcePlay(openal_stream_source);
                    //qDebug() << "RoomObject::UpdateMedia() playing" << this->GetS("id") << openal_stream_source;
                }

                //qDebug() << "RoomObject::UpdateMedia() buffer input queue" << this->GetS("id") << sound_buffers.size() << SoundManager::buffer_input_queue.size();

                sound_buffers.pop_front();
            }
        }
        else {
            //If no more buffers in queue, delete source
            ALint queued_buffers = 0;
            alGetSourcei(openal_stream_source, AL_BUFFERS_QUEUED, &queued_buffers);
            //qDebug() << "queued_buffers" << queued_buffers;
            if (queued_buffers == 0)
            {
                //Delete source if not in use
                //qDebug() << "deleting source";
                alSourcei(openal_stream_source, AL_BUFFER, 0);
                alDeleteSources(1, &openal_stream_source);
                openal_stream_source = 0;
            }
        }
    }


    //play a sound if we need to
    if (assetsound) {
        //Update gain, pos, vel, dist, pitch, positional sound
        media_ctx.audio_lock.lock();
        media_ctx.pos = model_matrix_parent.map(GetPos());
        media_ctx.vel = GetVel();
        const QVector3D scale = GetScale();
        media_ctx.dist = qMax(scale.x(), qMax(scale.y(), scale.z()));
        media_ctx.gain = gain_env * props->GetGain();
        media_ctx.doppler_factor = props->GetDopplerFactor();
        media_ctx.pitch = props->GetPitch();
        media_ctx.positional_sound = (positional_env && media_ctx.pos != QVector3D(0,0,0));
        media_ctx.audio_lock.unlock();
    }

    //volume adjustment for videos
    if (assetvideo) {
        //Update gain, pos, vel, dist, pitch, positional sound
        media_ctx.audio_lock.lock();
        media_ctx.pos = model_matrix_parent.map(GetPos());
        media_ctx.vel = GetVel();
        const QVector3D scale = GetScale();
        media_ctx.dist = qMax(scale.x(), qMax(scale.y(), scale.z()));
        media_ctx.gain = gain_env * props->GetGain();
        media_ctx.doppler_factor = props->GetDopplerFactor();
        media_ctx.pitch = props->GetPitch();
        media_ctx.positional_sound = (positional_env && media_ctx.pos != QVector3D(0,0,0));
        media_ctx.audio_lock.unlock();
    }
}

qsreal RoomObject::NextUUID()
{
    return DOMNode::NextUUID();
}

void RoomObject::SetCubemapRadiance(const QPointer <AssetImage> a)
{
    m_cubemap_radiance = a;
}

QPointer <AssetImage> RoomObject::GetCubemapRadiance() const
{
    return m_cubemap_radiance;
}

void RoomObject::SetCubemapIrradiance(const QPointer <AssetImage> a)
{
   m_cubemap_irradiance = a;
}

QPointer <AssetImage> RoomObject::GetCubemapIrradiance() const
{
    return m_cubemap_irradiance;
}

QVector3D RoomObject::GetBBoxMin()
{    
    if (assetobject) {
        return assetobject->GetBBoxMin();
    }
    return QVector3D();
}

QVector3D RoomObject::GetBBoxMax()
{
    if (assetobject) {
        return assetobject->GetBBoxMax();
    }
    return QVector3D();
}

void RoomObject::SetUserIDPos(const QVector3D & p)
{
    userid_pos = p;
}

QVector3D RoomObject::GetUserIDPos() const
{
    return userid_pos;
}

void RoomObject::SetHMDType(const QString & s)
{
    hmd_type = s;
}

QString RoomObject::GetHMDType() const
{
    return hmd_type;
}

void RoomObject::SetHeadAvatarPos(const QVector3D & p)
{
    head_avatar_pos = p;
}

QVector3D RoomObject::GetHeadAvatarPos() const
{
    return head_avatar_pos;
}

void RoomObject::SetGhostAssetShaders(const QHash <QString, QPointer <AssetShader> > & a)
{
    ghost_asset_shaders = a;
}

QHash <QString, QPointer <AssetShader> > RoomObject::GetGhostAssetShaders()
{
    return ghost_asset_shaders;
}

void RoomObject::SetGhostAssetObjects(const QHash <QString, QPointer <AssetObject> > & a)
{
    //qDebug() << "RoomObject::SetGhostAssetObjects()" << a;
    ghost_assetobjs = a;
}

QHash <QString, QPointer <AssetObject> > RoomObject::GetGhostAssetObjects()
{
    //59.3 - bugfix for child object AssetObjects appearing in userid.txt file
    //Note!  This function takes the ghost_assetobjects member at root, but only the assetobject of child_objects
    QHash <QString, QPointer <AssetObject> > gas = ghost_assetobjs;
    QList <QPointer <RoomObject> > co = child_objects;

    while (!co.isEmpty()) {
        QPointer <RoomObject> o = co.first();
        co.pop_front();

        const QPointer <AssetObject> a = o->GetAssetObject();
        if (a) {
            gas[a->GetProperties()->GetID()] = a;
        }
        //add this object's children to list to iterate over
        co += o->GetChildObjects();
    }

    return gas;
}

void RoomObject::SetChildObjects(QList <QPointer <RoomObject> > & e)
{
    child_objects = e;
}

QList <QPointer <RoomObject> > & RoomObject::GetChildObjects()
{
    return child_objects;
}

void RoomObject::SetAvatarCode(const QString & s)
{
    avatar_code = s;
}

QString RoomObject::GetAvatarCode() const
{
    return avatar_code;
}

QMatrix4x4 RoomObject::GetRotationMatrix() const
{
    QMatrix4x4 m;
    m.setColumn(0, GetXDir());
    m.setColumn(1, GetYDir());
    m.setColumn(2, GetZDir());
    return m;
}

void RoomObject::SetSelected(const bool b)
{
    selected = b;

    for (int i=0; i<child_objects.size(); ++i) {
        if (child_objects[i]) {
            child_objects[i]->SetSelected(b);
        }
    }
}

bool RoomObject::GetSelected() const
{
    return selected;
}

void RoomObject::SetCollisionSet(const QSet <QString> s)
{
    collision_set = s;
}

QSet <QString> RoomObject::GetCollisionSet() const
{
    return collision_set;
}

void RoomObject::SetPlaySounds(const bool b)
{
    play_sounds = b;
}

bool RoomObject::GetPlaySounds() const
{
    return play_sounds;
}

void RoomObject::SetAssetGhost(const QPointer <AssetGhost> a)
{
    assetghost = a;
}

QPointer<AssetGhost> RoomObject::GetAssetGhost()
{
    return assetghost;
}

void RoomObject::SetDoMultiplayerTimeout(const bool b)
{
    do_multiplayer_timeout = b;
}

void RoomObject::SetChatMessage(const QString & s)
{
    if (s.isEmpty()) {
        return;
    }

    QStringList message_list = s.split(" ");
    QList <QString> messages;
    QString each_message;

    //break apart long sentence into smaller pieces
    for (int i=0; i<message_list.size(); ++i) {

        const QString & eachword = message_list[i];

        each_message += eachword + QString(" ");
        if (each_message.length() > 25) {
            messages.push_back(each_message);
            each_message = QString("");
        }
    }
    messages.push_back(each_message);

    //add this message if it's new
    if (textgeom_chatmessages.empty() || QString::compare(textgeom_chatmessages.last()->GetText(), messages.last()) != 0) {

        //add the messages
        for (int i=0; i<messages.size(); ++i) {

            chat_message = s;
            chat_message_time_offsets.push_back(-float(i)*2.0f);

            QTime new_time;
            new_time.start();
            chat_message_times.push_back(new_time);

            QPointer <TextGeom> new_textgeom = new TextGeom();
            new_textgeom->SetLeftJustify(false);
            new_textgeom->SetFixedSize(true, 0.075f);
            new_textgeom->SetText(messages[i], QColor(255,255,255));
            textgeom_chatmessages.push_back(new_textgeom);
        }

        //play a pop sound
        if (play_sounds) {
            SoundManager::Play(SOUND_POP1, false, GetPos()+QVector3D(0,1,0), 1.0f);
        }
    }
}

QString RoomObject::GetChatMessage() const
{
    return chat_message;
}

void RoomObject::SetAssetImage(const QPointer <AssetImage> a)
{
    assetimage = a;
}

QPointer <AssetImage> RoomObject::GetAssetImage()
{
    return assetimage;
}

void RoomObject::SetAssetLightmap(const QPointer <AssetImage> a)
{
    assetimage_lmap = a;
}

QPointer <AssetImage> RoomObject::GetAssetLightmap()
{
    return assetimage_lmap;
}

void RoomObject::SetAssetObject(const QPointer <AssetObject> a)
{
    //qDebug() << "RoomObject::SetAssetObject" << this << a;
    assetobject = a;
}

QPointer <AssetObject> RoomObject::GetAssetObject()
{
    return assetobject;
}

void RoomObject::SetAnimAssetObject(const QPointer <AssetObject> a)
{
    assetobject_anim = a;
}

QPointer <AssetObject> RoomObject::GetAnimAssetObject() const
{
    return assetobject_anim;
}

QPointer<AssetSound> RoomObject::GetAssetSound() const
{
    return assetsound;
}

void RoomObject::SetBlendAssetObject(const int i, const QPointer <AssetObject> a)
{
    //qDebug() << "EnvObject::SetBlend0AssetObject()!" << a->GetID();
    if (i >= 0 && i < 4) {        
        assetobject_blendshapes[i] = a;
        //QPointer <AssetObject> o = assetobject;
        //QPointer <AssetObject> b = assetobject_blendshapes[i];
        //if (o && o->GetGeomOBJ() && b && b->GetGeomOBJ()) {
        //    o->GetGeomOBJ()->SetBlendShape(i, b->GetGeomOBJ());
        //}
    }
}

QPointer <AssetObject> RoomObject::GetBlendAssetObject(const int i)
{
    return assetobject_blendshapes[i];
}

void RoomObject::SetCollisionAssetObject(const QPointer <AssetObject> a)
{
    assetobject_collision = a;
}

QPointer <AssetObject> RoomObject::GetCollisionAssetObject()
{
    return assetobject_collision;
}

void RoomObject::SetEmitterAssetObject(const QPointer <AssetObject> a)
{
    assetobject_emitter = a;
}

QPointer <AssetObject> RoomObject::GetEmitterAssetObject()
{
    return assetobject_emitter;
}

void RoomObject::SetTeleportAssetObject(const QPointer <AssetObject> a)
{
    assetobject_teleport = a;
}

QPointer <AssetObject> RoomObject::GetTeleportAssetObject()
{
    return assetobject_teleport;
}

void RoomObject::SetInterpolation()
{
    interp_pos = props->GetPos()->toQVector3D();
    interp_xdir = props->GetXDir()->toQVector3D();
    interp_ydir = props->GetYDir()->toQVector3D();
    interp_zdir = props->GetZDir()->toQVector3D();
    interp_scale = props->GetScale()->toQVector3D();

    interp_val = 0.0f;
    interp_timer.restart();

    UpdateMatrices();
}

void RoomObject::Copy(const QPointer <RoomObject> o)
{
    if (o.isNull()) {
        return;
    }

    selected = o->selected;
    tex_width = o->tex_width;
    tex_height = o->tex_height;

    //image
    assetimage = o->assetimage;

    //sound
    assetsound = o->assetsound;    

    //websurface
    assetwebsurface = o->assetwebsurface;

    //video
    assetvideo = o->assetvideo;

    //object
    assetobject = o->assetobject;
    assetobject_collision = o->assetobject_collision;
    assetobject_anim = o->assetobject_anim;
    assetobject_teleport = o->assetobject_teleport;
    assetobject_blendshapes = o->assetobject_blendshapes;
    assetobject_emitter = o->assetobject_emitter;

    assetshader = o->assetshader; //optional shader

    //ghost
    assetghost = o->assetghost;
    time_elapsed = o->time_elapsed;
    playing = o->playing;
    chat_message = o->chat_message;
    chat_message_times = o->chat_message_times;
    head_avatar_pos = o->head_avatar_pos;
    ghost_assetobjs = o->ghost_assetobjs;
    ghost_asset_shaders = o->ghost_asset_shaders;
    do_multiplayer_timeout = o->do_multiplayer_timeout;
    avatar_code = o->avatar_code;
    play_sounds = o->play_sounds;

    child_objects = o->child_objects;

    textgeom_chatmessages = o->textgeom_chatmessages;
    chat_message_time_offsets = o->chat_message_time_offsets;

    error_reported = o->error_reported;

    // Do not set model matrices, always defer to calls to UpdateMatrices();
    save_to_markup = o->save_to_markup;   

    props->Copy(o->props);    
}

QMatrix4x4 RoomObject::GetModelMatrixLocal() const
{
    return model_matrix_local;
}

QMatrix4x4 RoomObject::GetModelMatrixGlobal() const
{
    return model_matrix_global;
}

void RoomObject::DrawLoadingGL(QPointer <AssetShader> shader)
{
    if (anim == NULL) {
        return;
    }

    const bool edit_mode_enabled = SettingsManager::GetEditModeEnabled();
    const bool edit_mode_icons_enabled = SettingsManager::GetEditModeIconsEnabled();

    const ElementType t = props->GetType();

    if (t == TYPE_OBJECT) {
        if (assetobject && !assetobject->GetFinished()) {
            const float value = assetobject->GetProgress();
            SpinAnimation::DrawGL(shader, value, true);
        }
        else { //59.0 - don't draw the cube icon, it interferes with stuff
            if (edit_mode_enabled && edit_mode_icons_enabled && !interface_object) {
                DrawIconGL(shader, object_img);
            }
        }
    }
    else if (t == TYPE_VIDEO) {
        if (assetvideo) {
            if (assetimage && !assetimage->GetFinished() && !GetPlaying()) {
                const float value = assetimage->GetProgress();
                SpinAnimation::DrawGL(shader, value, true);
            }
        }
    }
    else if (t == TYPE_IMAGE) {
        if (assetimage && !assetimage->GetFinished()) {
            const float value = assetimage->GetProgress();
            SpinAnimation::DrawGL(shader, value, true);
        }
    }
    else if (t == TYPE_PARTICLE) {
        if (edit_mode_enabled && edit_mode_icons_enabled && !interface_object) {
            DrawIconGL(shader, particle_img);
        }
    }
}

void RoomObject::UpdateMatrices()
{
    //if any components of the rotation value were updated in JS, modify the xdir/ydir/zdir
    const QVector3D r = props->GetRotation()->toQVector3D();
    if (r != last_rotation)
    {
        last_rotation = r;
        props->SetRotation(r);
    }

    const QVector3D xd = GetXDir();
    const QVector3D yd = GetYDir();
    const QVector3D zd = GetZDir();
    const QVector3D s = GetScale();
    const QVector3D p = GetPos();
    const ElementType t = props->GetType();

    if (t == TYPE_GHOST) {
        //compute rotation angle for ghost
        GhostFrame frame;
        if (assetghost) {
            assetghost->GetGhostFrame(time_elapsed, frame);
        }

        const QString body_id = props->GetBodyID();
        const QString head_id = props->GetHeadID();
        const bool custom_avatar = (SettingsManager::GetUpdateCustomAvatars() && (!body_id.isEmpty() || !head_id.isEmpty()));

        const float angle = MathUtil::RadToDeg(MathUtil::_PI_OVER_2 - atan2f(frame.dir.z(), frame.dir.x()));
        model_matrix_local.setToIdentity();
        model_matrix_local.translate(p);
        model_matrix_local.rotate(angle, 0, 1, 0);
        model_matrix_local.scale(custom_avatar ? s : QVector3D(1.4f,1.4f,1.4f));
    }
    else if (t == TYPE_TEXT || t == TYPE_PARAGRAPH || t == TYPE_IMAGE || t == TYPE_VIDEO) {
        model_matrix_local.setColumn(0, xd);
        model_matrix_local.setColumn(1, yd);
        model_matrix_local.setColumn(2, zd);
        model_matrix_local.setColumn(3, QVector4D(p, 1));
        model_matrix_local.translate(0,0,portal_spacing);
        model_matrix_local.scale(s);

        if (t == TYPE_TEXT) {
            model_matrix_local *= textgeom->GetModelMatrix();
        }
    }
    else if (t == TYPE_LINK) {
        const QVector3D p2 = p + zd * portal_spacing;

        model_matrix_local.setColumn(0, QVector4D(xd.x(), xd.y(), xd.z(), 0.0f));
        model_matrix_local.setColumn(1, QVector4D(yd.x(), yd.y(), yd.z(), 0.0f));
        model_matrix_local.setColumn(2, QVector4D(zd.x(), zd.y(), zd.z(), 0.0f));
        model_matrix_local.setColumn(3, QVector4D(p2.x(), p2.y(), p2.z(), 1.0f));
    }
    else if (t == TYPE_PARTICLE) {
        //model_matrix_local.setToIdentity();
        if (props->GetEmitLocal()) {
            model_matrix_local.setColumn(0, xd);
            model_matrix_local.setColumn(1, yd);
            model_matrix_local.setColumn(2, zd);
            model_matrix_local.setColumn(3, QVector4D(p, 1));
        }
        else {
            model_matrix_local.setToIdentity();
        }
    }
    else {
        model_matrix_local.setColumn(0, xd);
        model_matrix_local.setColumn(1, yd);
        model_matrix_local.setColumn(2, zd);
        model_matrix_local.setColumn(3, QVector4D(p, 1));
        model_matrix_local.scale(s);
    }

    if (props->GetSpinVal() != 0.0f) {
        const float t = time_elapsed * props->GetSpinVal();
        const QVector3D sa = props->GetSpinAxis()->toQVector3D();
        model_matrix_local.rotate(t, sa);
    }

    model_matrix_global = model_matrix_parent * model_matrix_local * MathUtil::ModelMatrix();
    /*
    model_matrix_local.optimize();
    model_matrix_global.optimize();

    if (this->GetParentObject() || selected) {
        qDebug() << "id" << this->GetID()
                 << "model" << MathUtil::ModelMatrix()
                 << "local" << model_matrix_local
                 << "global" << model_matrix_global;
        qDebug() << "done";
    }
    */
}

void RoomObject::DrawCursorGL(QPointer <AssetShader> shader)
{
    const bool visible = props->GetVisible();
    if (!visible) {
        return;
    }

    const ElementType t = GetType();

    if (t == TYPE_GHOST) {
        if (assetghost && assetghost->GetProcessed()) {

            //scalar value that affects rendering/disconnect
            float scale_fac = 1.0f;
            const float disc_time_sec = logoff_rate;
            if (do_multiplayer_timeout && time_elapsed > disc_time_sec) {
                const float time_disconnect = time_elapsed - disc_time_sec;
                scale_fac = qMax(0.0f, 1.0f - time_disconnect*0.25f);
                if (scale_fac <= 0.0f) {
                    return;
                }
            }

            Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::DISABLED);

            //draw ghost's cursor if active
            if (ghost_frame.cursor_active && cursor_arrow_obj) {

                bool use_lighting = shader->GetUseLighting();
                shader->SetUseLighting(false);
                shader->UpdateObjectUniforms();

                MathUtil::PushModelMatrix();
                MathUtil::LoadModelMatrix(ghost_frame.cursor_xform);
                MathUtil::ModelMatrix().translate(0,0,0.01f);
                MathUtil::ModelMatrix().scale(ghost_frame.cscale * scale_fac);

                cursor_arrow_obj->DrawGL(shader, QColor(255,255,255));

                MathUtil::PopModelMatrix();
                shader->SetUseLighting(use_lighting);
            }

            Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::BACK);
        }
    }    
}

void RoomObject::DrawGL(QPointer <AssetShader> shader, const bool left_eye, const QVector3D & player_pos)
{    
    //qDebug() << "RoomObject::DrawGL()" << GetID() << GetJSID() << GetScale() << GetType() << assetobject << assetobject->GetFinished();
    if (shader.isNull() || !shader->GetCompiled() || !props->GetVisible()) {
        return;
    }

    const ElementType t = GetType();
    const QColor col = MathUtil::GetVector4AsColour(props->GetColour()->toQVector4D());
    //qDebug() << "draw" << this << col << t;
    const QColor chromakey_col = MathUtil::GetVector4AsColour(props->GetChromaKeyColour()->toQVector4D());
    const bool edit_mode_enabled = SettingsManager::GetEditModeEnabled();
    const bool edit_mode_icons_enabled = SettingsManager::GetEditModeIconsEnabled();

    const QVector3D pos = GetPos();
    //qDebug() << "RoomObject::DrawGL" << GetS("_type") << GetS("id") << GetV("pos") << GetV("xdir") << GetV("ydir") << GetV("zdir") << GetV("scale");

    const QString cf = props->GetCullFace();
    if (cf == "front") {
        Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::FRONT);
    }
    else if (cf == "none") {
        Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::DISABLED);
    }
    else {
        Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::BACK);
    }
    //qDebug() << "cull_face"<< this << cf;

    const QString body_id = props->GetBodyID();
    const QString head_id = props->GetHeadID();

    //get the model matrix (not including the view portion)            
    //qDebug() << "setting colour" << QVector4D(col.redF(), col.greenF(), col.blueF(), col.alphaF());
    shader->SetAmbient(QVector3D(1,1,1));
    shader->SetDiffuse(QVector3D(1,1,1));
    shader->SetEmission(QVector3D(0,0,0));
    shader->SetSpecular(QVector3D(0.04f,0.04f,0.04f));
    shader->SetShininess(20.0f);
    shader->SetConstColour(QVector4D(col.redF(), col.greenF(), col.blueF(), col.alphaF()));
    shader->SetChromaKeyColour(QVector4D(chromakey_col.redF(), chromakey_col.greenF(), chromakey_col.blueF(), chromakey_col.alphaF()));
    shader->SetUseLighting(props->GetLighting());
    shader->SetUseSkelAnim(false); //53.10 - don't know why this solves the problem but it does, skeletal animation is disabled after FBX draw call but this is still needed
    shader->SetTiling(props->GetTiling()->toQVector4D());
    shader->SetDrawLayer(props->GetDrawLayer());

    //qDebug() << "draw layer" << this << GetI("draw_layer");

    // This allows us to sort objects based on their bounding spheres in the Ren
    float scale_fac = 1.0f;
    const float size = (GetBBoxMax() - GetBBoxMin()).length() * 0.5f;
    const float distance = qAbs(((pos - player_pos).length() - size));
    // Transform the position to be relative to the current_room origin so that it can be transformed into view_space by each camera's viewmatrix
    QVector4D position(pos, 1.0f);
    position = MathUtil::RoomMatrix() * position;

    shader->SetRoomSpacePositionAndDistance(QVector4D(position.x(), position.y(), position.z(), distance));

    //do the draw
    switch (t) {
    case TYPE_PARTICLE:
    {
        bool override_texture = false;

        shader->SetUseTextureAll(false);

        if (assetimage && assetimage->GetFinished()) {
            auto tex_id = assetimage->GetTextureHandle(left_eye);
            Renderer::m_pimpl->BindTextureHandle(0, tex_id);
            override_texture = true;

            shader->SetUseTexture(0, true);
        }

        if (assetobject.isNull()) {
            Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::DISABLED);
            Renderer::m_pimpl->SetDepthMask(DepthMask::DEPTH_WRITES_DISABLED);
        }

        MathUtil::PushModelMatrix();
        MathUtil::MultModelMatrix(model_matrix_local);
        shader->SetOverrideTexture(override_texture);
        particle_system->DrawGL(shader, player_pos, assetobject);
        shader->SetOverrideTexture(false);
        MathUtil::PopModelMatrix();

        if (assetobject.isNull()) {
            Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::BACK);
            Renderer::m_pimpl->SetDepthMask(DepthMask::DEPTH_WRITES_ENABLED);
        }
    }
        break;

    case TYPE_TEXT:
        Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::DISABLED);
        textgeom->SetColour(col);
        MathUtil::PushModelMatrix();
        MathUtil::MultModelMatrix(model_matrix_local);
        textgeom->DrawGL(shader);

        if (selected && edit_mode_enabled) {
            shader->SetConstColour(QVector4D(0.5f, 1.0f, 0.5f, 0.25f));
            shader->SetChromaKeyColour(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
            shader->SetUseTextureAll(false);
            shader->SetUseLighting(false);
            shader->SetAmbient(QVector3D(1.0f, 1.0f, 1.0f));
            shader->SetDiffuse(QVector3D(1.0f, 1.0f, 1.0f));
            shader->SetSpecular(QVector3D(0.04f, 0.04f, 0.04f));
            shader->SetShininess(20.0f);
            textgeom->DrawSelectedGL(shader);
            shader->SetConstColour(QVector4D(1,1,1,1));
        }

        MathUtil::PopModelMatrix();
        Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::BACK);
        break;

    case TYPE_PARAGRAPH:
        if (assetimage) {
            assetimage->UpdateGL();
        }
        if (assetimage && assetimage->GetFinished()) {
            shader->SetUseTextureAll(false);
            shader->SetUseTexture(0, true);

            const float iw = 1.0f;
            const float ih = iw * assetimage->GetAspectRatio();

            MathUtil::PushModelMatrix();
            MathUtil::MultModelMatrix(model_matrix_local);
            MathUtil::ModelMatrix().scale(iw, ih, 1.0f);

            auto tex_id = assetimage->GetTextureHandle(true);
            Renderer::m_pimpl->BindTextureHandle(0, tex_id);

            shader->UpdateObjectUniforms();

            Renderer * renderer = Renderer::m_pimpl;
            AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                    renderer->GetPlanePrimCount(),
                                    0,
                                    0,
                                    0,
                                    renderer->GetPlaneVAO(),
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

            MathUtil::PopModelMatrix();
        }
        break;

    case TYPE_GHOST:
        if (assetghost && assetghost->GetProcessed()) {

            //scalar value that affects rendering/disconnect
            const float disc_time_sec = logoff_rate;
            if (do_multiplayer_timeout && time_elapsed > disc_time_sec) {
                const float time_disconnect = time_elapsed - disc_time_sec;
                scale_fac = qMin(1.0f, qMax(0.0f, 1.0f - time_disconnect*0.25f));
            }

            if (scale_fac > 0.0f) {
                //draw body
                const bool custom_avatar = (SettingsManager::GetUpdateCustomAvatars() && (!body_id.isEmpty() || !head_id.isEmpty()));

                const QVector3D z = GetZDir();
                const QVector3D s = GetScale();

                MathUtil::PushModelMatrix();
                MathUtil::MultModelMatrix(model_matrix_local);
                MathUtil::ModelMatrix().scale(scale_fac, scale_fac, scale_fac);

                if (custom_avatar) {
                    if (ghost_assetobjs.contains(body_id) && ghost_assetobjs[body_id]) {
                        ghost_assetobjs[body_id]->Update();
                        ghost_assetobjs[body_id]->UpdateGL();
                        if (ghost_assetobjs[body_id]->GetFinished()) {
                            ghost_assetobjs[body_id]->DrawGL(shader, col);
                        }
                    }
                }
                else if (avatar_obj && avatar_obj->GetFinished()) {
                    avatar_obj->DrawGL(shader, col);
                }

                //draw head
                MathUtil::PushModelMatrix();

                QMatrix4x4 m = ghost_frame.head_xform;
                m.setColumn(3, QVector4D(head_avatar_pos, 1));
                MathUtil::MultModelMatrix(m);
                MathUtil::ModelMatrix().translate(-head_avatar_pos.x(), -head_avatar_pos.y(), -head_avatar_pos.z());

                if (custom_avatar) {
                    if (ghost_assetobjs.contains(head_id) && ghost_assetobjs[head_id]) {
                        ghost_assetobjs[head_id]->Update();
                        ghost_assetobjs[head_id]->UpdateGL();

                        if (ghost_assetobjs[head_id]->GetFinished()) {
                            ghost_assetobjs[head_id]->DrawGL(shader, col);
                        }
                    }
                }
                else if (avatar_head_obj && avatar_head_obj->GetFinished()) {
                    avatar_head_obj->DrawGL(shader, col);
                }
                MathUtil::PopModelMatrix();

                Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::DISABLED);

                DrawGhostUserIDChat(shader);

                Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::BACK);

                //draw controllers (if present)
                if (ghost_frame.hands.first.is_active || ghost_frame.hands.second.is_active) {
                    MathUtil::PushModelMatrix();
                    MathUtil::ModelMatrix().rotate(180,0,1,0);
                    MathUtil::ModelMatrix().scale(1.0f/s.x(), 1.0f/s.y(), 1.0f/s.z());

                    //qDebug() << MathUtil::ModelMatrix() << ghost_frame.hands.first.basis << ghost_frame.hands.second.basis;
                    if (ghost_frame.hands.first.is_active) {
                        ControllerManager::DrawGhostGL(shader, 0, ghost_frame.hmd_type, ghost_frame.hands.first.basis);
                    }
                    if (ghost_frame.hands.second.is_active) {
                        ControllerManager::DrawGhostGL(shader, 1, ghost_frame.hmd_type, ghost_frame.hands.second.basis);
                    }

                    MathUtil::PopModelMatrix();
                }

                MathUtil::PopModelMatrix();
            }
        }
        break;

    case TYPE_VIDEO:
        if (assetvideo) {

            MathUtil::PushModelMatrix();
            MathUtil::MultModelMatrix(model_matrix_local);

            if (selected && edit_mode_enabled) {
                MathUtil::PushModelMatrix();
                if (assetimage && assetimage->GetFinished() && !assetvideo->GetPlaying(&media_ctx) && !GetPlaying() && !assetvideo->GetCursorActive(&media_ctx)) {
                    MathUtil::ModelMatrix().scale(1.0f, assetimage->GetAspectRatio(), 1.0f);
                    assetimage->DrawSelectedGL(shader);
                }
                else {
                    MathUtil::ModelMatrix().scale(1.0f, assetvideo->GetAspectRatio(&media_ctx), 1.0f);
                    assetvideo->DrawSelectedGL(shader);
                }
                MathUtil::PopModelMatrix();
            }

            if (assetimage && assetimage->GetFinished() && !assetvideo->GetPlaying(&media_ctx) && !GetPlaying() && !assetvideo->GetCursorActive(&media_ctx)) {
                shader->SetUseTexture(0, true);
                assetimage->DrawGL(shader, left_eye);
            }
            else {
                //shader->SetUseTexture(0, true);
                float aspect = 0.7f;
                if (assetimage && assetimage->GetFinished()) {
                    aspect = assetimage->GetAspectRatio();
                }
                else {
                    aspect = assetvideo->GetAspectRatio(&media_ctx);
                }
                assetvideo->DrawGL(&media_ctx, shader, left_eye, aspect);
            }

            MathUtil::PopModelMatrix();
        }
        break;

    case TYPE_IMAGE:
        if (assetimage && assetimage->GetFinished()) {
            MathUtil::PushModelMatrix();
            MathUtil::MultModelMatrix(model_matrix_local);

            if (selected && edit_mode_enabled) {
                MathUtil::PushModelMatrix();
                MathUtil::ModelMatrix().scale(1.0f, assetimage->GetAspectRatio(), 1.0f);
                assetimage->DrawSelectedGL(shader);
                MathUtil::PopModelMatrix();
            }

            shader->SetUseTexture(0, true);
            shader->SetUseLighting(props->GetLighting());
            assetimage->DrawGL(shader, left_eye);

            MathUtil::PopModelMatrix();
        }
        break;

    case TYPE_LIGHT:
        if (edit_mode_icons_enabled) {
            DrawIconGL(shader, light_img);
        }
        break;

    case TYPE_SOUND:
        if (edit_mode_icons_enabled) {
            DrawIconGL(shader, sound_img);
        }
        break;

    case TYPE_OBJECT:
    {
        bool override_texture = false;
        bool is_hdr = false;
        bool is_flipped = false;

        QPointer <AssetObject> obj = assetobject;
        QPointer <AssetWebSurface> aw = assetwebsurface;
        QPointer <AssetImage> ai = assetimage;
        QPointer <AssetVideo> av = assetvideo;

        if (aw && (aw->GetFocus() || ai.isNull())) {
            Renderer::m_pimpl->BindTextureHandle(0, aw->GetTextureHandle());
            override_texture = true;
        }
        else if (av && av->GetPlaying(&media_ctx) && av->GetTextureHandle(&media_ctx, left_eye)
                 && av->GetTextureHandle(&media_ctx, left_eye) != AssetImage::null_image_tex_handle) {
            Renderer::m_pimpl->BindTextureHandle(0, av->GetTextureHandle(&media_ctx, left_eye));
            override_texture = true;
            is_flipped = false;
        }
        else if (ai && ai->GetFinished()) {
            auto tex_id = ai->GetTextureHandle(left_eye);
            Renderer::m_pimpl->BindTextureHandle(0, tex_id);
            override_texture = true;
            is_hdr = ai->GetIsHDR();
        }

        shader->SetUseTextureAll(false);
        shader->SetUseTexture(0, override_texture, is_hdr, is_flipped);

        if (m_cubemap_radiance && m_cubemap_radiance->GetFinished()) {
            Renderer::m_pimpl->BindTextureHandle(11, m_cubemap_radiance->GetTextureHandle(left_eye));
            shader->SetUseCubeTexture1(true);
        }

        if (m_cubemap_irradiance && m_cubemap_irradiance->GetFinished()) {
            Renderer::m_pimpl->BindTextureHandle(12, m_cubemap_irradiance->GetTextureHandle(left_eye));
            shader->SetUseCubeTexture2(true);
        }

        if (assetimage_lmap && assetimage_lmap->GetFinished())
        {
            Renderer::m_pimpl->BindTextureHandle(8, assetimage_lmap->GetTextureHandle(left_eye));
            shader->SetUseTexture(8, true, assetimage_lmap->GetIsHDR());
            QVector4D lmapScale = props->GetLightmapScale()->toQVector4D();
            shader->SetLightmapScale(lmapScale);
            shader->SetOverrideLightmap(true); //62.9 - AssetShaders now hold a boolean for texture or lightmap overrides
        }

        MathUtil::PushModelMatrix();
        MathUtil::MultModelMatrix(model_matrix_local);

        shader->SetOverrideTexture(override_texture);
        if (obj && obj->GetFinished()) {            
            obj->DrawGL(shader, col);
        }

        if (assetobject_teleport && draw_assetobject_teleport && assetobject_teleport->GetFinished()) {
            assetobject_teleport->DrawGL(shader, col);
        }
        shader->SetOverrideTexture(false);
        shader->SetOverrideLightmap(false);

        if (override_texture) {
            Renderer::m_pimpl->BindTextureHandle(0, AssetImage::null_image_tex_handle);
        }
        shader->SetUseTextureAll(false);

        if (obj && selected && edit_mode_enabled) {
            shader->SetChromaKeyColour(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
            shader->SetUseLighting(false);
            shader->SetAmbient(QVector3D(1.0f, 1.0f, 1.0f));
            shader->SetDiffuse(QVector3D(1.0f, 1.0f, 1.0f));
            shader->SetSpecular(QVector3D(0.04f, 0.04f, 0.04f));
            shader->SetShininess(20.0f);

            shader->SetOverrideTexture(true);
            obj->DrawGL(shader, QColor(128, 255, 128, 64));

            if (assetobject_collision && assetobject_collision->GetFinished()) {
                const QVector3D p = props->GetCollisionPos()->toQVector3D();
                const QVector3D s = props->GetCollisionScale()->toQVector3D();

                MathUtil::PushModelMatrix();
                MathUtil::ModelMatrix().translate(p);
                MathUtil::ModelMatrix().scale(s);
                assetobject_collision->DrawGL(shader, QColor(255, 0, 0, 64));
                MathUtil::PopModelMatrix();
            }

            shader->SetOverrideTexture(false);
            shader->SetConstColour(QVector4D(1,1,1,1));
        }

        MathUtil::PopModelMatrix();
    }
        break;

    default:
        break;

    }

    //draw child objects, and if ghost, don't continue to draw child objects if faded away
    if (!child_objects.isEmpty() && scale_fac > 0.0f) {
        MathUtil::PushModelMatrix();
        MathUtil::MultModelMatrix(model_matrix_local);
        MathUtil::ModelMatrix().scale(scale_fac);

        for (int i=0; i<child_objects.size(); ++i) {
            if (child_objects[i]) {

                QPointer <AssetObject> a;
                if (ghost_assetobjs.contains(body_id) && ghost_assetobjs[body_id]) {
                    a = ghost_assetobjs[body_id];
                }
                else {
                    a = assetobject;
                }

                QPointer <AssetShader> s = shader;
                if (child_objects[i]->GetAssetShader()) {
                    s = child_objects[i]->GetAssetShader();
                }

                //apply a secondary bone xform
                const QString bone = child_objects[i]->GetProperties()->GetBoneID();
                if (a && a->GetGeom() && bone.length() > 0) {
                    const QMatrix4x4 bone_xform = a->GetGeom()->GetFinalPose(bone);

                    MathUtil::PushModelMatrix();
                    MathUtil::MultModelMatrix(bone_xform);
                    child_objects[i]->DrawGL(s, left_eye, player_pos);
                    MathUtil::PopModelMatrix();
                }
                else {
                    child_objects[i]->DrawGL(s, left_eye, player_pos);
                }
            }
        }
        MathUtil::PopModelMatrix();
    }

    //56.0 - Karan's request: do not use clip plane on loading icons
    const bool clip_plane = shader->GetUseClipPlane();
    shader->SetUseClipPlane(false);
    MathUtil::PushModelMatrix();
    MathUtil::MultModelMatrix(model_matrix_local);
    DrawLoadingGL(shader);
    MathUtil::PopModelMatrix();
    shader->SetUseClipPlane(clip_plane);

    Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::BACK);

    shader->SetUseSkelAnim(false);
    shader->SetConstColour(QVector4D(1,1,1,1));
    shader->SetChromaKeyColour(QVector4D(0,0,0,0));
    shader->SetUseTextureAll(false);
}

void RoomObject::DrawIconGL(QPointer <AssetShader> shader, const QPointer <AssetImage> img)
{
    if (img) {
        MathUtil::PushModelMatrix();
        //MathUtil::MultModelMatrix(model_matrix_local);
        MathUtil::ModelMatrix().translate(GetPos());

        auto tex_id = img->GetTextureHandle(true);
        anim->DrawIconGL(shader, true, tex_id, QColor(255,255,255));

        if (selected) {            
            anim->DrawIconGL(shader, true, 0, QColor(128,255,128,64));
            shader->SetConstColour(QVector4D(1,1,1,1));            
        }

        MathUtil::PopModelMatrix();
    }
}

void RoomObject::DrawCollisionModelGL(QPointer <AssetShader> shader)
{
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }

    if (assetobject_collision.isNull() || !assetobject_collision->GetFinished()) {
        return;
    }

    const ElementType t = GetType();
    if (t == TYPE_OBJECT) {
        shader->SetConstColour(QVector4D(1,0.5f,0.5f,0.25f));
        MathUtil::PushModelMatrix();
        MathUtil::MultModelMatrix(model_matrix_local);
        assetobject_collision->DrawGL(shader);
        MathUtil::PopModelMatrix();
        shader->SetConstColour(QVector4D(1,1,1,1));       
    }          
}

void RoomObject::SetText(const QString & s)
{
    SetText(s, true);
}

void RoomObject::SetText(const QString & s, const bool add_markup)
{
    //qDebug() << "EnvObject::SetText" << s;
    props->SetText(s);

    const ElementType t = GetType();

    if (t == TYPE_TEXT) {
        textgeom->SetText(s, MathUtil::GetVector4AsColour(props->GetColour()->toQVector4D()));
        textgeom->SetMaxSize(1.0f, 1.0f);
    }
    else if (t == TYPE_PARAGRAPH) {
        if (assetimage.isNull()) {
            assetimage = new AssetImage();
        }
        assetimage->CreateFromText(s, props->GetFontSize(), add_markup,
                                   MathUtil::GetVector4AsColour(props->GetTextCol()->toQVector4D()),
                                   MathUtil::GetVector4AsColour(props->GetBackCol()->toQVector4D()),
                                   props->GetBackAlpha(), tex_width, tex_height, false);
    }
}

QString RoomObject::GetText() const
{
    return props->GetText();
}

void RoomObject::SetFixedSize(const bool fixed_size, const float size) {
    textgeom->SetFixedSize(fixed_size, size);
}

void RoomObject::SetAssetSound(const QPointer <AssetSound> a)
{
    if (a != assetsound) {
        if (assetsound) {
            assetsound->Stop(&media_ctx);
        }
        assetsound = a;
        media_ctx.setup = false;
    }
}

void RoomObject::SetAssetVideo(const QPointer <AssetVideo> a)
{
    if (a != assetvideo) {
        if (assetvideo) {
            assetvideo->Stop(&media_ctx);
        }
        assetvideo = a;
        media_ctx.setup = false;
    }
}

QPointer <AssetVideo> RoomObject::GetAssetVideo()
{
    return assetvideo;
}

void RoomObject::SetAssetWebSurface(const QPointer <AssetWebSurface> a)
{
    assetwebsurface = a;
}

QPointer<AssetWebSurface> RoomObject::GetAssetWebSurface()
{
    return assetwebsurface;
}

void RoomObject::SetAssetShader(const QPointer <AssetShader> a)
{
    assetshader = a;
}

QPointer <AssetShader> RoomObject::GetAssetShader()
{
    return assetshader;
}

bool RoomObject::TestPlayerPosition(const QPointF & p)
{
    const QRectF trigger_rect = props->GetTriggerRect();
    if (!props->GetTriggered() && GetType() == TYPE_SOUND && assetsound && trigger_rect.contains(p)) {
        props->SetTriggered(true);
        return true;
    }
    return false;
}

void RoomObject::Seek(const float f)
{
    if (props->GetType() == TYPE_SOUND && assetsound) {
        assetsound->Seek(&media_ctx, f);
    }
}

void RoomObject::Pause()
{
    if (GetType() == TYPE_SOUND && assetsound) {
        assetsound->Pause(&media_ctx);
    }
}

void RoomObject::Play()
{
    //qDebug() << "RoomObject::Play()" << this->GetID() << this->GetJSID() << this->GetPos();
    const ElementType t = GetType();
    if (t == TYPE_SOUND || t == TYPE_LINK || t == TYPE_OBJECT) {
        if (assetsound) {
            assetsound->Play(&media_ctx);
            playing = true;
        }
    }
    else if (t == TYPE_GHOST){
        playing = true;

        //if not on a loop and finished, restart it
        if (!props->GetLoop() && assetghost) {
            const bool sequence_end = assetghost->GetGhostFrame(time_elapsed, ghost_frame);
            if (sequence_end) {
                time_elapsed = 0.0f;
            }
        }
    }    
    else if (t == TYPE_VIDEO) {
        if (assetvideo) {
            assetvideo->Play(&media_ctx);
        }    
    }

}

void RoomObject::Stop()
{    
    const ElementType t = GetType();
    if (t == TYPE_SOUND) {
        if (assetsound) {
            assetsound->Stop(&media_ctx);
        }
    }
    else if (t == TYPE_VIDEO) {
        if (assetvideo) {
            assetvideo->Stop(&media_ctx);
        }
    }
    else if (t == TYPE_GHOST) {
        playing = false;    
    }
}

bool RoomObject::GetPlaying() const
{
    const ElementType t = GetType();
    if (t == TYPE_SOUND) {
        if (assetsound) {
            return assetsound->GetPlaying((MediaContext *) &media_ctx);
        }
    }
    else if (t == TYPE_VIDEO) {
        if (assetvideo) {
            return assetvideo->GetPlaying((MediaContext *) &media_ctx);
        }
    }
    else if (t == TYPE_GHOST) {
        return playing;    
    }

    return false;
}

void RoomObject::ReadXMLCode(const QString & str)
{
    QXmlStreamReader reader(str);

    while (!reader.atEnd()) {

        reader.readNext();

        QString tag_name = reader.name().toString().toLower();
        QXmlStreamAttributes attributes = reader.attributes();

        //qDebug() << reader.tokenString() << reader.text() << reader.name();
        if (reader.tokenType() == QXmlStreamReader::StartElement) {

            if (tag_name == "ghost" ||
                    tag_name == "object" ||
                    tag_name == "video" ||
                    tag_name == "image" ||
                    tag_name == "link" ||
                    tag_name == "text" ||
                    tag_name == "paragraph" ||
                    tag_name == "sound" ||
                    tag_name == "particle" ||
                    tag_name == "light") {

                if (attributes.hasAttribute("id")) {
                    props->SetID(attributes.value("id").toString());
                }
                if (attributes.hasAttribute("cubemap_radiance_id")) {
                    props->SetCubemapRadianceID(attributes.value("cubemap_radiance_id").toString());
                }
                if (attributes.hasAttribute("cubemap_irradiance_id")) {
                    props->SetCubemapIrradianceID(attributes.value("cubemap_irradiance_id").toString());
                }
                if (attributes.hasAttribute("collision_id")) {
                    props->SetCollisionID(attributes.value("collision_id").toString());
                }
                if (attributes.hasAttribute("collision_response")) {
                    props->SetCollisionResponse(MathUtil::GetStringAsBool(attributes.value("collision_response").toString()));
                }
                if (attributes.hasAttribute("js_id")) {
                    props->SetJSID(attributes.value("js_id").toString());
                }
                if (attributes.hasAttribute("pos")) {
                    props->SetPos(MathUtil::GetStringAsVector(attributes.value("pos").toString()));
                }
                if (attributes.hasAttribute("vel")) {
                    props->SetVel(MathUtil::GetStringAsVector(attributes.value("vel").toString()));
                }
                if (attributes.hasAttribute("accel")) {
                    props->SetAccel(MathUtil::GetStringAsVector(attributes.value("accel").toString()));
                }
                if (attributes.hasAttribute("xdir")) {
                    props->SetXDir(MathUtil::GetStringAsVector(attributes.value("xdir").toString()));
                }
                if (attributes.hasAttribute("ydir")) {
                    props->SetYDir(MathUtil::GetStringAsVector(attributes.value("ydir").toString()));
                }
                if (attributes.hasAttribute("zdir")) {
                    props->SetZDir(MathUtil::GetStringAsVector(attributes.value("zdir").toString()));
                }
                if (attributes.hasAttribute("rotation_order")) {
                    props->SetRotationOrder(attributes.value("rotation_order").toString());
                }
                if (attributes.hasAttribute("rotation")) {
                    props->SetRotation(MathUtil::GetStringAsVector(attributes.value("rotation").toString()));
                }
                if (attributes.hasAttribute("scale")) {
                    props->SetScale(MathUtil::GetStringAsVector(attributes.value("scale").toString()));
                }
                if (attributes.hasAttribute("col")) {
                    props->SetColour(MathUtil::GetStringAsColour(attributes.value("col").toString()));
                }
                if (attributes.hasAttribute("head_id")) {
                    props->SetHeadID(attributes.value("head_id").toString());
                }
                if (attributes.hasAttribute("head_pos")) {
                    SetHeadAvatarPos(MathUtil::GetStringAsVector(attributes.value("head_pos").toString()));
                }
                if (attributes.hasAttribute("body_id")) {
                    props->SetBodyID(attributes.value("body_id").toString());
                }
                if (attributes.hasAttribute("lighting")) {
                    props->SetLighting(MathUtil::GetStringAsBool(attributes.value("lighting").toString()));
                }
                if (attributes.hasAttribute("collision_static")) {
                    props->SetCollisionStatic(MathUtil::GetStringAsBool(attributes.value("collision_static").toString()));
                }
                if (attributes.hasAttribute("collision_trigger")) {
                    props->SetCollisionTrigger(MathUtil::GetStringAsBool(attributes.value("collision_trigger").toString()));
                }
                if (attributes.hasAttribute("collision_ccdmotionthreshold")) {
                    props->SetCollisionCcdMotionThreshold(attributes.value("collision_ccdmotionthreshold").toString().toFloat());
                }
                if (attributes.hasAttribute("collision_ccdsweptsphereradius")) {
                    props->SetCollisionCcdSweptSphereRadius(attributes.value("collision_ccdsweptsphereradius").toString().toFloat());
                }
                if (attributes.hasAttribute("visible")) {
                    props->SetVisible(MathUtil::GetStringAsBool(attributes.value("visible").toString()));
                }
                if (attributes.hasAttribute("cull_face")) {
                    props->SetCullFace(attributes.value("cull_face").toString());
                }
                if (attributes.hasAttribute("swallow")) {
                    props->SetSwallow(MathUtil::GetStringAsBool(attributes.value("swallow").toString()));
                }

                props->SetType(DOMNode::StringToElementType(tag_name));

                if (tag_name == TYPE_TEXT) {
                    reader.readNext();
                    SetText(reader.text().toString());
                }
                else if (tag_name == TYPE_PARAGRAPH) {
                    reader.readNext();
                    SetText(reader.text().toString());
                }
            }
        }
    }
}

QString RoomObject::GetXMLCode(const bool show_defaults) const
{        
    //qDebug() << "RoomObject::GetXMLCode" << this << props->GetJSID();
    const ElementType t = props->GetType();
    const QString t_str = DOMNode::ElementTypeToTagName(t);

    QString code_str = QString("<") + t_str;

    if (show_defaults || props->GetID().length() > 0) {
        code_str += " id=" + MathUtil::GetStringAsString(props->GetID());
    }
    bool jsid_ok;
    props->GetJSID().toFloat(&jsid_ok);
    if (show_defaults || (props->GetJSID().length() > 0 && !jsid_ok)) {
        code_str += " js_id=" + MathUtil::GetStringAsString(props->GetJSID());
    }
    if (show_defaults || props->GetLocked()) {
        code_str += " locked=" + MathUtil::GetBoolAsString(props->GetLocked());
    }
    if (show_defaults || props->GetOnClick().length() > 0) {
        code_str += " onclick=" + MathUtil::GetStringAsString(props->GetOnClick());
    }
    /*
    if ((show_defaults && !network_optimize) || GetOnCollision().length() > 0) {
        code_str += " oncollision=" + MathUtil::GetStringAsString(GetOnCollision());
    }
    */
    if (show_defaults || GetInterpTime() != 0.1f) {
        code_str += " interp_time=" + MathUtil::GetFloatAsString(GetInterpTime());
    }
    if (show_defaults || GetPos() != QVector3D(0,0,0)) {
        code_str += " pos=" + MathUtil::GetVectorAsString(props->GetPos()->toQVector3D());
    }
    if (show_defaults || GetVel() != QVector3D(0,0,0)) {
        code_str += " vel=" + MathUtil::GetVectorAsString(GetVel());
    }
    if (show_defaults || GetAccel() != QVector3D(0,0,0)) {
        code_str += " accel=" + MathUtil::GetVectorAsString(GetAccel());
    }
    if (show_defaults || props->GetRotationOrder() != "xyz") {
        code_str += " rotation_order=" + MathUtil::GetStringAsString(props->GetRotationOrder());
    }
    if (props->GetRotation()->toQVector3D() != QVector3D(0,0,0)) {
        code_str += " rotation=" + MathUtil::GetVectorAsString(props->GetRotation()->toQVector3D());
    }
    else {
        if (show_defaults || GetXDir() != QVector3D(1,0,0)) {
            code_str += " xdir=" + MathUtil::GetVectorAsString(props->GetXDir()->toQVector3D());
        }
        if (show_defaults || GetYDir() != QVector3D(0,1,0)) {
            code_str += " ydir=" + MathUtil::GetVectorAsString(props->GetYDir()->toQVector3D());
        }
        if (show_defaults || GetZDir() != QVector3D(0,0,1)) {
            code_str += " zdir=" + MathUtil::GetVectorAsString(props->GetZDir()->toQVector3D());
        }
    }
    if (show_defaults || GetScale() != QVector3D(1,1,1) || t == TYPE_GHOST) {
        code_str += " scale=" + MathUtil::GetVectorAsString(props->GetScale()->toQVector3D());
    }
    if (show_defaults || MathUtil::GetVector4AsColour(props->GetColour()->toQVector4D()) != QColor(255,255,255)) {
        code_str += " col=" + MathUtil::GetColourAsString(MathUtil::GetVector4AsColour(props->GetColour()->toQVector4D()));
    }
    if (show_defaults || !props->GetLighting()) {
        code_str += " lighting=" + MathUtil::GetBoolAsString(props->GetLighting());
    }
    if (show_defaults || !props->GetCollisionStatic()) {
        code_str += " collision_static=" + MathUtil::GetBoolAsString(props->GetCollisionStatic());
    }
    if (show_defaults || props->GetCollisionTrigger()) {
        code_str += " collision_trigger=" + MathUtil::GetBoolAsString(props->GetCollisionTrigger());
    }
    if (show_defaults || props->GetCollisionCcdMotionThreshold() != 1.0f) {
        code_str += " collision_ccdmotionthreshold=" + MathUtil::GetFloatAsString(props->GetCollisionCcdMotionThreshold());
    }
    if (show_defaults || props->GetCollisionCcdSweptSphereRadius() != 0.0f) {
        code_str += " collision_ccdsweptsphereradius=" + MathUtil::GetFloatAsString(props->GetCollisionCcdSweptSphereRadius());
    }
    if (show_defaults || !props->GetVisible()) {
        code_str += " visible=" + MathUtil::GetBoolAsString(props->GetVisible());
    }
    if (show_defaults || props->GetShaderID().length() > 0) {
        code_str += " shader_id=" + MathUtil::GetStringAsString(props->GetShaderID());
    }
    if (t == TYPE_PARAGRAPH && (show_defaults || MathUtil::GetVector4AsColour(props->GetBackCol()->toQVector4D()) != QColor(255,255,255))) {
        code_str += " back_col=" + MathUtil::GetColourAsString(MathUtil::GetVector4AsColour(props->GetBackCol()->toQVector4D()));
    }
    if (t == TYPE_PARAGRAPH && (show_defaults || props->GetBackAlpha() != 1.0f)) {
        code_str += " back_alpha=" + MathUtil::GetFloatAsString(props->GetBackAlpha());
    }
    if (t == TYPE_PARAGRAPH && (show_defaults || MathUtil::GetVector4AsColour(props->GetTextCol()->toQVector4D())  != QColor(0,0,0))) {
        code_str += " text_col=" + MathUtil::GetVector4AsString(props->GetTextCol()->toQVector4D());
    }
    if (t == TYPE_PARAGRAPH && (show_defaults || props->GetFontSize() != 16)) {
        code_str += " font_size=" + MathUtil::GetFloatAsString(props->GetFontSize());
    }
    if (t == TYPE_GHOST && (show_defaults || props->GetHeadID().length() > 0)) {
        code_str += " head_id=" + MathUtil::GetStringAsString(props->GetHeadID());
    }
    if (t == TYPE_GHOST && (show_defaults || head_avatar_pos != QVector3D(0,1,0))) {
        code_str += " head_pos=" + MathUtil::GetVectorAsString(head_avatar_pos);
    }
    if (t == TYPE_GHOST && (show_defaults || props->GetBodyID().length() > 0)) {
        code_str += " body_id=" + MathUtil::GetStringAsString(props->GetBodyID());
    }
    if ((t == TYPE_GHOST || t == TYPE_OBJECT) && ((show_defaults) || props->GetAnimID().length() > 0)) {
        code_str += " anim_id=" + MathUtil::GetStringAsString(props->GetAnimID());
    }
    if ((t == TYPE_GHOST || t == TYPE_OBJECT) && ((show_defaults) || props->GetAnimSpeed() != 1.0f)) {
        code_str += " anim_speed=" + MathUtil::GetFloatAsString(props->GetAnimSpeed());
    }
    if (t == TYPE_GHOST && (show_defaults || props->GetEyePos()->toQVector3D() != QVector3D(0,1.6f,0))) {
        code_str += " eye_pos=" + MathUtil::GetVectorAsString(props->GetEyePos()->toQVector3D());
    }
    if (t == TYPE_GHOST && (show_defaults || userid_pos != QVector3D(0,0,0))) {
        code_str += " userid_pos=" + MathUtil::GetVectorAsString(userid_pos);
    }
    if (t == TYPE_GHOST && (show_defaults || eye_ipd != 0.0f)) {
        code_str += " eye_ipd=" + MathUtil::GetFloatAsString(eye_ipd);
    }
    if (show_defaults || props->GetThumbID().length() > 0) {
        code_str += " thumb_id=" + MathUtil::GetStringAsString(props->GetThumbID());
    }
    if (t == TYPE_LINK && (show_defaults || props->GetOriginalURL().length() > 0)) {
        code_str += " url=" + MathUtil::GetStringAsString(props->GetOriginalURL());
    }
    if (t == TYPE_LINK && (show_defaults || GetTitle().length() > 0)) {
        code_str += " title=" + MathUtil::GetStringAsString(GetTitle());
    }
    if (t == TYPE_LINK && (show_defaults || props->GetAutoLoad())) {
        code_str += " auto_load=" + MathUtil::GetBoolAsString(props->GetAutoLoad());
    }
    if (t == TYPE_LINK && (show_defaults || !props->GetDrawText())) {
        code_str += " draw_text=" + MathUtil::GetBoolAsString(props->GetDrawText());
    }
    if (t == TYPE_LINK && (show_defaults || props->GetMirror())) {
        code_str += " mirror=" + MathUtil::GetBoolAsString(props->GetMirror());
    }
    if (t == TYPE_LINK && (show_defaults || !props->GetActive())) {
        code_str += " active=" + MathUtil::GetBoolAsString(props->GetActive());
    }
    if (t == TYPE_SOUND && (show_defaults || props->GetTriggerRect() != QRect(0,0,0,0))) {
        code_str += " rect=" + MathUtil::GetRectangleAsString(props->GetTriggerRect());
    }
    if (t != TYPE_OBJECT && (show_defaults || props->GetLoop())) {
        code_str += " loop=" + MathUtil::GetBoolAsString(props->GetLoop());
    }
    if (t != TYPE_OBJECT && (show_defaults || props->GetGain() != 1.0f)) {
        code_str += " gain=" + MathUtil::GetFloatAsString(props->GetGain());
    }
    if (t != TYPE_OBJECT && (show_defaults || props->GetDopplerFactor() != 1.0f)) {
        code_str += " doppler_factor=" + MathUtil::GetFloatAsString(props->GetDopplerFactor());
    }
    if (t != TYPE_OBJECT && (show_defaults || props->GetPitch() != 1.0f)) {
        code_str += " pitch=" + MathUtil::GetFloatAsString(props->GetPitch());
    }
    if (t != TYPE_OBJECT && (show_defaults || props->GetAutoPlay())) {
        code_str += " auto_play=" + MathUtil::GetBoolAsString(props->GetAutoPlay());
    }
    if ((t == TYPE_OBJECT || t == TYPE_GHOST) && (show_defaults || props->GetCullFace() != "back")) {
        code_str += " cull_face=" + MathUtil::GetStringAsString(props->GetCullFace());
    }
    if (t != TYPE_OBJECT && (show_defaults || props->GetPlayOnce())) {
        code_str += " play_once=" + MathUtil::GetBoolAsString(props->GetPlayOnce());
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetCollisionID().length() > 0)) {
        code_str += " collision_id=" + MathUtil::GetStringAsString(props->GetCollisionID());
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetCollisionRadius() != 0)) {
        code_str += " collision_radius=" + MathUtil::GetFloatAsString(props->GetCollisionRadius());
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetCollisionResponse() == false)) {
        code_str += " collision_response=" + MathUtil::GetBoolAsString(props->GetCollisionResponse());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetWebsurfaceID().length() > 0)) {
        code_str += " websurface_id=" + MathUtil::GetStringAsString(props->GetWebsurfaceID());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetTeleportID().length() > 0)) {
        code_str += " teleport_id=" + MathUtil::GetStringAsString(props->GetTeleportID());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetBoneID().length() > 0)) {
        code_str += " bone_id=" + MathUtil::GetStringAsString(props->GetBoneID());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetVideoID().length() > 0)) {
        code_str += " video_id=" + MathUtil::GetStringAsString(props->GetVideoID());
    }
    if ((t == TYPE_OBJECT || t == TYPE_PARTICLE) && (show_defaults || props->GetImageID().length() > 0)) {
        code_str += " image_id=" + MathUtil::GetStringAsString(props->GetImageID());
    }
    if ((t == TYPE_OBJECT || t == TYPE_LIGHT) && (show_defaults || props->GetSpinAxis()->toQVector3D() != QVector3D(0,1,0))) {
        code_str += " rotate_axis=" + MathUtil::GetVectorAsString(props->GetSpinAxis()->toQVector3D());
    }
    if ((t == TYPE_OBJECT || t == TYPE_LIGHT) && ((show_defaults) || props->GetSpinVal() != 0)) {
        code_str += " rotate_deg_per_sec=" + MathUtil::GetFloatAsString(props->GetSpinVal());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetBlend0ID().length() > 0)) {
        code_str += " blend0_id="+ MathUtil::GetStringAsString(props->GetBlend0ID());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetBlend1ID().length() > 0)) {
        code_str += " blend1_id="+ MathUtil::GetStringAsString(props->GetBlend1ID());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetBlend2ID().length() > 0)) {
        code_str += " blend2_id="+ MathUtil::GetStringAsString(props->GetBlend2ID());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetBlend3ID().length() > 0)) {
        code_str += " blend3_id="+ MathUtil::GetStringAsString(props->GetBlend3ID());
    }
    if (t == TYPE_SOUND && (show_defaults || props->GetOuterGain() != 0.0f)) {
        code_str += " outer_gain=" + MathUtil::GetFloatAsString(props->GetOuterGain());
    }
    if (t == TYPE_SOUND && (show_defaults || props->GetInnerAngle() != 360.0f)) {
        code_str += " inner_angle=" + MathUtil::GetFloatAsString(props->GetInnerAngle());
    }
    if (t == TYPE_SOUND && (show_defaults || props->GetOuterAngle() != 360.0f)) {
        code_str += " outer_angle=" + MathUtil::GetFloatAsString(props->GetOuterAngle());
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetCount() != 0)) {
        code_str += " count=" + MathUtil::GetIntAsString(props->GetCount());
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRate() != 1)) {
        code_str += " rate=" + MathUtil::GetIntAsString(props->GetRate());
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetDuration() != 1.0f)) {
        code_str += " duration=" + MathUtil::GetFloatAsString(props->GetDuration());
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetFadeIn() != 1.0f)) {
        code_str += " fade_in=" + MathUtil::GetFloatAsString(props->GetFadeIn());
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetFadeOut() != 1.0f)) {
        code_str += " fade_out=" + MathUtil::GetFloatAsString(props->GetFadeOut());
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRandPos()->toQVector3D() != QVector3D(0,0,0))) {
        code_str += " rand_pos=" + MathUtil::GetVectorAsString(props->GetRandPos()->toQVector3D());
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRandVel()->toQVector3D() != QVector3D(0,0,0))) {
        code_str += " rand_vel=" + MathUtil::GetVectorAsString(props->GetRandVel()->toQVector3D());
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRandAccel()->toQVector3D() != QVector3D(0,0,0))) {
        code_str += " rand_accel=" + MathUtil::GetVectorAsString(props->GetRandAccel()->toQVector3D());
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRandColour()->toQVector3D() != QVector3D(0,0,0))) {
        code_str += " rand_col=" + MathUtil::GetVectorAsString(props->GetRandColour()->toQVector3D());
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRandScale()->toQVector3D() != QVector3D(0,0,0))) {
        code_str += " rand_scale=" + MathUtil::GetVectorAsString(props->GetRandScale()->toQVector3D());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || m_cubemap_radiance != 0)) {
        if (m_cubemap_radiance) {
            code_str += " cubemap_radiance_id=\"" + m_cubemap_radiance->GetProperties()->GetID() + "\"";
        }
    }
    if (t == TYPE_OBJECT && ((show_defaults) || m_cubemap_irradiance != 0)) {
        if (m_cubemap_irradiance) {
            code_str += " cubemap_irradiance_id=\"" + m_cubemap_irradiance->GetProperties()->GetID() + "\"";
        }
    }
    if (t == TYPE_LIGHT) {
        code_str += " light_intensity=" + MathUtil::GetFloatAsString(props->GetLightIntensity());
    }
    if (t == TYPE_LIGHT) {
        code_str += " light_cone_angle=" + MathUtil::GetFloatAsString(props->GetLightConeAngle());
    }
    if (t == TYPE_LIGHT) {
        code_str += " light_cone_exponent=" + MathUtil::GetFloatAsString(props->GetLightConeExponent());
    }
    if (t == TYPE_LIGHT) {
        code_str += " light_range=" + MathUtil::GetFloatAsString(props->GetLightRange());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionPos()->toQVector3D() != QVector3D(0,0,0))) {
        code_str += " collision_pos=" + MathUtil::GetVectorAsString(props->GetCollisionPos()->toQVector3D());
    }    
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionScale()->toQVector3D() != QVector3D(1,1,1))) {
        code_str += " collision_scale=" + MathUtil::GetVectorAsString(props->GetCollisionScale()->toQVector3D());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionFriction() != 0.5f)) {
        code_str += " collision_friction=" + MathUtil::GetFloatAsString(props->GetCollisionFriction());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionRollingFriction() != 0.01f)) {
        code_str += " collision_rollingfriction=" + MathUtil::GetFloatAsString(props->GetCollisionRollingFriction());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionRestitution() != 0.85f)) {
        code_str += " collision_restitution=" + MathUtil::GetFloatAsString(props->GetCollisionRestitution());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionAngularDamping() != 0.1f)) {
        code_str += " collision_angulardamping=" + MathUtil::GetFloatAsString(props->GetCollisionAngularDamping());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionLinearDamping() != 0.15f)) {
        code_str += " collision_lineardamping=" + MathUtil::GetFloatAsString(props->GetCollisionLinearDamping());
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetDrawLayer() != 0)) {
        code_str += " draw_layer=" + MathUtil::GetIntAsString(props->GetDrawLayer());
    }
    if (t == TYPE_LINK && ((show_defaults) || props->GetSwallow())) {
        code_str += " swallow=" + MathUtil::GetBoolAsString(props->GetSwallow());
    }   
    if (!props->GetLightmapID().isEmpty()) {
        code_str += " lmap_id=" + MathUtil::GetStringAsString(props->GetLightmapID());
    }
    if (props->GetLightmapScale()->toQVector4D() != QVector4D(1,1,0,0)) {
        code_str += " lmap_sca=" + MathUtil::GetVector4AsString(props->GetLightmapScale()->toQVector4D());
    }

    //add text stuff if there is any in the middle
    switch (t) {
    case TYPE_TEXT:
        code_str += ">";
        code_str += GetText();
        break;

    case TYPE_PARAGRAPH:
        code_str += ">";
        code_str += GetText();
        break;

    default:
        break;
    }

    //now close it off
    if (child_objects.empty()) {
        switch (t) {
        case TYPE_TEXT:
            code_str += QString("</Text>");
            break;
        case TYPE_PARAGRAPH:
            code_str += "</Paragraph>";
            break;
        default:
            code_str += " />";
            break;
        }
    }
    else {
        code_str += " >\n";
        //Add in all my child's tags recursively
        QMap <ElementType, QString> code;
        for (const QPointer <RoomObject> & obj : child_objects) {
            if (obj && obj->GetSaveToMarkup()) {
                code[obj->GetType()] += obj->GetXMLCode(show_defaults) + "\n";
            }
        }
        for (const QString & s : code) {
            code_str += s;
        }

        //closing tag
        code_str += QString("</") + t_str + QString(">");
    }

    return code_str;
}

QVariantMap RoomObject::GetJSONCode(const bool show_defaults) const
{
    QVariantMap m;
    QMap <ElementType, QVariantList> elementlistmap;

    const ElementType t = props->GetType();

    if (show_defaults || props->GetID().length() > 0) {
        m["id"] = props->GetID();
    }
    if (show_defaults || props->GetJSID().length() > 0) {
        m["js_id"] = props->GetJSID();
    }
    if (show_defaults || props->GetLocked()) {
        m["locked"] = props->GetLocked();
    }
    if (show_defaults || props->GetOnClick().length() > 0) {
        m["onclick"] = props->GetOnClick();
    }
    if (show_defaults || props->GetOnCollision().length() > 0) {
        m["oncollision"] = props->GetOnCollision();
    }
    if (show_defaults || GetInterpTime() != 0.1f) {
        m["interp_time"] = GetInterpTime();
    }
    if (show_defaults || GetPos() != QVector3D(0,0,0)) {
        m["pos"] = MathUtil::GetVectorAsString(props->GetPos()->toQVector3D(), false);
    }
    if (show_defaults || GetVel() != QVector3D(0,0,0)) {
        m["vel"] = MathUtil::GetVectorAsString(GetVel(), false);
    }
    if (show_defaults || GetAccel() != QVector3D(0,0,0)) {
        m["accel"] = MathUtil::GetVectorAsString(GetAccel(), false);
    }
    if (show_defaults || props->GetRotationOrder() != "xyz") {
        m["rotation_order"] = props->GetRotationOrder();
    }
    if (props->GetRotation()->toQVector3D() != QVector3D(0,0,0)) {
        m["rotation"] = MathUtil::GetVectorAsString(props->GetRotation()->toQVector3D(), false);
    }
    else {
        if (show_defaults || GetXDir() != QVector3D(1,0,0)) {
            m["xdir"] = MathUtil::GetVectorAsString(props->GetXDir()->toQVector3D(), false);
        }
        if (show_defaults || GetYDir() != QVector3D(0,1,0)) {
            m["ydir"] = MathUtil::GetVectorAsString(props->GetYDir()->toQVector3D(), false);
        }
        if (show_defaults || GetZDir() != QVector3D(0,0,1)) {
            m["zdir"] = MathUtil::GetVectorAsString(props->GetZDir()->toQVector3D(), false);
        }
    }
    if (show_defaults || GetScale() != QVector3D(1,1,1) || t == TYPE_GHOST) {
        m["scale"] = MathUtil::GetVectorAsString(props->GetScale()->toQVector3D(), false);
    }
    if (show_defaults || MathUtil::GetVector4AsColour(props->GetColour()->toQVector4D()) != QColor(255,255,255)) {
        m["col"] = MathUtil::GetColourAsString(MathUtil::GetVector4AsColour(props->GetColour()->toQVector4D()), false);
    }
    if (show_defaults || !props->GetLighting()) {
        m["lighting"] = props->GetLighting();
    }
    if (show_defaults || !props->GetCollisionStatic()) {
        m["collision_static"] = props->GetCollisionStatic();
    }
    if (show_defaults || props->GetCollisionTrigger()) {
        m["collision_trigger"] = props->GetCollisionTrigger();
    }
    if (show_defaults || props->GetCollisionCcdMotionThreshold() != 1.0f) {
        m["collision_ccdmotionthreshold"] = props->GetCollisionCcdMotionThreshold();
    }
    if (show_defaults || props->GetCollisionCcdSweptSphereRadius() != 0.0f) {
        m["collision_ccdsweptsphereradius"] = props->GetCollisionCcdSweptSphereRadius();
    }
    if (show_defaults || !props->GetVisible()) {
        m["visible"] = props->GetVisible();
    }
    if (show_defaults || props->GetShaderID().length() > 0) {
        m["shader_id"] = props->GetShaderID();
    }
    if (t == TYPE_PARAGRAPH && (show_defaults || MathUtil::GetVector4AsColour(props->GetBackCol()->toQVector4D()) != QColor(255,255,255))) {
        m["back_col"] = MathUtil::GetColourAsString(MathUtil::GetVector4AsColour(props->GetBackCol()->toQVector4D()), false);
    }
    if (t == TYPE_PARAGRAPH && (show_defaults || props->GetBackAlpha() != 1.0f)) {
        m["back_alpha"] = props->GetBackAlpha();
    }
    if (t == TYPE_PARAGRAPH && (show_defaults || MathUtil::GetVector4AsColour(props->GetTextCol()->toQVector4D()) != QColor(0,0,0))) {
        m["text_col"] = MathUtil::GetColourAsString(MathUtil::GetVector4AsColour(props->GetTextCol()->toQVector4D()), false);
    }
    if (t == TYPE_PARAGRAPH && (show_defaults || props->GetFontSize() != 16)) {
        m["font_size"] = props->GetFontSize();
    }
    if (t == TYPE_GHOST && (show_defaults || props->GetHeadID().length() > 0)) {
        m["head_id"] = props->GetHeadID();
    }
    if (t == TYPE_GHOST && (show_defaults || head_avatar_pos != QVector3D(0,1,0))) {
        m["head_pos"] = MathUtil::GetVectorAsString(head_avatar_pos, false);
    }
    if (t == TYPE_GHOST && (show_defaults || props->GetBodyID().length() > 0)) {
        m["body_id"] = props->GetBodyID();
    }
    if ((t == TYPE_GHOST || t == TYPE_OBJECT) && (show_defaults || props->GetAnimID().length() > 0)) {
        m["anim_id"] = props->GetAnimID();
    }
    if ((t == TYPE_GHOST || t == TYPE_OBJECT) && (show_defaults || props->GetAnimSpeed() != 1.0f)) {
        m["anim_speed"] = props->GetAnimSpeed();
    }
    if (t == TYPE_GHOST && (show_defaults || props->GetEyePos()->toQVector3D() != QVector3D(0,1.6f,0))) {
        m["eye_pos"] = MathUtil::GetVectorAsString(props->GetEyePos()->toQVector3D(), false);
    }
    if (t == TYPE_GHOST && (show_defaults || userid_pos != QVector3D(0,0,0))) {
        m["userid_pos"] = MathUtil::GetVectorAsString(userid_pos, false);
    }
    if (show_defaults || props->GetThumbID().length() > 0) {
        m["thumb_id"] = props->GetThumbID();
    }
    if (t == TYPE_LINK && (show_defaults || props->GetOriginalURL().length() > 0)) {
        m["url"] = props->GetOriginalURL();
    }
    if (t == TYPE_LINK && (show_defaults || GetTitle().length() > 0)) {
        m["title"] = props->GetTitle();
    }
    if (t == TYPE_LINK && (show_defaults || props->GetAutoLoad())) {
        m["auto_load"] = props->GetAutoLoad();
    }
    if (t == TYPE_LINK && (show_defaults || !props->GetDrawText())) {
        m["draw_text"] = props->GetDrawText();
    }
    if (t == TYPE_LINK && (show_defaults || props->GetMirror())) {
        m["mirror"] = props->GetMirror();
    }
    if (t == TYPE_LINK && (show_defaults || !props->GetActive())) {
        m["active"] = props->GetActive();
    }
    if (t == TYPE_SOUND && (show_defaults || props->GetTriggerRect()!= QRect(0,0,0,0))) {
        m["rect"] = MathUtil::GetRectangleAsString(props->GetTriggerRect(), false);
    }
    if (show_defaults || props->GetLoop()) {
        m["loop"] = props->GetLoop();
    }
    if (t != TYPE_OBJECT && (show_defaults || props->GetGain() != 1.0f)) {
        m["gain"] = props->GetGain();
    }
    if (show_defaults || props->GetDopplerFactor() != 1.0f) {
        m["doppler_factor"] = props->GetDopplerFactor();
    }
    if (t != TYPE_OBJECT && (show_defaults || props->GetPitch() != 1.0f)) {
        m["pitch"] = props->GetPitch();
    }
    if (t != TYPE_OBJECT && (show_defaults || props->GetAutoPlay())) {
        m["auto_play"] = props->GetAutoPlay();
    }
    if ((t == TYPE_OBJECT || t == TYPE_GHOST) && (show_defaults || props->GetCullFace() != "back")) {
        m["cull_face"] = props->GetCullFace();
    }
    if (t != TYPE_OBJECT && (show_defaults || props->GetPlayOnce())) {
        m["play_once"] = props->GetPlayOnce();
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetEmitterID().length() > 0)) {
        m["emitter_id"] = props->GetEmitterID();
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetEmitLocal())) {
        m["emit_local"] = props->GetEmitLocal();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetCollisionID().length() > 0)) {
        m["collision_id"] = props->GetCollisionID();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetCollisionRadius() != 0)) {
        m["collision_radius"] = props->GetCollisionRadius();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetCollisionResponse() == false)) {
        m["collision_response"] = props->GetCollisionResponse();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetWebsurfaceID().length() > 0)) {
        m["websurface_id"] = props->GetWebsurfaceID();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetTeleportID().length() > 0)) {
        m["teleport_id"] = props->GetTeleportID();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetVideoID().length() > 0)) {
        m["video_id"] = props->GetVideoID();
    }
    if ((t == TYPE_OBJECT || t == TYPE_PARTICLE) && (show_defaults || props->GetImageID().length() > 0)) {
        m["image_id"] = props->GetImageID();
    }
    if ((t == TYPE_OBJECT || t == TYPE_LIGHT) && (show_defaults || props->GetSpinAxis()->toQVector3D() != QVector3D(0,1,0))) {
        m["rotate_axis"] = MathUtil::GetVectorAsString(props->GetSpinAxis()->toQVector3D(), false);
    }
    if ((t == TYPE_OBJECT || t == TYPE_LIGHT) && (show_defaults || props->GetSpinVal() != 0)) {
        m["rotate_deg_per_sec"] = props->GetSpinVal();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetBlend0ID().length() > 0)) {
        m["blend0_id"] = props->GetBlend0ID();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetBlend1ID().length() > 0)) {
        m["blend1_id"] = props->GetBlend1ID();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetBlend2ID().length() > 0)) {
        m["blend2_id"] = props->GetBlend2ID();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetBlend3ID().length() > 0)) {
        m["blend3_id"] = props->GetBlend3ID();
    }
    if (t == TYPE_SOUND && (show_defaults || props->GetOuterGain() != 0.0f)) {
        m["outer_gain"] = props->GetOuterGain();
    }
    if (t == TYPE_SOUND && (show_defaults || props->GetInnerAngle() != 360.0f)) {
        m["inner_angle"] = props->GetInnerAngle();
    }
    if (t == TYPE_SOUND && (show_defaults || props->GetOuterAngle() != 360.0f)) {
        m["outer_angle"] =props->GetOuterAngle();
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetCount() != 0)) {
        m["count"] = props->GetCount();
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRate() != 1)) {
        m["rate"] = props->GetRate();
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetDuration() != 1.0f)) {
        m["duration"] = props->GetDuration();
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetFadeIn() != 1.0f)) {
        m["fade_in"] = props->GetFadeIn();
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetFadeOut() != 1.0f)) {
        m["fade_out"] = props->GetFadeOut();
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRandPos()->toQVector3D() != QVector3D(0,0,0))) {
        m["rand_pos"] = MathUtil::GetVectorAsString(props->GetRandPos()->toQVector3D(), false);
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRandVel()->toQVector3D() != QVector3D(0,0,0))) {
        m["rand_vel"] = MathUtil::GetVectorAsString(props->GetRandVel()->toQVector3D(), false);
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRandAccel()->toQVector3D() != QVector3D(0,0,0))) {
        m["rand_accel"] = MathUtil::GetVectorAsString(props->GetRandAccel()->toQVector3D(), false);
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRandColour()->toQVector3D() != QVector3D(0,0,0))) {
        m["rand_col"] = MathUtil::GetVectorAsString(props->GetRandColour()->toQVector3D(), false);
    }
    if (t == TYPE_PARTICLE && (show_defaults || props->GetRandScale()->toQVector3D() != QVector3D(0,0,0))) {
        m["rand_scale"] = MathUtil::GetVectorAsString(props->GetRandScale()->toQVector3D(), false);
    }
    if (t == TYPE_OBJECT && (show_defaults || m_cubemap_radiance)) {
        if (m_cubemap_radiance) {
            m["cubemap_radiance_id"] = m_cubemap_radiance->GetProperties()->GetID();
        }
    }
    if (t == TYPE_OBJECT && (show_defaults || m_cubemap_irradiance)) {
        if (m_cubemap_irradiance) {
            m["cubemap_irradiance_id"] = m_cubemap_irradiance->GetProperties()->GetID();
        }
    }
    if (t == TYPE_LIGHT) {
        m["light_intensity"] = props->GetLightIntensity();
    }
    if (t == TYPE_LIGHT) {
        m["light_cone_angle"] = props->GetLightConeAngle();
    }
    if (t == TYPE_LIGHT) {
        m["light_cone_exponent"] = props->GetLightConeExponent();
    }
    if (t == TYPE_LIGHT) {
        m["light_range"] = props->GetLightRange();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetCollisionPos()->toQVector3D() != QVector3D(0,0,0))) {
        m["collision_pos"] = MathUtil::GetVectorAsString(props->GetCollisionPos()->toQVector3D(), false);
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetCollisionScale()->toQVector3D() != QVector3D(1,1,1))) {
        m["collision_scale"] = MathUtil::GetVectorAsString(props->GetCollisionScale()->toQVector3D(), false);
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionFriction() != 0.5f)) {
        m["collision_friction"] = props->GetCollisionFriction();
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionRollingFriction() != 0.01f)) {
        m["collision_rollingfriction"] = props->GetCollisionRollingFriction();
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionRestitution() != 0.85f)) {
        m["collision_restitution"] = props->GetCollisionRestitution();
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionAngularDamping() != 0.1f)) {
        m["collision_angulardamping"] = props->GetCollisionAngularDamping();
    }
    if (t == TYPE_OBJECT && ((show_defaults) || props->GetCollisionLinearDamping() != 0.15f)) {
        m["collision_lineardamping"] = props->GetCollisionLinearDamping();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetBoneID().length() > 0)) {
        m["bone_id"] = props->GetBoneID();
    }
    if (t == TYPE_OBJECT && (show_defaults || props->GetDrawLayer() != 0)) {
        m["draw_layer"] = props->GetDrawLayer();
    }
    if (!props->GetLightmapID().isEmpty()) {
        m["lmap_id"] = props->GetLightmapID();
    }
    if (props->GetLightmapScale()->toQVector4D() != QVector4D(1,1,0,0)) {
        m["lmap_sca"] = props->GetLightmapScale()->toQVector4D();
    }

    //add text stuff if there is any in the middle
    switch (t) {
    case TYPE_TEXT:
    case TYPE_PARAGRAPH:
        m["innertext"] = GetText();
        break;
    default:
        break;
    }

    //We have to add this node's children
    if (!child_objects.empty()) {
        //Add in all my child's tags recursively
        for (int i=0; i<child_objects.size(); ++i) {
            if (child_objects[i]) {
                elementlistmap[child_objects[i]->GetType()].push_back(child_objects[i]->GetJSONCode(show_defaults));
            }
        }
    }

    QMap <ElementType, QVariantList>::const_iterator ele_cit;
    for (ele_cit = elementlistmap.begin(); ele_cit != elementlistmap.end(); ++ele_cit) {
        m.insert(DOMNode::ElementTypeToString(ele_cit.key()), ele_cit.value());
    }

    return m;
}

void RoomObject::SetURL(const QString & base, const QString & s)
{
    //qDebug() << "RoomObject::SetURL" << this << base << s;
    props->SetBaseURL(base);
    props->SetURL(s);

    //do extra stuff - if this is a link, has an attached websurface, etc.
    if (GetType() == TYPE_LINK) {
        if (QString::compare(s, "workspaces") != 0 && QString::compare(s, "bookmarks") != 0) {
            //qDebug() << "RoomObject::SetURL setting url" << base << s;
            QUrl base_url(base);
            QString resolved_url = QUrl::fromPercentEncoding(base_url.resolved(s).toString().toLatin1());
            props->SetURL(resolved_url);
            if (props->GetOriginalURL().isEmpty()) {
                props->SetOriginalURL(resolved_url);
            }
        }
        textgeom_url->SetText(ShortenString(props->GetURL()));
    }
    else {
        if (props->GetWebsurfaceID().length() > 0 && assetwebsurface && assetwebsurface->GetProperties()->GetSaveToMarkup()) {
            assetwebsurface->SetURL(s);
        }
    }
}

void RoomObject::SetURL(const QString & s)
{
    SetURL("", s);
}

QString RoomObject::GetURL() const
{
    return props->GetURL();
}

void RoomObject::SetTitle(const QString & s)
{
    props->SetTitle(s);
    textgeom_title->SetText(ShortenString(s));
}

QString RoomObject::GetTitle() const
{
    return props->GetTitle();
}

QPointer <TextGeom> RoomObject::GetTextGeomURL()
{
    return textgeom_url;
}

QPointer <TextGeom> RoomObject::GetTextGeomTitle()
{
    return textgeom_title;
}

void RoomObject::DrawPortalGL(QPointer <AssetShader> shader)
{
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }

    const QVector3D scale = GetScale();
    const float w = scale.x() * 0.5f;
    const float h = scale.y() * 0.5f;

    MathUtil::PushModelMatrix();
    MathUtil::MultModelMatrix(model_matrix_local);

    MathUtil::ModelMatrix().translate(0,h,0);
    MathUtil::ModelMatrix().scale(w, h, 1.0f);

    shader->SetUseTextureAll(false);    
    shader->SetUseLighting(false);

    if (selected) {
        shader->SetConstColour(QVector4D(0.5f, 1.0f, 0.5f, 0.5f));

        shader->UpdateObjectUniforms();

        Renderer * renderer = Renderer::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                renderer->GetTexturedCubePrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetTexturedCubeVAO(),
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
    }

    shader->SetUseTexture(0, true);
    shader->SetConstColour(QVector4D(1,1,1,1));
    shader->SetUseLighting(props->GetLighting());

    if (!props->GetOpen()) {
        DrawPortalInsideGL(shader);
    }
    if (draw_back) {
        DrawPortalBackGL(shader);
    }

    MathUtil::PopModelMatrix();
}


void RoomObject::DrawPortalDecorationsGL(QPointer <AssetShader> shader, const float anim_val)
{
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }

    const QVector3D s = GetScale();
    const float w = s.x() * 0.5f;
    const float h = s.y() * 0.5f;

    MathUtil::PushModelMatrix();
    MathUtil::MultModelMatrix(model_matrix_local);    
    MathUtil::ModelMatrix().translate(0, h, 0);
    MathUtil::ModelMatrix().scale(w, h, 1.0f);
    MathUtil::ModelMatrix().translate(0,0,portal_spacing);

    if (props->GetDrawText() && portal_text && portal_text->GetTextGeomTitle()) {
        shader->SetUseTexture(0, true);
        shader->SetConstColour(QVector4D(1,1,1,1));

        MathUtil::PushModelMatrix();
        MathUtil::ModelMatrix().translate(0, 0.9f, 0);
        MathUtil::ModelMatrix().scale(1.0f, s.x() / s.y(), 1.0f);
        portal_text->DrawGL(shader, true, QVector3D(0,0,0));
        MathUtil::PopModelMatrix();
        shader->SetUseTexture(0, false);
    }

    if (anim_val != 1.0f) {
        MathUtil::ModelMatrix().translate(0,0,0.1f);
        MathUtil::ModelMatrix().scale(1.0f/s.x(), 1.0f/s.y(), 1.0f);       
        SpinAnimation::DrawGL(shader, anim_val, false);

        shader->SetUseTextureAll(false);
    }

    MathUtil::PopModelMatrix();
}

void RoomObject::DrawPortalBackGL(QPointer <AssetShader> shader) const
{
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }   

    const bool use_thumb = (portal_thumb_img && portal_thumb_img->GetFinished() && !portal_thumb_img->GetError() && portal_thumb_img->GetTextureHandle(true) != nullptr);
    //qDebug() << "RoomObject::DrawPortalBackGL" << this->GetURL() << thumb_image;

    shader->SetUseLighting(false);
    shader->SetUseTextureAll(false);
    shader->SetUseTexture(0, use_thumb);

    const QColor col = MathUtil::GetVector4AsColour(props->GetColour()->toQVector4D());
    if (use_thumb) {
        const float max_col = qMax(qMax(col.redF(), col.greenF()), col.blueF());
        shader->SetConstColour(QVector4D(max_col, max_col, max_col, 1));
        Renderer::m_pimpl->SetDepthMask(DepthMask::DEPTH_WRITES_ENABLED);
        Renderer::m_pimpl->BindTextureHandle(0, portal_thumb_img->GetTextureHandle(true));
    }
    else {
        shader->SetConstColour(QVector4D(col.redF(), col.greenF(), col.blueF(), 0.5f));
    }

    MathUtil::PushModelMatrix();
    MathUtil::ModelMatrix().scale(-1.0f, 1.0f, -1.0f); //scales and does 180 rotation

    shader->UpdateObjectUniforms();
    if (props->GetCircular()) {
        Renderer * renderer = Renderer::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLE_FAN,
                                renderer->GetDiscPrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetDiscVAO(),
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
    }
    else {        
        Renderer * renderer = Renderer::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                renderer->GetPlanePrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetPlaneVAO(),
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
    }
    Renderer::m_pimpl->SetDepthMask(DepthMask::DEPTH_WRITES_DISABLED);
    MathUtil::PopModelMatrix();
}

void RoomObject::DrawPortalInsideGL(QPointer <AssetShader> shader) const
{
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }      

    //qDebug() << "EnvObject::DrawPortalInsideGL" << this->GetURL() << thumb_image << GetColour();
    const bool use_thumb = (portal_thumb_img && portal_thumb_img->GetFinished() && !portal_thumb_img->GetError() && portal_thumb_img->GetTextureHandle(true) != nullptr);
    QColor col = MathUtil::GetVector4AsColour(props->GetColour()->toQVector4D());

    if (col == QColor(0,0,0,0) || col == QColor(0,0,0,255)) {
        col = QColor(255,255,255,255);
    }
    if (!props->GetHighlighted()) {
        col = col.darker(150);
    }

    shader->SetUseLighting(false);
    shader->SetUseTexture(0, use_thumb);

    if (use_thumb) {
        const float max_col = qMax(qMax(col.redF(), col.greenF()), col.blueF());
        shader->SetConstColour(QVector4D(max_col, max_col, max_col, 1));
        Renderer::m_pimpl->SetDepthMask(DepthMask::DEPTH_WRITES_ENABLED);
        Renderer::m_pimpl->BindTextureHandle(0, portal_thumb_img->GetTextureHandle(true));
    }
    else {
        shader->SetConstColour(QVector4D(col.redF(), col.greenF(), col.blueF(), 0.5f));
    }

    MathUtil::PushModelMatrix();

    shader->UpdateObjectUniforms();        
    if (props->GetCircular()) {
        Renderer * renderer = Renderer::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLE_FAN,
                                renderer->GetDiscPrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetDiscVAO(),
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
    }
    else {
        Renderer * renderer = Renderer::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                renderer->GetPlanePrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetPlaneVAO(),
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
    }
    Renderer::m_pimpl->SetDepthMask(DepthMask::DEPTH_WRITES_DISABLED);
    MathUtil::PopModelMatrix();
}

void RoomObject::DrawPortalFrameGL(QPointer <AssetShader> shader)
{
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }

    //qDebug() << "RoomObject::DrawPortalFrameGL" << linear_gradient_img;
    if (linear_gradient_img.isNull()) {
        return;
    }   

    const QColor col = MathUtil::GetVector4AsColour(props->GetColour()->toQVector4D());
    auto tex_id = linear_gradient_img->GetTextureHandle(true);
    Renderer::m_pimpl->BindTextureHandle(1, tex_id);

    shader->SetConstColour(QVector4D(col.redF(), col.greenF(), col.blueF(), 1));

    MathUtil::PushModelMatrix();

    if (props->GetCircular()) {
        MathUtil::ModelMatrix().scale(1.0f, 1.0f, !props->GetOpen() ? 0.2f : 0.1f);
        shader->UpdateObjectUniforms();
        Renderer * renderer = Renderer::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                renderer->GetConePrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetConeVAO(),
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

        MathUtil::ModelMatrix().scale(1.0f, 1.0f, 1.5f);
        shader->UpdateObjectUniforms();
        AbstractRenderCommand a2(PrimitiveType::TRIANGLES,
                                 renderer->GetConePrimCount(),
                                 0,
                                 0,
                                 0,
                                 renderer->GetConeVAO(),
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
        renderer->PushAbstractRenderCommand(a2);

        MathUtil::ModelMatrix().scale(1.0f, 1.0f, 1.5f);
        shader->UpdateObjectUniforms();
        AbstractRenderCommand a3(PrimitiveType::TRIANGLES,
                                 renderer->GetConePrimCount(),
                                 0,
                                 0,
                                 0,
                                 renderer->GetConeVAO(),
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
        renderer->PushAbstractRenderCommand(a3);
    }
    else {
        MathUtil::ModelMatrix().scale(1.0f, 1.0f, 0.1f);
        shader->UpdateObjectUniforms();
        Renderer * renderer = Renderer::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                renderer->GetPyramidPrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetPyramidVAO(),
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

        MathUtil::ModelMatrix().scale(1.0f, 1.0f, 1.5f);
        shader->UpdateObjectUniforms();
        AbstractRenderCommand a2(PrimitiveType::TRIANGLES,
                                 renderer->GetPyramidPrimCount(),
                                 0,
                                 0,
                                 0,
                                 renderer->GetPyramidVAO(),
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
        renderer->PushAbstractRenderCommand(a2);

        MathUtil::ModelMatrix().scale(1.0f, 1.0f, 1.5f);
        shader->UpdateObjectUniforms();
        AbstractRenderCommand a3(PrimitiveType::TRIANGLES,
                                 renderer->GetPyramidPrimCount(),
                                 0,
                                 0,
                                 0,
                                 renderer->GetPyramidVAO(),
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
        renderer->PushAbstractRenderCommand(a3);
    }

    MathUtil::PopModelMatrix();

}

void RoomObject::DrawPortalStencilGL(QPointer <AssetShader> shader, const bool draw_offset_stencil) const
{
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }

    QMatrix4x4 m = model_matrix_local;

    const float w = GetScale().x() * 0.5f;
    const float h = GetScale().y() * 0.5f;
    m.translate(0, h, 0);
    m.scale(w, h, 1.0f);

    shader->SetUseTextureAll(false);
    shader->SetUseLighting(false);
    shader->SetUseSkelAnim(false); //53.11 - TODO: find out why is this necessary?
    shader->SetConstColour(QVector4D(0,0,0,1));

    MathUtil::PushModelMatrix();
    MathUtil::MultModelMatrix(m);

    shader->UpdateObjectUniforms();   

    if (props->GetCircular()) {
        Renderer * renderer = Renderer::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLE_FAN,
                                renderer->GetDiscPrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetDiscVAO(),
                                shader->GetProgramHandle(),
                                shader->GetFrameUniforms(),
                                shader->GetRoomUniforms(),
                                shader->GetObjectUniforms(),
                                shader->GetMaterialUniforms(),
                                renderer->GetCurrentlyBoundTextures(),
                                FaceCullMode::BACK,
                                DepthFunc::LEQUAL,
                                renderer->GetDepthMask(),
                                renderer->GetStencilFunc(),
                                renderer->GetStencilOp(),
                                renderer->GetColorMask());
        renderer->PushAbstractRenderCommand(a);
        if (draw_offset_stencil)
        {
            Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::DISABLED);
            Renderer * renderer = Renderer::m_pimpl;
            AbstractRenderCommand a2(PrimitiveType::TRIANGLES,
                                     renderer->GetPortalStencilCylinderPrimCount(),
                                     0,
                                     0,
                                     0,
                                     renderer->GetPortalStencilCylinderVAO(),
                                     shader->GetProgramHandle(),
                                     shader->GetFrameUniforms(),
                                     shader->GetRoomUniforms(),
                                     shader->GetObjectUniforms(),
                                     shader->GetMaterialUniforms(),
                                     renderer->GetCurrentlyBoundTextures(),
                                     FaceCullMode::DISABLED,
                                     DepthFunc::LEQUAL,
                                     renderer->GetDepthMask(),
                                     renderer->GetStencilFunc(),
                                     renderer->GetStencilOp(),                                     
                                     renderer->GetColorMask());
            renderer->PushAbstractRenderCommand(a2);
            Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::BACK);
        }
    }
    else
    {
        Renderer * renderer = Renderer::m_pimpl;
        AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                renderer->GetPlanePrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetPlaneVAO(),
                                shader->GetProgramHandle(),
                                shader->GetFrameUniforms(),
                                shader->GetRoomUniforms(),
                                shader->GetObjectUniforms(),
                                shader->GetMaterialUniforms(),
                                renderer->GetCurrentlyBoundTextures(),
                                FaceCullMode::BACK,
                                DepthFunc::LEQUAL,
                                renderer->GetDepthMask(),
                                renderer->GetStencilFunc(),
                                renderer->GetStencilOp(),                                
                                renderer->GetColorMask());
        renderer->PushAbstractRenderCommand(a);
        if (draw_offset_stencil)
        {
            Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::DISABLED);
            Renderer * renderer = Renderer::m_pimpl;
            AbstractRenderCommand a2(PrimitiveType::TRIANGLES,
                                     renderer->GetPortalStencilCubePrimCount(),
                                     0,
                                     0,
                                     0,
                                     renderer->GetPortalStencilCubeVAO(),
                                     shader->GetProgramHandle(),
                                     shader->GetFrameUniforms(),
                                     shader->GetRoomUniforms(),
                                     shader->GetObjectUniforms(),
                                     shader->GetMaterialUniforms(),
                                     renderer->GetCurrentlyBoundTextures(),
                                     FaceCullMode::DISABLED,
                                     DepthFunc::LEQUAL,
                                     renderer->GetDepthMask(),
                                     renderer->GetStencilFunc(),
                                     renderer->GetStencilOp(),                                     
                                     renderer->GetColorMask());
            renderer->PushAbstractRenderCommand(a2);
            Renderer::m_pimpl->SetDefaultFaceCullMode(FaceCullMode::BACK);
        }
    }

    MathUtil::PopModelMatrix();
    shader->SetConstColour(QVector4D(1,1,1,1));
}

bool RoomObject::Clicked(const QVector3D & p)
{
    const QVector3D p_local = GetLocal(p);

    const ElementType t = GetType();

    if (t == TYPE_VIDEO) {
        if (assetvideo) {
            if (fabsf(p_local.x()) < 1.0f && fabsf(p_local.y()) < assetvideo->GetAspectRatio(&media_ctx) && p_local.z() > 0.0f && p_local.z() < 1.0f) {
                return true;
            }
        }
    }
    else if (t == TYPE_OBJECT) {
        if (assetobject) {
            const QVector3D bbmin = assetobject->GetBBoxMin() - QVector3D(0.1f, 0.1f, 0.1f);
            const QVector3D bbmax = assetobject->GetBBoxMax() + QVector3D(0.1f, 0.1f, 0.1f);
            if (p_local.x() > bbmin.x() && p_local.x() < bbmax.x() &&
                    p_local.y() > bbmin.y() && p_local.y() < bbmax.y() &&
                    p_local.z() > bbmin.z() && p_local.z() < bbmax.z()) {
                return true;
            }
        }
    }
    else if (t == TYPE_GHOST) {
        if (fabsf(p_local.x()) < 1.0f && fabsf(p_local.y()) < 2.0f && fabsf(p_local.z()) < 1.0f) {
            return true;
        }
    }
    else if (t == TYPE_IMAGE) {
        if (assetimage) {
            if (fabsf(p_local.x()) < 1.0f && fabsf(p_local.y()) < assetimage->GetAspectRatio() && p_local.z() > 0.0f && p_local.z() < 1.0f) {
                return true;
            }
        }
    }   

    return false;
}

QVector3D RoomObject::GetLocal(const QVector3D & pos) const
{
    const QVector3D p = GetPos();
    const QVector3D x = GetXDir();
    const QVector3D y = GetYDir();
    const QVector3D z = GetZDir();
    const QVector3D s = GetScale();
    const QVector3D d = (pos - (p + z * RoomObject::portal_spacing));
    return QVector3D(QVector3D::dotProduct(d, x) / s.x(),
                     QVector3D::dotProduct(d, y) / s.y(),
                     QVector3D::dotProduct(d, z) / s.z());
}

QVector3D RoomObject::GetGlobal(const QVector3D & pos) const
{
    const QVector3D p = GetPos();
    const QVector3D x = GetXDir();
    const QVector3D y = GetYDir();
    const QVector3D z = GetZDir();
    const QVector3D s = GetScale();
    return pos.x() * x * s.x() +
            pos.y() * y * s.y() +
            pos.z() * z * s.z() +
            p + z * RoomObject::portal_spacing;
}

int RoomObject::GetNumTris() const
{
    const ElementType t = GetType();
    if (t == TYPE_TEXT || t == TYPE_PARAGRAPH) {
        return 2;
    }
    else if (t == TYPE_IMAGE || t == TYPE_VIDEO) {
        return 12;
    }
    else if (t == TYPE_LINK) {
        return 4;
    }
    else if (t == TYPE_OBJECT) {
        if (assetobject) {
            return assetobject->GetNumTris();
        }
        else {
            return 0;
        }
    }
    else if (t == TYPE_GHOST) {
        int total = 0;        
        for (const QPointer <AssetObject> & a: ghost_assetobjs) {
            if (a) {
                total += a->GetNumTris();
            }
        }
        return total;
    }

    return 0;
}

void RoomObject::SetInterpTime(const float f)
{
    interp_time = f;
}

float RoomObject::GetInterpTime() const
{
    return interp_time;
}

void RoomObject::SetDirty(const bool val)
{
    if (props) {
        props->SetDirty(val);
    }
}

bool RoomObject::IsDirty() const
{
    if (props) {
        return props->IsDirty();
    }
    return false;
}

QPointer <RoomObject> RoomObject::CreateText(const QString & js_id, const float fixed_size, const QString & text, const QColor col, QVector3D scale)
{
    QPointer <RoomObject> obj(new RoomObject());
    obj->SetType(TYPE_TEXT);
    obj->GetProperties()->SetJSID(js_id);
    obj->SetFixedSize(true, fixed_size);
    obj->SetText(text);
    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(col));
    obj->GetProperties()->SetScale(scale);
    return obj;
}


QPointer <RoomObject> RoomObject::CreateImage(const QString & js_id, const QString & id, const QColor col, const bool lighting)
{
    QPointer <RoomObject> obj(new RoomObject());
    obj->SetType(TYPE_IMAGE);
    obj->GetProperties()->SetJSID(js_id);
    obj->GetProperties()->SetID(id);
    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(col));
    obj->GetProperties()->SetLighting(lighting);
    return obj;
}

QPointer <RoomObject> RoomObject::CreateObject(const QString & js_id, const QString & id, const QColor col, const bool lighting)
{
    QPointer <RoomObject> obj(new RoomObject());
    obj->SetType(TYPE_OBJECT);
    obj->GetProperties()->SetJSID(js_id);
    obj->GetProperties()->SetID(id);
    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(col));
    obj->GetProperties()->SetLighting(lighting);
    return obj;
}

QPointer <RoomObject> RoomObject::CreateParagraph(const QString & js_id, const QString & id, const QString & text, const QColor col, const bool lighting)
{
    QPointer <RoomObject> obj(new RoomObject());
    obj->SetType(TYPE_PARAGRAPH);
    obj->GetProperties()->SetJSID(js_id);
    obj->GetProperties()->SetID(id);
    obj->SetText(text);
    obj->GetProperties()->SetColour(MathUtil::GetColourAsVector4(col));
    obj->GetProperties()->SetLighting(lighting);
    return obj;
}

void RoomObject::PlayCreateObject()
{
    switch (qrand() % 3) {
    case 0:
        SoundManager::Play(SOUND_SELECTOBJECT, false, GetPos(), 1.0f);
        break;
    case 1:
        SoundManager::Play(SOUND_SELECTIMAGE, false, GetPos(), 1.0f);
        break;
    default:
        SoundManager::Play(SOUND_NEWTEXT, false, GetPos(), 1.0f);
        break;
    }
}

void RoomObject::PlayCreatePortal()
{
    SoundManager::Play(SOUND_NEWENTRANCE, false, GetPos(), 1.0f);
}

void RoomObject::PlayDeleteObject()
{
    SoundManager::Play(SOUND_DELETING, false, GetPos(), 1.0f);
}

float RoomObject::GetTimeElapsed() const
{
    return time_elapsed;
}

void RoomObject::LoadGhost_Helper(const int depth, const QVariantMap & d, QPointer <RoomObject> parent_object, QHash <QString, QPointer <AssetObject> > & asset_obj_list,  QHash <QString, QPointer <AssetShader> > & asset_shader_list)
{    
    //qDebug() << "RoomObject::LoadGhost_Helper() - GOT HERE1" << this->GetJSID() << this->GetID();
    if (depth >= 32) {
        return;
    }

    if (d.contains("assetobject")) {
        QVariantList l = d["assetobject"].toList();
        for (int i=0; i<l.size(); ++i) {
            QVariantMap a = l[i].toMap();
            QPointer <AssetObject> new_asset_obj(new AssetObject());
            new_asset_obj->SetSrc("", a["src"].toString());
            new_asset_obj->SetProperties(a);
            new_asset_obj->Load();
            asset_obj_list[a["id"].toString()] = new_asset_obj;
        }
    }

    if (d.contains("assetshader"))
    {
        QVariantList l = d["assetshader"].toList();
        for (int i=0; i<l.size(); ++i)
        {
            QVariantMap a = l[i].toMap();
            QPointer <AssetShader> new_asset_shader(new AssetShader());
            new_asset_shader->SetSrc("", a["src"].toString(), "");
            new_asset_shader->SetProperties(a);
            new_asset_shader->Load();
            asset_shader_list[a["id"].toString()] = new_asset_shader;
        }
    }

    if (d.contains("ghost")) {
        QVariantList l = d["ghost"].toList();
        for (int i=0; i<l.size(); ++i) {
            QVariantMap o = l[i].toMap();
            SetType(TYPE_GHOST);
            SetProperties(o);
            LoadGhost_Helper(depth+1, o, this, asset_obj_list, asset_shader_list);
        }
    }

    if (d.contains("object")) {
        QVariantList l = d["object"].toList();
        for (int i=0; i<l.size(); ++i) {
            QVariantMap o = l[i].toMap();

            QPointer <RoomObject> new_thing(new RoomObject());
            new_thing->SetType(TYPE_OBJECT);
            new_thing->SetProperties(o);

            //make sure we can link this Object to an AssetObject
            if (asset_obj_list.contains(o["id"].toString())) {
                new_thing->SetAssetObject(asset_obj_list[o["id"].toString()]);
            }

            LoadGhost_Helper(depth+1, o, new_thing, asset_obj_list, asset_shader_list);

            if (parent_object) {
                parent_object->GetChildObjects().push_back(new_thing);
            }
        }
    }

    if (d.contains("assets")) {
        LoadGhost_Helper(depth+1, d["assets"].toMap(), parent_object, asset_obj_list, asset_shader_list);
    }
    if (d.contains("room")) {
        LoadGhost_Helper(depth+1, d["room"].toMap(), parent_object, asset_obj_list, asset_shader_list);
    }
    if (d.contains("FireBoxRoom")) {
        LoadGhost_Helper(depth+1, d["FireBoxRoom"].toMap(), parent_object, asset_obj_list, asset_shader_list);
    }
}

void RoomObject::LoadGhost(const QString & data)
{
    //qDebug() << "RoomObject::LoadGhost" << data;
    //return;
    child_objects.clear(); //54.8 - prevent userid.txt from filling up with child objects

    HTMLPage avatar_page;
    avatar_page.ReadXMLContent(data);

    QHash <QString, QPointer <AssetObject> > asset_obj_list;
    QHash <QString, QPointer <AssetShader> > asset_shader_list;
    QPointer <RoomObject> root_object(new RoomObject());

    LoadGhost_Helper(0, avatar_page.GetData(), root_object, asset_obj_list, asset_shader_list);

    SetGhostAssetObjects(asset_obj_list);
    SetGhostAssetShaders(asset_shader_list);

    root_object->GetChildObjects().clear();
    delete root_object;
}

void RoomObject::DoGhostMoved(const QVariantMap & m)
{
    const QString userid = m["userId"].toString();
    const QString roomid = m["roomId"].toString();    
    GhostFrame frame0;
    GhostFrame frame1;

    //qDebug() << "RoomObject::DoGhostMoved doing ghost moved" << userid << roomid << GetURL();
    time_elapsed = 0.0f;   

    //Set sample rate
    if (m.contains("sample_rate")) {
        sound_buffers_sample_rate = m["sample_rate"].toUInt();
    }

    AssetGhost::ConvertPacketToFrame(m, frame1);
    frame1.time_sec = rate;

    QVector <GhostFrame> frames;

    if (assetghost.isNull() || QString::compare(GetURL(), roomid) != 0) { //if no ghost (new player), or this player changed URLs
        if (assetghost.isNull()) {
            assetghost = new AssetGhost();
        }

        //set first and last frames to be the same (current packet)
        props->SetPos(frame1.pos);
        SetDir(frame1.dir);

        ghost_frame = frame1;

        //49.12 - prevents interpolation between positions (immediately moves to next place)
        frame0 = frame1;
        frame0.time_sec = 0.0f;

        frames.push_back(frame0);
        frames.push_back(frame1);                        
    }
    else {
        //set new frames                
        frame0 = ghost_frame;
        frame0.time_sec = 0.0f;       
        frames.push_back(frame0);
        frames.push_back(frame1);        
    }

    if (assetghost) {
        assetghost->SetFromFrames(frames, rate);
    }

    ghost_frame_index = -1;

    SetURL(roomid);
}

QList <RoomObjectEdit> & RoomObject::GetRoomEditsIncoming()
{
    return room_edits_incoming;
}

QList <RoomObjectEdit> & RoomObject::GetRoomDeletesIncoming()
{
    return room_deletes_incoming;
}

QList <QString> & RoomObject::GetSendPortalURL()
{
    return send_portal_url;
}

QList <QString> & RoomObject::GetSendPortalJSID()
{
    return send_portal_jsid;
}

QList <QVector3D> & RoomObject::GetSendPortalPos()
{
    return send_portal_pos;
}

QList <QVector3D> & RoomObject::GetSendPortalFwd()
{
    return send_portal_fwd;
}

void RoomObject::SetPlayerInRoom(const bool b)
{
    player_in_room = b;
}

void RoomObject::SetPlayerInAdjacentRoom(const bool b)
{
    player_in_adjacent_room = b;
}

void RoomObject::SetRescaleOnLoad(const bool b)
{
    rescale_on_load = b;
}

void RoomObject::SetRescaleOnLoadAspect(const bool b)
{
    rescale_on_load_aspect = b;
}

bool RoomObject::GetRescaleOnLoad() const
{
    return rescale_on_load;
}

void RoomObject::SetParentObject(QPointer <RoomObject> p)
{
    parent_object = p;
}

void RoomObject::AppendChild(QPointer <RoomObject> child)
{
    //qDebug()<<"RoomObject::AppendChild() - Appended"<<child->GetJSID()<<"to"<<GetJSID();
    child_objects.push_back(child);
    child->SetParentObject(this);
}

void RoomObject::RemoveChildByJSID(QString jsid)
{
    for (int i=0; i<child_objects.size(); ++i)
    {
        if (child_objects[i]->GetProperties()->GetJSID() == jsid)
        {
            child_objects[i]->SetParentObject(NULL);
            child_objects.removeAt(i);
            return;
        }
    }
}

void RoomObject::GetLight(LightContainer* p_container, QMatrix4x4* p_model_matrix)
{
    if (GetType() == TYPE_LIGHT) {
        float light_cone_angle = props->GetLightConeAngle();
        if (light_cone_angle == -1.0f) {
            return; // Skip over when light is disabled
        }
        float xScale = p_model_matrix->column(0).length();
        float yScale = p_model_matrix->column(1).length();
        float zScale = p_model_matrix->column(2).length();
        float averageScale = (xScale + yScale + zScale) / 3;

        QColor color = MathUtil::GetVector4AsColour(props->GetColour()->toQVector4D());
        QVector3D light_intensity_color(color.redF(), color.greenF(), color.blueF());
        light_intensity_color *= props->GetLightIntensity();
        light_intensity_color *= averageScale * averageScale;
        if (light_intensity_color == QVector3D(0.0f, 0.0f, 0.0f))
        {
            return; // Skip over when light has no intensity
        }

        float light_range = props->GetLightRange() * averageScale;
        if (light_range == 0.0f)
        {
            return; // Skip over when light has no range
        }
        light_range *= light_range; // We square this to simplify the shader code

        // Get room-space position
        QVector3D light_pos = GetPos();
        // Apply roomMatrix to bring it into world-space for lighting
        light_pos = (MathUtil::RoomMatrix() * (*p_model_matrix) * QVector4D(light_pos.x(), light_pos.y(), light_pos.z(), 1)).toVector3D();

        // Get room-space direction
        QVector3D light_direction = GetModelMatrixLocal().column(2).toVector3D().normalized();

        // Calculate rotation only matrix to apply to it to bring it into world-space
        QMatrix4x4 room_matrix = MathUtil::RoomMatrix();
        room_matrix.setColumn(0, room_matrix.column(0).normalized());
        room_matrix.setColumn(1, room_matrix.column(1).normalized());
        room_matrix.setColumn(2, room_matrix.column(2).normalized());
        room_matrix.setColumn(3, QVector4D(0.0, 0.0, 0.0, 0.0));

        QMatrix4x4 model_matrix_normalized = (*p_model_matrix);
        model_matrix_normalized.setColumn(0, model_matrix_normalized.column(0).normalized());
        model_matrix_normalized.setColumn(1, model_matrix_normalized.column(1).normalized());
        model_matrix_normalized.setColumn(2, model_matrix_normalized.column(2).normalized());
        model_matrix_normalized.setColumn(3, QVector4D(0.0, 0.0, 0.0, 0.0));

        QMatrix4x4 rotation_matrix = room_matrix * model_matrix_normalized;
        light_direction = (rotation_matrix * QVector4D(light_direction, 0.0f)).toVector3D();

        float light_exponent = props->GetLightConeExponent();
        p_container->m_lights.push_back(Light(light_intensity_color, light_cone_angle, light_pos, light_range, light_direction, light_exponent));
    }
}

void RoomObject::GetLights(LightContainer* p_container, QMatrix4x4* p_model_matrix)
{
    // Add our own Light to the container if we are of type light
    GetLight(p_container, p_model_matrix);
    QMatrix4x4 prev_model_matrix = *p_model_matrix;
    (*p_model_matrix) *= GetModelMatrixLocal();

    // Recusively call GetLights on any child RoomObjects with our model matrix applied
    QList<QPointer<RoomObject>> children = GetChildObjects();
    auto children_itr = children.begin();
    auto children_end = children.end();
    for (;children_itr != children_end; children_itr++)
    {
        if ((*children_itr)) {
            (*children_itr)->GetLights(p_container, p_model_matrix);
        }
    }

    // Restore model matrix back to its previous state
    (*p_model_matrix) = prev_model_matrix;
}

void RoomObject::SetDrawAssetObjectTeleport(const bool b)
{
    draw_assetobject_teleport = b;
}

bool RoomObject::GetDrawAssetObjectTeleport()
{
    return draw_assetobject_teleport;
}

float RoomObject::GetRate()
{
    return rate;
}

float RoomObject::GetLogoffRate()
{
    return logoff_rate;
}

void RoomObject::DrawGhostUserIDChat(QPointer <AssetShader> shader)
{
    //compute dialog dimensions
    const float message_duration_msec = 7000.0f;
    const float row_height = 0.16f;
    float width = textgeom_player_id->GetScale()*(textgeom_player_id->GetText().length()+2.0f);
    float height = row_height;

    if (!textgeom_chatmessages.isEmpty()) {
        height += row_height*0.5f; //space between userid and text msgs
    }

    for (int i=0; i<textgeom_chatmessages.size(); ++i) {
        const float interp = (chat_message_times[i].elapsed() + chat_message_time_offsets[i] * 1000.0f)/message_duration_msec;

        if (interp > 0.0f && interp < 1.0f) {
            const QVector <TextGeomLine> texts = textgeom_chatmessages[i]->GetAllText();
            height += float(texts.size()) * row_height + row_height * 0.25f;
            for (int j=0; j<texts.size(); ++j) {
                width = qMax(width, textgeom_player_id->GetScale()*float(texts[j].text.length()+2.0f));
            }
        }
    }

    //draw chat messages
    const QString body_id = props->GetBodyID();
    const QString head_id = props->GetHeadID();
    const bool custom_avatar = (SettingsManager::GetUpdateCustomAvatars() && (!body_id.isEmpty() || !head_id.isEmpty()));

    const QVector3D s = custom_avatar ? GetScale() : QVector3D(1.4f, 1.4f, 1.4f);
    MathUtil::PushModelMatrix();
    MathUtil::ModelMatrix().scale(1.0f/s.x(), 1.0f/s.y(), 1.0f/s.z());
    MathUtil::ModelMatrix().translate(userid_pos + QVector3D(0,2.0f,0));

    MathUtil::BillboardModelMatrix();

    //draw grey box
    MathUtil::PushModelMatrix();
    MathUtil::ModelMatrix().scale(width, height, 1);

    QColor col;
    if (ghost_frame.speaking) {
        const int v = qMin(255, int(512.0f*ghost_frame.current_sound_level));
        col = QColor(0,v,0,128);
    }
    else if (ghost_frame.typing) {
        col = QColor(128,128,0,128);
    }
    else if (ghost_frame.editing) {
        col = QColor(128,0,128,128);
    }
    else if (ghost_frame.hands.first.is_active || ghost_frame.hands.second.is_active) {
        col = QColor(0,0,128,128);
    }
    else {
        col = QColor(0,0,0,128);
    }
    SpinAnimation::DrawPlaneGL(shader, col);
    MathUtil::PopModelMatrix();

    MathUtil::ModelMatrix().translate(0,height*0.5f-row_height*0.5f,0);

    //draw userid slightly in front of back box
    MathUtil::PushModelMatrix();
    MathUtil::ModelMatrix().translate(0,0,0.02f);
    MathUtil::MultModelMatrix(textgeom_player_id->GetModelMatrix());
    textgeom_player_id->DrawGL(shader);

    //draw speaker if talking    
    if (ghost_frame.speaking && sound_img) {
        MathUtil::ModelMatrix().translate(-width*0.5f -1.0f,0,0);
        MathUtil::ModelMatrix().scale(2.0f, 2.0f, 1.0f);

        Renderer::m_pimpl->BindTextureHandle(0, sound_img->GetTextureHandle(true));
        shader->SetUseTexture(0, true);
        SpinAnimation::DrawPlaneGL(shader, QColor(255,255,255,255));
        shader->SetUseTexture(0, false);
    }

    MathUtil::PopModelMatrix();


    MathUtil::PushModelMatrix();

    MathUtil::ModelMatrix().translate(0,-row_height*1.5f,0.02f);
    for (int i=0; i<textgeom_chatmessages.size(); ++i) {

        const float interp = (chat_message_times[i].elapsed() + chat_message_time_offsets[i] * 1000.0f)/message_duration_msec;

        float alpha = 1.0f;
        if (interp < 0.1f) {
            alpha = interp * 10.0f;
        }
        else if (interp > 0.9f) {
            alpha = 1.0f - (interp-0.9f) * 10.0f;
        }

        if (interp > 0.0f && interp < 1.0f) {
            //draw grey box
            MathUtil::PushModelMatrix();
            MathUtil::ModelMatrix().scale(float(textgeom_chatmessages[i]->GetTextLength()), row_height*textgeom_chatmessages[i]->GetAllText().size(), 1);
            SpinAnimation::DrawPlaneGL(shader, QColor(128,128,128,128*alpha));
            MathUtil::PopModelMatrix();

            MathUtil::PushModelMatrix();
            MathUtil::ModelMatrix().translate(0,0,0.02f);
            MathUtil::MultModelMatrix(textgeom_chatmessages[i]->GetModelMatrix());
            textgeom_chatmessages[i]->SetColour(QColor(255,255,255,255*alpha));
            textgeom_chatmessages[i]->DrawGL(shader);
            MathUtil::PopModelMatrix();
        }

        MathUtil::ModelMatrix().translate(0,-row_height*textgeom_chatmessages[i]->GetAllText().size() - row_height * 0.25f,0);

        if (interp > 1.0f) {
            textgeom_chatmessages.removeAt(i);
            chat_message_times.removeAt(i);
            chat_message_time_offsets.removeAt(i);
            --i;
        }
    }

    MathUtil::PopModelMatrix();

    MathUtil::PopModelMatrix();
}

void RoomObject::SetGrabbed(const bool b)
{
    grabbed = b;
}

bool RoomObject::GetGrabbed() const
{
    return grabbed;
}

QString RoomObject::ShortenString(const QString & s)
{
    const int maxlen = 40;
    if (s.length() < maxlen) {
       return s;
    }
    else {
        return s.left(maxlen/2) + QString("...") + s.right(maxlen/2);
    }
}

void RoomObject::DrawGL(QPointer <AssetShader> shader)
{ 
    DrawPortalGL(shader);
}

void RoomObject::DrawDecorationsGL(QPointer <AssetShader> shader, const float anim_val)
{
    DrawPortalDecorationsGL(shader, anim_val);
}

void RoomObject::DrawStencilGL(QPointer <AssetShader> shader, const QVector3D & player_pos) const
{    
    DrawPortalStencilGL(shader, GetPlayerAtSigned(player_pos));
}

float RoomObject::GetWidth() const
{
    return GetScale().x();
}

bool RoomObject::GetPlayerAtSigned(const QVector3D & player_pos) const
{
    const QVector3D local = GetLocal(player_pos);
    return (fabsf(local.x()) <= 0.5f && local.y() >= 0.0f && local.y() <= 1.6f && local.z() >= 0.0f && local.z() <= 1.0f);
}

bool RoomObject::GetPlayerCrossed(const QVector3D & player_pos, const QVector3D & player_last_pos) const
{
    QVector3D local1 = GetLocal(player_pos);
    QVector3D local2 = GetLocal(player_last_pos);    

    return (local1.z()) < 0.0f && (local2.z()) >= 0.0f && fabsf(local2.x()) <= 0.5f && local2.y() >= 0.0f && local2.y() <= 1.6f;
}

float RoomObject::GetSpacing()
{
    return portal_spacing;
}

void RoomObject::SetThumbAssetImage(const QPointer <AssetImage> a)
{
    portal_thumb_img = a;
}

QPointer <AssetImage> RoomObject::GetThumbAssetImage()
{
    return portal_thumb_img;
}

MediaContext * RoomObject::GetMediaContext()
{
    return &media_ctx;
}

QPointer <RoomObject> RoomObject::CreateFromProperties(QPointer<DOMNode> properties)
{
    QPointer <RoomObject> obj(new RoomObject());
    obj->SetProperties(properties);
    return obj;
}

void RoomObject::UpdateAssets()
{
    if (cursor_arrow_obj) {
        cursor_arrow_obj->Update();
        cursor_arrow_obj->UpdateGL();
    }
    if (cursor_crosshair_obj) {
        cursor_crosshair_obj->Update();
        cursor_crosshair_obj->UpdateGL();
    }
    if (cursor_hand_obj) {
        cursor_hand_obj->Update();
        cursor_hand_obj->UpdateGL();
    }
    if (linear_gradient_img) {
        linear_gradient_img->UpdateGL();
    }
    if (sound_img) {
        sound_img->UpdateGL();
    }
    if (light_img) {
        light_img->UpdateGL();
    }
    if (object_img) {
        object_img->UpdateGL();
    }
    if (particle_img) {
        particle_img->UpdateGL();
    }
    if (avatar_obj) {
        avatar_obj->Update();
        avatar_obj->UpdateGL();
    }
    if (avatar_head_obj) {
        avatar_head_obj->Update();
        avatar_head_obj->UpdateGL();
    }
}

bool RoomObject::GetDrawBack() const
{
    return draw_back;
}

void RoomObject::SetDrawBack(bool value)
{
    draw_back = value;
}

void RoomObject::SetSwallowState(const int i)
{
    swallow_state = i;
}

int RoomObject::GetSwallowState() const
{
    return swallow_state;
}

void RoomObject::SetSwallowTime(const int i)
{
    swallow_time = i;
}

int RoomObject::GetSwallowTime() const
{
    return swallow_time;
}

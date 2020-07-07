#ifndef ENVOBJECT_H
#define ENVOBJECT_H

#include <QtScript>
#include <QtGui>

#include "htmlpage.h"
#include "settingsmanager.h"
#include "assetskybox.h"
#include "leaphands.h"
#include "textgeom.h"
#include "spinanimation.h"
#include "soundmanager.h"
#include "particlesystem.h"
#include "controllermanager.h"
#include "lightmanager.h"
#include "assetvideo.h"
#include "domnode.h"
#include "mediaplayer.h"
#include "assetwebsurface.h"

//59.7 - circ deps that need resolving
class SpinAnimation;
class ParticleSystem;

struct RoomObjectEdit
{
    QList <QString> data;
    QString user;
    QString room;
};

class RoomObject : public QObject
{
public:

    RoomObject();
    ~RoomObject();   

    static QPointer <RoomObject> CreateText(const QString & js_id, const float fixed_size, const QString & text, const QColor col, QVector3D scale);
    static QPointer <RoomObject> CreateImage(const QString & js_id, const QString & id, const QColor col, const bool lighting);
    static QPointer <RoomObject> CreateObject(const QString & js_id, const QString & id, const QColor col, const bool lighting);
    static QPointer <RoomObject> CreateParagraph(const QString & js_id, const QString & id, const QString & text, const QColor col, const bool lighting);

    static void initializeGL();

    QMatrix4x4 GetModelMatrixLocal() const;
    QMatrix4x4 GetModelMatrixGlobal() const;

    void Copy(const QPointer <RoomObject> obj);

    void SetInterpolation();

    void Clear();

    static QPointer <RoomObject> CreateFromProperties(QPointer <DOMNode> properties);

    void SetProperties(QPointer <DOMNode> props);
    void SetProperties(QVariantMap d);
    QPointer <DOMNode> GetProperties();

    void SetType(const ElementType t);
    ElementType GetType() const;

    void SetInterfaceObject(const bool b);
    bool GetInterfaceObject() const;

    bool GetRaycastIntersection(const QMatrix4x4 transform, QList <QVector3D> & int_verts, QList <QVector3D> & int_normals, QList <QVector2D> & int_texcoords);

    void SetAttributeModelMatrix(const QMatrix4x4 & m);
    QMatrix4x4 GetAttributeModelMatrix() const;

    void SetAttributeModelMatrixNoScaling(const QMatrix4x4 & m);
    QMatrix4x4 GetAttributeModelMatrixNoScaling() const;

    QVector3D GetPos() const;

    QVector3D GetVel() const;

    QVector3D GetAccel() const;

    void SetDir(const QVector3D & p);
    QVector3D GetDir() const;

    void SnapXDirsToMajorAxis();

    inline QVector3D GetXDir() const
    {
        QVector3D xdir = props->GetXDir()->toQVector3D();
        if (props->GetInterpolate() && interp_val < 1.0f) {
            return (interp_xdir * (1.0f - interp_val) + xdir * interp_val).normalized();
        }        
        return xdir;
    }

    QVector3D GetYDir() const
    {
        QVector3D ydir = props->GetYDir()->toQVector3D();
        if (props->GetInterpolate()  && interp_val < 1.0f) {
            return (interp_ydir * (1.0f - interp_val) + ydir * interp_val).normalized();
        }
        return ydir;
    }

    QVector3D GetZDir() const
    {
        QVector3D zdir = props->GetZDir()->toQVector3D();
        if (props->GetInterpolate()  && interp_val < 1.0f) {
            return (interp_zdir * (1.0f - interp_val) + zdir * interp_val).normalized();
        }
        return zdir;
    }

    void SetSaveToMarkup(const bool b);
    bool GetSaveToMarkup() const;

    inline QVector3D GetScale() const
    {
        QVector3D scale = props->GetScale()->toQVector3D();
        if (props->GetInterpolate()  && interp_val < 1.0f) {
            return interp_scale * (1.0f - interp_val) + scale * interp_val;
        }        
        return scale;
    }

    static qsreal NextUUID();    

    QMatrix4x4 GetRotationMatrix() const;

    void SetSelected(const bool b);
    bool GetSelected() const;

    void SetCollisionSet(const QSet <QString> s);
    QSet <QString> GetCollisionSet() const;   

    void SetPlaySounds(const bool b);
    bool GetPlaySounds() const;

    void ReadXMLCode(const QString & str);
    QString GetXMLCode(const bool show_defaults = false) const;

    QVariantMap GetJSONCode(const bool show_defaults) const;

    void Update(const double dt_sec);
    void UpdateMedia();

    void DoLoadEffect() const;   

    void DrawGL(QPointer <AssetShader> shader, const bool left_eye, const QVector3D & player_pos);
    void DrawCursorGL(QPointer <AssetShader> shader);
    void DrawLoadingGL(QPointer <AssetShader> shader);            
    void DrawCollisionModelGL(QPointer <AssetShader> shader);

    //unique to text
    void SetText(const QString & s);
    void SetText(const QString & s, const bool add_markup);
    QString GetText() const;

    void SetFixedSize(const bool fixed_size, const float size);

    //unique to ghost       
    void SetDoMultiplayerTimeout(const bool b);

    void SetChatMessage(const QString & s);
    QString GetChatMessage() const;

    //unique to ghost
    void SetHMDType(const QString & s);
    QString GetHMDType() const;   

    void SetUserIDPos(const QVector3D & p);
    QVector3D GetUserIDPos() const;

    void SetHeadAvatarPos(const QVector3D & p); //the offset of the head from the body for the avatar
    QVector3D GetHeadAvatarPos() const;

    void SetAvatarCode(const QString & s);
    QString GetAvatarCode() const;

    //unique to Object
    void SetCollisionAssetObject(const QPointer <AssetObject> a);
    QPointer <AssetObject> GetCollisionAssetObject();

    void SetEmitterAssetObject(const QPointer <AssetObject> a);
    QPointer <AssetObject> GetEmitterAssetObject();

    //setting assets    
    void SetAssetObject(const QPointer <AssetObject> a);
    QPointer <AssetObject> GetAssetObject();

    void SetTeleportAssetObject(const QPointer <AssetObject> a);
    QPointer <AssetObject> GetTeleportAssetObject();

    //objects, animation related
    void SetAnimAssetObject(const QPointer <AssetObject> a);
    QPointer <AssetObject> GetAnimAssetObject() const;

    void SetBlendAssetObject(const int i, const QPointer <AssetObject> a);
    QPointer <AssetObject> GetBlendAssetObject(const int i);

    //Objects, parented
    void SetChildObjects(QList <QPointer <RoomObject> > & e);
    QList <QPointer <RoomObject> > & GetChildObjects();

    void AppendChild(QPointer <RoomObject> child);
    void RemoveChildByJSID(QString);

    //assets - ghost related
    void SetGhostAssetObjects(const QHash <QString, QPointer <AssetObject> > & a);
    QHash <QString, QPointer <AssetObject> > GetGhostAssetObjects();

    void SetLeftAssetImage(const QPointer <AssetImage> a);
    void SetRightAssetImage(const QPointer <AssetImage> a);
    void SetAssetVideo(const QPointer <AssetVideo> a);
    QPointer <AssetVideo> GetAssetVideo();
    void SetAssetWebSurface(const QPointer <AssetWebSurface> a);
    QPointer <AssetWebSurface> GetAssetWebSurface();

    void SetAssetImage(const QPointer <AssetImage> a);
    QPointer <AssetImage> GetAssetImage();

    void SetAssetLightmap(const QPointer <AssetImage> a);
    QPointer <AssetImage> GetAssetLightmap();

    void SetAssetShader(const QPointer <AssetShader> a);
    QPointer <AssetShader> GetAssetShader();
    void SetAssetGhost(const QPointer <AssetGhost> a);
    QPointer <AssetGhost> GetAssetGhost();

    //unique to Sound
    void SetAssetSound(const QPointer <AssetSound> a);
    QPointer <AssetSound> GetAssetSound() const;
    void Play();
    void Pause();
    void Seek(const float f);
    void Stop();
    bool GetPlaying() const;

    bool TestPlayerPosition(const QPointF & p);

    bool Clicked(const QVector3D & p);      

    //unique to portal
    void SetURL(const QString & base, const QString & url_s);
    void SetURL(const QString & s);
    QString GetURL() const;        
    void SetTitle(const QString & s);
    QString GetTitle() const;
    QPointer <TextGeom> GetTextGeomURL();
    QPointer <TextGeom> GetTextGeomTitle();
    void DrawPortalGL(QPointer <AssetShader> shader);
    void DrawPortalDecorationsGL(QPointer <AssetShader> shader, const float anim_val);
    void DrawPortalBackGL(QPointer <AssetShader> shader) const;
    void DrawPortalInsideGL(QPointer <AssetShader> shader) const;
    void DrawPortalFrameGL(QPointer <AssetShader> shader);
    void DrawPortalStencilGL(QPointer <AssetShader> shader, const bool draw_offset_stencil) const;
    void DrawPortalSelectionGL(QPointer <AssetShader> shader);    

    void SetInterpTime(const float f);
    float GetInterpTime() const;

    int GetNumTris() const;

    void PlayCreateObject();
    void PlayDeleteObject();

    void PlayCreatePortal();

    float GetTimeElapsed() const;

    void LoadGhost(const QString & data);
    void DoGhostMoved(const QVariantMap & m);
    QList <RoomObjectEdit> & GetRoomEditsIncoming();
    QList <RoomObjectEdit> & GetRoomDeletesIncoming();

    QList <QString> & GetSendPortalURL();
    QList <QString> & GetSendPortalJSID();
    QList <QVector3D> & GetSendPortalPos();
    QList <QVector3D> & GetSendPortalFwd();

    void SetPlayerInRoom(const bool b);
    void SetPlayerInAdjacentRoom(const bool b);

    void SetRescaleOnLoad(const bool b);
    void SetRescaleOnLoadAspect(const bool b);
    bool GetRescaleOnLoad() const;
    void SetPlayWhenReady(const bool b);   

    void SetParentObject(QPointer<RoomObject>);
    QPointer <RoomObject> GetParentObject()
    {
        return parent_object;
    }
    //void SetContainerRoom(QPointer<Room>);

    void SetDirty(const bool);
    bool IsDirty() const;

    void SetCubemapRadiance(const QPointer <AssetImage> a);
    QPointer <AssetImage> GetCubemapRadiance() const;

    void SetCubemapIrradiance(const QPointer <AssetImage> a);
    QPointer <AssetImage> GetCubemapIrradiance() const;

    // Frustum Culling Member Functions
    QVector3D GetBBoxMin();
    QVector3D GetBBoxMax();    

    void GetLights(LightContainer* p_container, QMatrix4x4* p_model_matrix);
    void GetLight(LightContainer* p_container, QMatrix4x4* p_model_matrix);

    void SetGrabbed(const bool b);
    bool GetGrabbed() const;

    bool GetDrawBack() const;
    void SetDrawBack(bool value);

    static void SetDrawAssetObjectTeleport(const bool b);
    static bool GetDrawAssetObjectTeleport();   

    static float GetRate();
    static float GetLogoffRate();

    void SetGhostAssetShaders(const QHash<QString, QPointer <AssetShader> > &a);
    QHash<QString, QPointer <AssetShader> > GetGhostAssetShaders();

    void SetAutoLoadTriggered(const bool b);
    bool GetAutoLoadTriggered() const;   

    void SetThumbAssetImage(const QPointer <AssetImage> a);
    QPointer <AssetImage> GetThumbAssetImage();

    void DrawGL(QPointer <AssetShader> shader);
    void DrawStencilGL(QPointer <AssetShader> shader, const QVector3D & player_pos) const;
    void DrawDecorationsGL(QPointer <AssetShader> shader, const float anim_val);

    float GetWidth() const;

    bool GetPlayerAtSigned(const QVector3D & player_pos) const;
    bool GetPlayerCrossed(const QVector3D & player_pos, const QVector3D & player_last_pos) const;
    QVector3D GetLocal(const QVector3D & p) const;
    QVector3D GetGlobal(const QVector3D & p) const;

    MediaContext * GetMediaContext();

    static void UpdateAssets();

    static float GetSpacing();

    void SetSwallowState(const int i);
    int GetSwallowState() const;

    void SetSwallowTime(const int i);
    int GetSwallowTime() const;

    static QPointer <AssetObject> cursor_arrow_obj;
    static QPointer <AssetObject> cursor_crosshair_obj;
    static QPointer <AssetObject> cursor_hand_obj;

protected:

    void DrawGhostUserIDChat(QPointer <AssetShader> shader);
    void DrawIconGL(QPointer <AssetShader> shader, const QPointer <AssetImage> img);
    void UpdateMatrices();

    void LoadGhost_Helper(const int depth, const QVariantMap & d, QPointer <RoomObject> parent_object, QHash<QString, QPointer<AssetObject> > &asset_obj_list, QHash<QString, QPointer<AssetShader> > &asset_shader_list);   

    //portal specific
    QString ShortenString(const QString & s);

    QPointer <DOMNode> props;

    QVector3D last_rotation;
    QString hmd_type;    
    QVector3D userid_pos;
    float eye_ipd; //virtual ipd   

    bool selected;    

    QPointer <SpinAnimation> anim;

    //paragraph    
    int tex_width;
    int tex_height;

    //text
    QPointer <TextGeom> textgeom;
    QPointer <TextGeom> textgeom_player_id;    

    //image
    QPointer <AssetImage> assetimage;
    QPointer <AssetImage> assetimage_lmap;

    // Per-Object Cubemaps
    QPointer <AssetImage> m_cubemap_radiance;
    QPointer <AssetImage> m_cubemap_irradiance;

    //sound
    QPointer <AssetSound> assetsound;     

    static float rate;
    static float logoff_rate;

    //websurface
    QPointer <AssetWebSurface> assetwebsurface;

    //video
    QPointer <AssetVideo> assetvideo;
    MediaContext media_ctx;

    //object
    QPointer <AssetObject> assetobject_anim;
    QPointer <AssetObject> assetobject;
    QPointer <AssetObject> assetobject_collision;
    QPointer <AssetObject> assetobject_teleport;
    QPointer <AssetObject> assetobject_emitter;
    QVector <QPointer <AssetObject> > assetobject_blendshapes;

    QPointer <AssetShader> assetshader; //optional shader

    //ghost    
    QPointer <AssetGhost> assetghost;
    float time_elapsed;
    bool playing;
    QString chat_message;    
    QList <QTime> chat_message_times;
    QVector3D head_avatar_pos;

    QHash <QString, QPointer <AssetObject> > ghost_assetobjs;
    QHash <QString, QPointer <AssetShader> > ghost_asset_shaders;
    bool do_multiplayer_timeout;        
    QString avatar_code;
    bool play_sounds;
    int ghost_frame_index;

    GhostFrame ghost_frame;

    QList <QPointer <RoomObject> > child_objects;
    QPointer <RoomObject> parent_object;   

    QList <QPointer <TextGeom> > textgeom_chatmessages;
    QList <float> chat_message_time_offsets;

    //portal
    QPointer <TextGeom> textgeom_url;
    QPointer <TextGeom> textgeom_title;

    QPointer <AssetImage> portal_thumb_img;

    int swallow_time;
    int swallow_state;

    //collider internal tracking related
    QSet <QString> collision_set;

    bool error_reported;

    QMatrix4x4 model_matrix_parent;
    QMatrix4x4 model_matrix_local;
    QMatrix4x4 model_matrix_global;

    float interp_val;
    float interp_time;
    QTime interp_timer;
    QVector3D interp_pos;
    QVector3D interp_xdir;
    QVector3D interp_ydir;
    QVector3D interp_zdir;
    QVector3D interp_scale;   

    QList <QByteArray> sound_buffers;
    unsigned int sound_buffers_sample_rate;

    ALuint openal_stream_source;

    QPointer <ParticleSystem> particle_system;

    QList <RoomObjectEdit> room_edits_incoming;
    QList <RoomObjectEdit> room_deletes_incoming;

    QList <QString> send_portal_url;
    QList <QString> send_portal_jsid;
    QList <QVector3D> send_portal_pos;
    QList <QVector3D> send_portal_fwd;

    bool player_in_room;
    bool player_in_adjacent_room;

    bool rescale_on_load;
    bool rescale_on_load_aspect;
    bool rescale_on_load_done;

    bool save_to_markup;

    bool grabbed;
    bool draw_back;

    bool interface_object;

private:

    QPointer <RoomObject> portal_text;
    QString portal_last_url;
    QString portal_last_title;

    static float portal_spacing;

    //statics
    static QPointer <AssetImage> linear_gradient_img;
    static QPointer <AssetImage> sound_img;
    static QPointer <AssetImage> light_img;
    static QPointer <AssetImage> object_img;
    static QPointer <AssetImage> particle_img;
    static QPointer <AssetObject> avatar_obj;
    static QPointer <AssetObject> avatar_head_obj;

    static int objects_allocated;   
    static bool draw_assetobject_teleport;
};

#endif // ENVOBJECT_H

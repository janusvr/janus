#ifndef DOMNODE_H
#define DOMNODE_H

#include <QObject>
#include <QtScript>

#include "settingsmanager.h"
#include "scriptablevector.h"
#include "mathutil.h"

class DOMNode : public QObject, protected QScriptable
{
    Q_OBJECT

    //RoomProperties stuff
    Q_PROPERTY(bool fog READ GetFog WRITE SetFog)
    Q_PROPERTY(float fog_density READ GetFogDensity WRITE SetFogDensity)
    Q_PROPERTY(float fog_start READ GetFogStart WRITE SetFogStart)
    Q_PROPERTY(float fog_end READ GetFogEnd WRITE SetFogEnd)
    Q_PROPERTY(ScriptableVector * fog_col READ GetFogCol WRITE SetFogCol)
    Q_PROPERTY(ScriptableVector * col READ GetColour WRITE SetColour)

    Q_PROPERTY(float gravity READ GetGravity WRITE SetGravity)
    Q_PROPERTY(float jump_velocity READ GetJumpVelocity WRITE SetJumpVelocity)
    Q_PROPERTY(float walk_speed READ GetWalkSpeed WRITE SetWalkSpeed)
    Q_PROPERTY(float run_speed READ GetRunSpeed WRITE SetRunSpeed)

    Q_PROPERTY(bool swallow READ GetSwallow WRITE SetSwallow)

    Q_PROPERTY(bool interpolate READ GetInterpolate WRITE SetInterpolate)
    Q_PROPERTY(ScriptableVector * pos READ GetPos WRITE SetPos)
    Q_PROPERTY(ScriptableVector * xdir READ GetXDir WRITE SetXDir)
    Q_PROPERTY(ScriptableVector * ydir READ GetYDir WRITE SetYDir)
    Q_PROPERTY(ScriptableVector * zdir READ GetZDir WRITE SetZDir)
    Q_PROPERTY(ScriptableVector * fwd READ GetDir WRITE SetDir)
    Q_PROPERTY(ScriptableVector * vel READ GetVel WRITE SetVel)
    Q_PROPERTY(ScriptableVector * accel READ GetAccel WRITE SetAccel)
    Q_PROPERTY(ScriptableVector * scale READ GetScale WRITE SetScale)

    Q_PROPERTY(ScriptableVector * view_dir READ GetViewDir WRITE SetViewDir)
    Q_PROPERTY(ScriptableVector * up_dir READ GetUpDir WRITE SetUpDir)
    Q_PROPERTY(ScriptableVector * rotation READ GetRotation WRITE SetRotation)
    Q_PROPERTY(QString rotation_order READ GetRotationOrder WRITE SetRotationOrder)

    Q_PROPERTY(ScriptableVector * rand_pos READ GetRandPos WRITE SetRandPos)
    Q_PROPERTY(ScriptableVector * rand_vel READ GetRandVel WRITE SetRandVel)
    Q_PROPERTY(ScriptableVector * rand_accel READ GetRandAccel WRITE SetRandAccel)
    Q_PROPERTY(ScriptableVector * rand_col READ GetRandColour WRITE SetRandColour)
    Q_PROPERTY(ScriptableVector * rand_scale READ GetRandScale WRITE SetRandScale)

    Q_PROPERTY(int uuid READ GetUUID)
    Q_PROPERTY(QString js_id READ GetJSID WRITE SetJSID)
    Q_PROPERTY(QString id READ GetID WRITE SetID)

    Q_PROPERTY(QString cubemap_radiance_id READ GetCubemapRadianceID WRITE SetCubemapRadianceID)
    Q_PROPERTY(QString cubemap_irradiance_id READ GetCubemapIrradianceID WRITE SetCubemapIrradianceID)

    Q_PROPERTY(QString type READ GetTypeAsString WRITE SetType)

    Q_PROPERTY(QString blend0_id READ GetBlend0ID WRITE SetBlend0ID)
    Q_PROPERTY(QString blend1_id READ GetBlend1ID WRITE SetBlend1ID)
    Q_PROPERTY(QString blend2_id READ GetBlend2ID WRITE SetBlend2ID)
    Q_PROPERTY(QString blend3_id READ GetBlend3ID WRITE SetBlend3ID)

    Q_PROPERTY(float blend0 READ GetBlend0 WRITE SetBlend0)
    Q_PROPERTY(float blend1 READ GetBlend0 WRITE SetBlend1)
    Q_PROPERTY(float blend2 READ GetBlend0 WRITE SetBlend2)
    Q_PROPERTY(float blend3 READ GetBlend0 WRITE SetBlend3)

    Q_PROPERTY(float collision_radius READ GetCollisionRadius WRITE SetCollisionRadius)

    Q_PROPERTY(QString text READ GetText WRITE SetText)
    Q_PROPERTY(int font_size READ GetFontSize WRITE SetFontSize)

    Q_PROPERTY(QString onclick READ GetOnClick WRITE SetOnClick)
    Q_PROPERTY(QString oncollision READ GetOnCollision WRITE SetOnCollision)
    Q_PROPERTY(QString onentry READ GetOnEnter WRITE SetOnEnter)
    Q_PROPERTY(QString onexit READ GetOnExit WRITE SetOnExit)

    Q_PROPERTY(bool collision_static READ GetCollisionStatic WRITE SetCollisionStatic)
    Q_PROPERTY(bool collision_trigger READ GetCollisionTrigger WRITE SetCollisionTrigger)

    //Particle related
    Q_PROPERTY(QString emitter_id READ GetEmitterID WRITE SetEmitterID)
    Q_PROPERTY(bool emit_local READ GetEmitLocal WRITE SetEmitLocal)

    //Collision related
    Q_PROPERTY(QString collision_id READ GetCollisionID WRITE SetCollisionID)
    Q_PROPERTY(float collision_friction READ GetCollisionFriction WRITE SetCollisionFriction)
    Q_PROPERTY(float collision_rollingfriction READ GetCollisionRollingFriction WRITE SetCollisionRollingFriction)
    Q_PROPERTY(float collision_restitution READ GetCollisionRestitution WRITE SetCollisionRestitution)
    Q_PROPERTY(float collision_angulardamping READ GetCollisionAngularDamping WRITE SetCollisionAngularDamping)
    Q_PROPERTY(float collision_lineardamping READ GetCollisionLinearDamping WRITE SetCollisionLinearDamping)
    Q_PROPERTY(ScriptableVector * collision_pos READ GetCollisionPos WRITE SetCollisionPos)
    Q_PROPERTY(ScriptableVector * collision_scale READ GetCollisionScale WRITE SetCollisionScale)

    Q_PROPERTY(QString image_id READ GetImageID WRITE SetImageID)
    Q_PROPERTY(QString websurface_id READ GetWebsurfaceID WRITE SetWebsurfaceID)
    Q_PROPERTY(QString teleport_id READ GetTeleportID WRITE SetTeleportID)
    Q_PROPERTY(QString bone_id READ GetBoneID WRITE SetBoneID)
    Q_PROPERTY(QString video_id READ GetVideoID WRITE SetVideoID)
    Q_PROPERTY(QString thumb_id READ GetThumbID WRITE SetThumbID)
    Q_PROPERTY(QString shader_id READ GetShaderID WRITE SetShaderID)

    Q_PROPERTY(ScriptableVector * rotate_axis READ GetSpinAxis WRITE SetSpinAxis)
    Q_PROPERTY(float rotate_deg_per_sec READ GetSpinVal WRITE SetSpinVal)

    Q_PROPERTY(bool lighting READ GetLighting WRITE SetLighting)

    Q_PROPERTY(bool visible READ GetVisible WRITE SetVisible)

    Q_PROPERTY(bool loop READ GetLoop WRITE SetLoop)
    Q_PROPERTY(float gain READ GetGain WRITE SetGain)
    Q_PROPERTY(float doppler_factor READ GetDopplerFactor WRITE SetDopplerFactor)
    Q_PROPERTY(float outer_gain READ GetOuterGain WRITE SetOuterGain)
    Q_PROPERTY(float inner_angle READ GetInnerAngle WRITE SetInnerAngle)
    Q_PROPERTY(float outer_angle READ GetOuterAngle WRITE SetOuterAngle)
    Q_PROPERTY(float pitch READ GetPitch WRITE SetPitch)

    Q_PROPERTY(bool sync READ GetSync WRITE SetSync)

    Q_PROPERTY(QString anim_id READ GetAnimID WRITE SetAnimID)
    Q_PROPERTY(float anim_speed READ GetAnimSpeed WRITE SetAnimSpeed)

    Q_PROPERTY(float current_time READ GetCurTime WRITE SetCurTime)
    Q_PROPERTY(float total_time READ GetTotalTime WRITE SetTotalTime)

    //portal related
    Q_PROPERTY(QString title READ GetTitle WRITE SetTitle)
    Q_PROPERTY(bool open READ GetOpen WRITE SetOpen)

    //particle emitter related
    Q_PROPERTY(int count READ GetCount WRITE SetCount)
    Q_PROPERTY(int rate READ GetRate WRITE SetRate)
    Q_PROPERTY(float duration READ GetDuration WRITE SetDuration)
    Q_PROPERTY(float fade_in READ GetFadeIn WRITE SetFadeIn)
    Q_PROPERTY(float fade_out READ GetFadeOut WRITE SetFadeOut)

    Q_PROPERTY(QString url READ GetURL WRITE SetURL)

    // Light related
    Q_PROPERTY(float light_intensity READ GetLightIntensity WRITE SetLightIntensity)
    Q_PROPERTY(float light_cone_angle READ GetLightConeAngle WRITE SetLightConeAngle)
    Q_PROPERTY(float light_cone_exponent READ GetLightConeExponent WRITE SetLightConeExponent)
    Q_PROPERTY(float light_range READ GetLightRange WRITE SetLightRange)

    // Draw Priority
    Q_PROPERTY(float Draw_Layer READ GetDrawLayer WRITE SetDrawLayer)

    Q_PROPERTY(DOMNode * parent READ GetParent WRITE SetParent)
    Q_PROPERTY(QList <DOMNode * > children READ GetChildren WRITE SetChildren)
    Q_PROPERTY(DOMNode * leftSibling READ GetLeftSibling WRITE SetLeftSibling)
    Q_PROPERTY(DOMNode * rightSibling READ GetRightSibling WRITE SetRightSibling)
    Q_PROPERTY(bool dirty READ IsDirty WRITE SetDirty)

    //player-related
    Q_PROPERTY(bool cursor_active MEMBER cursor0_active)
    Q_PROPERTY(QString cursor_object MEMBER cursor0_object)
    Q_PROPERTY(ScriptableVector * cursor_pos READ GetCursor0Pos WRITE SetCursor0Pos)
    Q_PROPERTY(ScriptableVector * cursor_xdir READ GetCursor0XDir WRITE SetCursor0XDir)
    Q_PROPERTY(ScriptableVector * cursor_ydir READ GetCursor0YDir WRITE SetCursor0YDir)
    Q_PROPERTY(ScriptableVector * cursor_zdir READ GetCursor0ZDir WRITE SetCursor0ZDir)
    Q_PROPERTY(bool cursor0_active MEMBER cursor0_active)
    Q_PROPERTY(QString cursor0_object MEMBER cursor0_object)
    Q_PROPERTY(ScriptableVector * cursor0_pos READ GetCursor0Pos WRITE SetCursor0Pos)
    Q_PROPERTY(ScriptableVector * cursor0_xdir READ GetCursor0XDir WRITE SetCursor0XDir)
    Q_PROPERTY(ScriptableVector * cursor0_ydir READ GetCursor0YDir WRITE SetCursor0YDir)
    Q_PROPERTY(ScriptableVector * cursor0_zdir READ GetCursor0ZDir WRITE SetCursor0ZDir)
    Q_PROPERTY(bool cursor1_active MEMBER cursor1_active)
    Q_PROPERTY(QString cursor1_object MEMBER cursor1_object)
    Q_PROPERTY(ScriptableVector * cursor1_pos READ GetCursor1Pos WRITE SetCursor1Pos)
    Q_PROPERTY(ScriptableVector * cursor1_xdir READ GetCursor1XDir WRITE SetCursor1XDir)
    Q_PROPERTY(ScriptableVector * cursor1_ydir READ GetCursor1YDir WRITE SetCursor1YDir)
    Q_PROPERTY(ScriptableVector * cursor1_zdir READ GetCursor1ZDir WRITE SetCursor1ZDir)    
    Q_PROPERTY(QString userid READ GetUserID)    
//    Q_PROPERTY(bool hmd_enabled READ GetHMDEnabled)
    Q_PROPERTY(ScriptableVector * local_head_pos READ GetLocalHeadPos)
    Q_PROPERTY(ScriptableVector * head_pos READ GetGlobalHeadPos)
    Q_PROPERTY(ScriptableVector * eye_pos READ GetEyePos)
    Q_PROPERTY(bool hand0_active READ GetHand0Active)
    Q_PROPERTY(ScriptableVector * hand0_trackpad READ GetHand0Trackpad)
    Q_PROPERTY(ScriptableVector * hand0_pos READ GetHand0Pos)
    Q_PROPERTY(ScriptableVector * hand0_vel READ GetHand0Vel)
    Q_PROPERTY(ScriptableVector * hand0_xdir READ GetHand0XDir)
    Q_PROPERTY(ScriptableVector * hand0_ydir READ GetHand0YDir)
    Q_PROPERTY(ScriptableVector * hand0_zdir READ GetHand0ZDir)
    Q_PROPERTY(bool hand1_active READ GetHand1Active)
    Q_PROPERTY(ScriptableVector * hand1_trackpad READ GetHand1Trackpad)
    Q_PROPERTY(ScriptableVector * hand1_pos READ GetHand1Pos)
    Q_PROPERTY(ScriptableVector * hand1_vel READ GetHand1Vel)
    Q_PROPERTY(ScriptableVector * hand1_xdir READ GetHand1XDir)
    Q_PROPERTY(ScriptableVector * hand1_ydir READ GetHand1YDir)
    Q_PROPERTY(ScriptableVector * hand1_zdir READ GetHand1ZDir)
    Q_PROPERTY(QString hmd_type READ GetHMDType)
    Q_PROPERTY(QString device_type READ GetDeviceType)
    Q_PROPERTY(ScriptableVector * emitter_pos READ GetEmitterPos WRITE SetEmitterPos)
//    Q_PROPERTY(bool running READ GetRunning)
//    Q_PROPERTY(bool flying READ GetFlying)
//    Q_PROPERTY(bool walking READ GetWalking)
//    Q_PROPERTY(bool speaking READ GetSpeaking)

//    Q_PROPERTY(ScriptableVector * fwd READ GetFwd WRITE SetFwd)
public:

    explicit DOMNode(QObject *parent = 0);
    ~DOMNode();

    void SetProperties(const QVariantMap & d);

    void SetFwd(const ScriptableVector * v);
    ScriptableVector * GetFwd() const;

    void Copy(QPointer <DOMNode> p);

    void SetParent(DOMNode *);
    DOMNode * GetParent() const;

    void SetChildren(QList <DOMNode *>);
    QList <DOMNode *> GetChildren() const;
    void ClearChildren();

    void SetLeftSibling (DOMNode *);
    DOMNode * GetLeftSibling() const;

    void SetRightSibling (DOMNode*);
    DOMNode * GetRightSibling() const;

    void AppendChild (QPointer <DOMNode>);
    void PrependChild (QPointer <DOMNode>);
    void RemoveChild (QPointer <DOMNode>);
    void RemoveChildAt (int pos);
    QPointer <DOMNode> RemoveChildByJSID(QString);

    void SetType(QString & t);
    void SetType(const ElementType &t);
    inline ElementType GetType() const { return type; }
    QString GetTypeAsString();

    void SetInterpolate(const bool b);
    bool GetInterpolate() const;

    void SetPos(ScriptableVector *&p);
    void SetPos(const QVector3D & p);
    inline ScriptableVector * GetPos() { return pos; }

    void SetXDir(ScriptableVector * & x);
    void SetXDir(const QVector3D & x);
    inline ScriptableVector * GetXDir() { return xdir; }

    void SetYDir(ScriptableVector * & y);
    void SetYDir(const QVector3D & y);
    inline ScriptableVector * GetYDir() { return ydir; }

    void SetZDir(ScriptableVector * & z);
    void SetZDir(const QVector3D & z);
    inline ScriptableVector * GetZDir() { return zdir; }

    void SetDir(ScriptableVector * & d);
    void SetDir(const QVector3D & d);
    inline ScriptableVector * GetDir() { return zdir; }

    void SetRotation(const QVector3D & v);
    void SetRotation(ScriptableVector *&v);
    ScriptableVector *GetRotation();

    void SetRotationOrder(const QString & s);
    QString GetRotationOrder() const;

    void SetVel(ScriptableVector * & v);
    void SetVel(const QVector3D & v);
    inline ScriptableVector * GetVel() { return vel; }

    void SetAccel(ScriptableVector * & v);
    void SetAccel(const QVector3D & v);
    inline ScriptableVector * GetAccel() { return accel; }

    void SetColour(ScriptableVector * c);
    void SetColour(const QVector4D & c);
    void SetColour(const QVector3D & c);
    void SetColour(const QColor & c);
    inline ScriptableVector * GetColour() { return col; }

    void SetChromaKeyColour(ScriptableVector * & c);
    void SetChromaKeyColour(const QVector4D & c);
    inline ScriptableVector * GetChromaKeyColour() { return chromakey_col; }

    void SetScale(ScriptableVector * s);
    void SetScale(const QVector3D & s);
    inline ScriptableVector * GetScale() { return scale; }

    void SetRandPos(ScriptableVector * & v);
    void SetRandPos(const QVector3D & p);
    inline ScriptableVector * GetRandPos() { return rand_pos; }

    void SetRandVel(ScriptableVector * & v);
    void SetRandVel(const QVector3D & v);
    inline ScriptableVector * GetRandVel() { return rand_vel; }

    void SetRandAccel(ScriptableVector * & v);
    void SetRandAccel(const QVector3D & v);
    inline ScriptableVector * GetRandAccel() { return rand_accel; }

    void SetRandColour(ScriptableVector * & v);
    void SetRandColour(const QVector3D & c);
    inline ScriptableVector * GetRandColour() { return rand_col; }

    void SetRandScale(ScriptableVector * & v);
    void SetRandScale(const QVector3D & s);
    inline ScriptableVector * GetRandScale() { return rand_scale; }

    void SetSync(const bool b);
    inline bool GetSync() const { return sync; }

    static int NextUUID();
    inline int GetUUID() const { return uuid; }

    void SetJSID(const QString & id);
    inline QString GetJSID() const { return js_id; }   

    void SetID(const QString & id);
    inline QString GetID() const { return id; }

    void SetCubemapRadianceID(const QString & id);
    inline QString GetCubemapRadianceID() const { return m_cubemap_radiance_id; }

    void SetCubemapIrradianceID(const QString & id);
    inline QString GetCubemapIrradianceID() const { return m_cubemap_irradiance_id; }

    void SetBlend0ID(const QString & id);
    inline QString GetBlend0ID() const { return blend0_id; }
    void SetBlend1ID(const QString & id);
    inline QString GetBlend1ID() const { return blend1_id; }
    void SetBlend2ID(const QString & id);
    inline QString GetBlend2ID() const { return blend2_id; }
    void SetBlend3ID(const QString & id);
    inline QString GetBlend3ID() const { return blend3_id; }

    void SetBlend0(const float f);
    inline float GetBlend0() const { return blend0; }
    void SetBlend1(const float f);
    inline float GetBlend1() const { return blend1; }
    void SetBlend2(const float f);
    inline float GetBlend2() const { return blend2; }
    void SetBlend3(const float f);
    inline float GetBlend3() const { return blend3; }

    void SetLoop(const bool b);
    inline bool GetLoop() const { return loop; }
    void SetGain(const float f);
    inline float GetGain() const { return gain; }
    void SetDopplerFactor(const float f);
    inline float GetDopplerFactor() const { return doppler_factor; }
    void SetOuterGain(const float f);
    inline float GetOuterGain() const { return outer_gain; }
    void SetInnerAngle(const float f);
    inline float GetInnerAngle() const { return inner_angle; }
    void SetOuterAngle(const float f);
    inline float GetOuterAngle() const { return outer_angle; }
    void SetPitch(const float f);
    inline float GetPitch() const { return pitch; }

    void SetCollisionID(const QString & id);
    inline QString GetCollisionID() const { return collision_id; }

    void SetEmitterID(const QString & id);
    inline QString GetEmitterID() const { return emitter_id; }

    void SetEmitLocal(const bool b);
    inline bool GetEmitLocal() const { return emit_local; }

    void SetCollisionRadius(const float r);
    inline float GetCollisionRadius() const { return collision_radius; }

    void SetCollisionResponse(const bool b);
    inline bool GetCollisionResponse() const { return collision_response; }

    void SetCollisionFriction(const float f);
    inline float GetCollisionFriction() const { return collision_friction; }

    void SetCollisionRollingFriction(const float f);
    inline float GetCollisionRollingFriction() const { return collision_rollingfriction; }

    void SetCollisionRestitution(const float f);
    inline float GetCollisionRestitution() const { return collision_restitution; }

    void SetCollisionAngularDamping(const float f);
    inline float GetCollisionAngularDamping() const { return collision_angulardamping; }

    void SetCollisionLinearDamping(const float f);
    inline float GetCollisionLinearDamping() const { return collision_lineardamping; }

    void SetCollisionPos(ScriptableVector *&p);
    void SetCollisionPos(const QVector3D & p);
    inline ScriptableVector * GetCollisionPos() { return collision_pos; }

    void SetCollisionScale(ScriptableVector *&p);
    void SetCollisionScale(const QVector3D & p);
    inline ScriptableVector * GetCollisionScale() { return collision_scale; }

    void SetCollisionStatic(const bool b);
    bool GetCollisionStatic() const { return collision_static; }

    void SetCollisionTrigger(const bool b);
    bool GetCollisionTrigger() const { return collision_trigger; }

    void SetCollisionCcdMotionThreshold(const float param);
    float GetCollisionCcdMotionThreshold() const { return collision_ccdmotionthreshold; }

    void SetCollisionCcdSweptSphereRadius(const float param);
    float GetCollisionCcdSweptSphereRadius() const { return collision_ccdsweptsphereradius; }

    void SetLighting(const bool b);
    inline bool GetLighting() const { return lighting; }

    void SetVisible(const bool b);
    inline bool GetVisible() const { return visible; }

    void SetText(const QString & text);
    inline QString GetText() const { return text; }

    void SetTextChanged(const bool & b);
    inline bool GetTextChanged() { return text_changed; }

    void SetFontSize(const int & newSize);
    inline int GetFontSize() const { return font_size; }

    void SetWebsurfaceID(const QString & s);
    inline QString GetWebsurfaceID() const { return websurface_id; }

    void SetTeleportID(const QString & s);
    inline QString GetTeleportID() const { return teleport_id; }

    void SetVideoID(const QString & s);
    inline QString GetVideoID() const { return video_id; }

    void SetThumbID(const QString & s);
    inline QString GetThumbID() const { return thumb_id; }

    void SetShaderID(const QString & s);
    inline QString GetShaderID() const { return shader_id; }

    void SetImageID(const QString & s);
    inline QString GetImageID() const { return image_id; }

    void SetLightmapID(const QString & s);
    inline QString GetLightmapID() const { return lightmap_id; }

    void SetTiling(ScriptableVector* & v);
    void SetTiling(const QVector4D & v);
    inline ScriptableVector * GetTiling() { return tiling; }

    void SetBoneID(const QString & s);
    inline QString GetBoneID() const { return bone_id; }

    void SetBodyID(const QString & s);
    inline QString GetBodyID() const { return body_id; }

    void SetHeadID(const QString & s);
    inline QString GetHeadID() const { return head_id; }

    void SetLightmapScale(ScriptableVector * & v);
    void SetLightmapScale(const QVector4D & c);
    inline ScriptableVector * GetLightmapScale() { return lightmap_scale; }

    void SetEmitterPos(ScriptableVector * & v);
    void SetEmitterPos(const QVector4D & c);
    inline ScriptableVector * GetEmitterPos() { return emitter_pos; }

    void SetAnimID(const QString & s);
    inline QString GetAnimID() const { return anim_id; }

    void SetAnimSpeed(const float f);
    inline float GetAnimSpeed() const { return anim_speed; }

    void SetURL(const QString & s);
    inline QString GetURL() const { return url; }

    void SetURLChanged(const bool b);
    inline bool GetURLChanged() const { return url_changed; }

    void SetSwallow(const bool b);
    inline bool GetSwallow() const { return swallow; }

    void SetBaseURL(const QString & s);
    inline QString GetBaseURL() const { return base_url; }

    void SetSrc(const QString & s);
    inline QString GetSrc() const { return src; }

    void SetVertexSrc(const QString & s);
    inline QString GetVertexSrc() const { return vertex_src; }

    void SetSrcURL(const QString & s);
    inline QString GetSrcURL() const { return src_url; }

    void SetOriginalURL(const QString & s);
    inline QString GetOriginalURL() const { return url_orig; }

    void SetTitle(const QString & s);
    inline QString GetTitle() const { return title; }

    void SetAutoLoad(const bool b);
    inline bool GetAutoLoad() const { return auto_load; }

    void SetDrawText(const bool b);
    inline bool GetDrawText() const { return draw_text; }

    void SetOpen(const bool b);
    inline bool GetOpen() const { return open; }

    void SetMirror(const bool b);
    inline bool GetMirror() const { return mirror; }

    void SetActive(const bool b);
    inline bool GetActive() const { return active; }

    void SetCount(const int i);
    inline int GetCount() const { return count; }

    void SetRate(const int i);
    inline int GetRate() const { return rate; }

    void SetDuration(const float f);
    inline float GetDuration() const { return duration; }

    void SetCurTime(const float f);
    inline float GetCurTime() const { return current_time; }

    void SetTotalTime(const float f);
    inline float GetTotalTime() const { return total_time; }

    void SetFadeIn(const float f);
    inline float GetFadeIn() const { return fade_in; }

    void SetFadeOut(const float f);
    inline float GetFadeOut() const { return fade_out; }

    void SetDirty(const bool);
    inline bool IsDirty() const { return dirty; }

    void SetSpinAxis(ScriptableVector *&p);
    void SetSpinAxis(const QVector3D & p);
    inline ScriptableVector * GetSpinAxis() { return spinaxis; }

    void SetSpinVal(const float f);
    inline float GetSpinVal() const { return spinval; }

    void SetOnClick(const QString &str);
    inline QString GetOnClick() const { return onclick_code; }

    void SetOnCollision(const QString &str);
    inline QString GetOnCollision() const { return oncollision; }

    void SetOnEnter(const QString &str);
    inline QString GetOnEnter() const { return onenter; }

    void SetOnExit(const QString &str);
    inline QString GetOnExit() const { return onexit; }

    //    void SetContainerRoom(QPointer <RoomProperties> r);

    // Light Related Member Functions
    void SetLightIntensity(float p_light_intensity);
    float GetLightIntensity();

    void SetLightConeAngle(float p_light_cone_angle);
    float GetLightConeAngle();

    void SetLightConeExponent(float p_light_cone_exponent);
    float GetLightConeExponent();

    void SetLightRange(float p_light_range);
    float GetLightRange();

    void SetDrawLayer(int p_Draw_Layer);
    int GetDrawLayer();

    void SetBackAlpha(const float f);
    inline float GetBackAlpha() const { return back_alpha; }

    void SetSBS3D(const bool b);
    inline bool GetSBS3D() const { return sbs3d; }

    void SetReverse3D(const bool b);
    inline bool GetReverse3D() const { return reverse3d; }

    void SetOU3D(const bool b);
    inline bool GetOU3D() const { return ou3d; }

    void SetTexLinear(const bool b);
    inline bool GetTexLinear() const { return tex_linear; }

    void SetTexMipmap(const bool b);
    inline bool GetTexMipmap() const { return tex_mipmap; }

    void SetTexClamp(const bool b);
    inline bool GetTexClamp() const { return tex_clamp; }

    void SetTexPremultiply(const bool b);
    inline bool GetTexPreMultiply() const { return tex_premultiply; }

    void SetTexAlpha(const QString s);
    inline QString GetTexAlpha() const { return tex_alpha; }

    void SetTexColorspace(const QString s);
    inline QString GetTexColorspace() const { return tex_colorspace; }

    void SetTex(const QString s, int i);
    inline QString GetTex(const int i) const { return tex[i]; }

    void SetMTL(const QString s);
    inline QString GetMTL() const { return mtl; }

    void SetSampleRate(const int i);
    inline int GetSampleRate() const { return sample_rate; }

    void SetAutoPlay(const bool b);
    inline bool GetAutoPlay() const { return auto_play; }

    void SetWidth(const int i);
    inline int GetWidth() const { return width; }

    void SetHeight(const int i);
    inline int GetHeight() const { return height; }

    void SetLocked(const bool b);
    inline bool GetLocked() const { return locked; }

    void SetNearDist(const float f);
    inline float GetNearDist() const { return near_dist; }

    void SetFarDist(const float f);
    inline float GetFarDist() const { return far_dist; }

    void SetTeleportMinDist(const float f);
    inline float GetTeleportMinDist() const { return teleport_min_dist; }

    void SetTeleportMaxDist(const float f);
    inline float GetTeleportMaxDist() const { return teleport_max_dist; }

    void SetCullFace(const QString s);
    inline QString GetCullFace() const { return cull_face; }

    void SetReloaded(const bool b);
    inline bool GetReloaded() const { return reloaded; }

    void SetCircular(const bool b);
    inline bool GetCircular() const { return circular; }

    void SetUserID(const QString s);
    inline QString GetUserID() const { return userid; }

    void SetEyePoint(const QVector3D v);
    QVector3D GetEyePoint() const { return eye_point; }

    void SetLocalHeadPos(const QVector3D &p);
    void SetLocalHeadPos(ScriptableVector *&p);
    ScriptableVector * GetLocalHeadPos();

    void SetGlobalHeadPos(const QVector3D &p);
    void SetGlobalHeadPos(ScriptableVector *&p);
    ScriptableVector * GetGlobalHeadPos();

    void SetEyePos(const QVector3D &p);
    void SetEyePos(ScriptableVector *&p);
    ScriptableVector * GetEyePos();   

    void SetServer(const QString s);
    QString GetServer() const { return server; }

    void SetServerPort(const int i);
    int GetServerPort() const { return port; }

    void SetSaveToMarkup(const bool b);
    inline bool GetSaveToMarkup() const { return save_to_markup; }

    void SetWalkSpeed(const float f);
    inline float GetWalkSpeed() const { return walk_speed; }

    void SetRunSpeed(const float f);
    inline float GetRunSpeed() const { return run_speed; }

    void SetAutoLoadTriggered(const bool b);
    inline bool GetAutoLoadTriggered() const { return auto_load_triggered; }

    void SetTeleportOverride(const bool b);
    inline bool GetTeleportOverride() const { return teleport_override; }

    void SetCursorVisible(const bool b);
    inline bool GetCursorVisible() const { return cursor_visible; }


    void SetViewDir(const QVector3D & v);
    void SetViewDir(ScriptableVector * v);
    ScriptableVector * GetViewDir();

    void SetUpDir(const QVector3D & v);
    void SetUpDir(ScriptableVector * v);
    ScriptableVector * GetUpDir();

    void SetCursor0Active(const bool b);
    bool GetCursor0Active() const;

    void SetCursor0Object(const QString & s);
    QString GetCursor0Object() const;

    void SetCursor0Pos(const QVector3D &p);
    void SetCursor0Pos(ScriptableVector *&p);
    ScriptableVector * GetCursor0Pos();

    void SetCursor0XDir(const QVector3D &p);
    void SetCursor0XDir(ScriptableVector *&p);
    ScriptableVector * GetCursor0XDir();

    void SetCursor0YDir(const QVector3D &p);
    void SetCursor0YDir(ScriptableVector *&p);
    ScriptableVector * GetCursor0YDir();

    void SetCursor0ZDir(const QVector3D &p);
    void SetCursor0ZDir(ScriptableVector *&p);
    ScriptableVector * GetCursor0ZDir();

    void SetCursor1Active(const bool b);
    bool GetCursor1Active() const;

    void SetCursor1Object(const QString & s);
    QString GetCursor1Object() const;

    void SetCursor1Pos(const QVector3D &p);
    void SetCursor1Pos(ScriptableVector *&p);
    ScriptableVector * GetCursor1Pos();

    void SetCursor1XDir(const QVector3D &p);
    void SetCursor1XDir(ScriptableVector *&p);
    ScriptableVector * GetCursor1XDir();

    void SetCursor1YDir(const QVector3D &p);
    void SetCursor1YDir(ScriptableVector *&p);
    ScriptableVector * GetCursor1YDir();

    void SetCursor1ZDir(const QVector3D &p);
    void SetCursor1ZDir(ScriptableVector *&p);
    ScriptableVector * GetCursor1ZDir();

    void SetHand0Active(const bool b);
    bool GetHand0Active();

    void SetHand0Trackpad(const QVector3D &p);
    void SetHand0Trackpad(ScriptableVector *&p);
    ScriptableVector * GetHand0Trackpad();

    void SetHand0Pos(const QVector3D &p);
    void SetHand0Pos(ScriptableVector *&p);
    ScriptableVector * GetHand0Pos();

    void SetHand0Vel(const QVector3D &p);
    void SetHand0Vel(ScriptableVector *&p);
    ScriptableVector * GetHand0Vel();

    void SetHand0XDir(const QVector3D &p);
    void SetHand0XDir(ScriptableVector *&p);
    ScriptableVector * GetHand0XDir();

    void SetHand0YDir(const QVector3D &p);
    void SetHand0YDir(ScriptableVector *&p);
    ScriptableVector * GetHand0YDir();

    void SetHand0ZDir(const QVector3D &p);
    void SetHand0ZDir(ScriptableVector *&p);
    ScriptableVector * GetHand0ZDir();

    void SetHand1Active(const bool b);
    bool GetHand1Active();

    void SetHand1Trackpad(const QVector3D &p);
    void SetHand1Trackpad(ScriptableVector *&p);
    ScriptableVector * GetHand1Trackpad();

    void SetHand1Pos(const QVector3D &p);
    void SetHand1Pos(ScriptableVector *&p);
    ScriptableVector * GetHand1Pos();

    void SetHand1Vel(const QVector3D &p);
    void SetHand1Vel(ScriptableVector *&p);
    ScriptableVector * GetHand1Vel();

    void SetHand1XDir(const QVector3D &p);
    void SetHand1XDir(ScriptableVector *&p);
    ScriptableVector * GetHand1XDir();

    void SetHand1YDir(const QVector3D &p);
    void SetHand1YDir(ScriptableVector *&p);
    ScriptableVector * GetHand1YDir();

    void SetHand1ZDir(const QVector3D &p);
    void SetHand1ZDir(ScriptableVector *&p);
    ScriptableVector * GetHand1ZDir();

    QString GetHMDType();
    void SetHMDType(const QString &s);

    QString GetDeviceType();
    void SetDeviceType(const QString &s);

    static QString ElementTypeToString(const ElementType t);
    static QString ElementTypeToTagName(const ElementType t);
    static ElementType StringToElementType(const QString name);

    Q_INVOKABLE int valueOf() const;

    //scripting
    QString onclick_code;
    QString oncollision;
    QString onenter;
    QString onexit;      

    int GetRoomObjectUUID() const;
    void SetRoomObjectUUID(int value);

    QString GetUseLocalAsset() const;
    void SetUseLocalAsset(const QString &value);

    bool GetFog() const;
    void SetFog(bool value);

    QString GetFogMode() const;
    void SetFogMode(const QString &value);

    float GetFogDensity() const;
    void SetFogDensity(float value);

    float GetFogStart() const;
    void SetFogStart(float value);

    float GetFogEnd() const;
    void SetFogEnd(float value);

    ScriptableVector *GetFogCol() const;
    void SetFogCol(const QVector4D & c);
    void SetFogCol(ScriptableVector *value);

    float GetGrabDist() const;
    void SetGrabDist(float value);

    bool GetPartyMode() const;
    void SetPartyMode(bool value);   

    bool GetTexCompress() const;
    void SetTexCompress(bool value);

    bool GetTriggered() const;
    void SetTriggered(bool value);   

    bool GetHighlighted() const;
    void SetHighlighted(bool value);

    bool GetPrimitive() const;
    void SetPrimitive(bool value);

    float GetGravity() const;
    void SetGravity(float value);

    float GetJumpVelocity() const;
    void SetJumpVelocity(float value);

    QPair<QVector3D, QVector3D> GetResetVolume() const;
    void SetResetVolume(const QPair<QVector3D, QVector3D> &value);

    ScriptableVector *GetBackCol() const;
    void SetBackCol(const QVector4D & c);
    void SetBackCol(ScriptableVector *value);

    ScriptableVector *GetTextCol() const;
    void SetTextCol(const QVector4D & c);
    void SetTextCol(ScriptableVector *value);

    QRectF GetTriggerRect() const;
    void SetTriggerRect(const QRectF &value);

    bool GetPlayOnce() const;
    void SetPlayOnce(bool value);   

    int GetCurMount() const;
    void SetCurMount(int value);

    bool GetReady() const;
    void SetReady(bool value);

    float GetProgress() const;
    void SetProgress(float value);

    bool GetReadyForScreenshot() const;
    void SetReadyForScreenshot(bool value);   

    bool GetStartedAutoPlay() const;
    void SetStartedAutoPlay(bool value);

    void SetXDirs(const QString & x, const QString & y, const QString & z);
    void SetXDirs(const QVector3D & x, const QVector3D & y, const QVector3D & z);

protected:

    //assets
    QString base_url;
    QString src;
    QString vertex_src;
    QString src_url;
    bool ou3d;
    bool sbs3d;
    bool reverse3d;

    bool tex_linear;
    bool tex_mipmap;
    bool tex_clamp;
    bool tex_premultiply;
    bool tex_compress;
    QString tex_alpha;
    QString tex_colorspace;
    QVector <QString> tex;
    QString mtl;

    int sample_rate;
    bool auto_play;

    int width;
    int height;

    bool save_to_markup;

    //room
    bool locked;
    bool party_mode;
    float gravity;
    float jump_velocity;
    float walk_speed;
    float run_speed;
    bool fog;
    QString fog_mode;
    float fog_density;
    float fog_start;
    float fog_end;
    ScriptableVector * fog_col;
    QString room_id;
    float near_dist;
    float far_dist;
    float teleport_min_dist;
    float teleport_max_dist;
    bool reloaded;
    QString server;
    int port;
    bool teleport_override;
    bool cursor_visible;
    QPair <QVector3D, QVector3D> reset_volume;
    int room_object_uuid;
    QString use_local_asset;
    float grab_dist;
    bool ready;
    bool ready_for_screenshot;
    bool translator_busy;
    bool started_auto_play;

    //player
    QString userid;    
    QVector3D eye_point;
    ScriptableVector * local_head_pos; //DK2 tracked head position
    ScriptableVector * head_pos; //DK2 tracked head position
    ScriptableVector * eye_pos;
    ScriptableVector * view_dir;
    ScriptableVector * up_dir;

    bool cursor0_active;
    QString cursor0_object; //JSID of object cursor is on
    ScriptableVector * cursor0_pos;
    ScriptableVector * cursor0_xdir;
    ScriptableVector * cursor0_ydir;
    ScriptableVector * cursor0_zdir;
    bool cursor1_active;
    QString cursor1_object; //JSID of object cursor is on
    ScriptableVector * cursor1_pos;
    ScriptableVector * cursor1_xdir;
    ScriptableVector * cursor1_ydir;
    ScriptableVector * cursor1_zdir;
    bool hand0_active;
    ScriptableVector * hand0_trackpad;
    ScriptableVector * hand0_pos;
    ScriptableVector * hand0_vel;
    ScriptableVector * hand0_xdir;
    ScriptableVector * hand0_ydir;
    ScriptableVector * hand0_zdir;
    bool hand1_active;
    ScriptableVector * hand1_trackpad;
    ScriptableVector * hand1_pos;
    ScriptableVector * hand1_vel;
    ScriptableVector * hand1_xdir;
    ScriptableVector * hand1_ydir;
    ScriptableVector * hand1_zdir;    
    QString hmd_type;
    QString device_type;

    //objects
    bool interpolate;
    ScriptableVector * pos;
    ScriptableVector * xdir;
    ScriptableVector * ydir;
    ScriptableVector * zdir;
    ScriptableVector * rotation;
    QString rotation_order;
    ScriptableVector * vel;
    ScriptableVector * accel;
    ScriptableVector * col;
    ScriptableVector * chromakey_col;
    ScriptableVector * scale;

    ScriptableVector * rand_pos;
    ScriptableVector * rand_vel;
    ScriptableVector * rand_accel;
    ScriptableVector * rand_col;
    ScriptableVector * rand_scale;

    ScriptableVector * tiling;

    int uuid; // unique identifier for each object
    QString js_id;
    bool interface_object;
    QString id;
    QString m_cubemap_radiance_id;
    QString m_cubemap_irradiance_id;
    QString blend0_id;
    QString blend1_id;
    QString blend2_id;
    QString blend3_id;
    float blend0;
    float blend1;
    float blend2;
    float blend3;

    QString cull_face;

    bool lighting;
    bool visible;

    bool triggered;
    QRectF rect;
    bool play_once;

    int cur_mount;

    float progress;

    //collision related
    QString collision_id;
    bool collision_static;
    bool collision_trigger;
    float collision_ccdmotionthreshold;
    float collision_ccdsweptsphereradius;
    float collision_radius;
    bool collision_response;
    float collision_friction;
    float collision_rollingfriction;
    float collision_restitution;
    float collision_angulardamping;
    float collision_lineardamping;
    ScriptableVector * collision_pos;
    ScriptableVector * collision_scale;

    QString blend_src;
    QString blend_dest;

    bool emit_local;
    QString emitter_id;

    bool loop;
    float pitch;
    float gain;
    float doppler_factor;
    float outer_gain;
    float inner_angle;
    float outer_angle;

    float back_alpha;
    int font_size;

    ScriptableVector * back_col;
    ScriptableVector * text_col;

    ElementType type;

    // object   
    bool circular;
    bool highlighted;
    bool auto_load_triggered;

    QString text; //paragraph/text

    // property flags
    bool text_changed;

    QString websurface_id;
    QString teleport_id;
    QString video_id;
    QString thumb_id;
    QString shader_id;
    QString image_id;
    QString lightmap_id;
    ScriptableVector * lightmap_scale;
    QString bone_id;

    QString head_id;
    QString body_id;

    bool sync;

    QString anim_id;
    float anim_speed;

    QString url;
    bool url_changed;
    QString url_orig;
    QString title;
    bool auto_load;
    bool draw_text;
    bool open;
    bool mirror;
    bool active;

    int count;
    int rate;
    float duration;
    float fade_in;
    float fade_out;

    bool dirty;
    bool primitive;

    float current_time;
    float total_time;

    ScriptableVector * spinaxis;
    float spinval;

    // Light Related Properites
    float m_light_intensity;
    float m_light_cone_angle;
    float m_light_cone_exponent;
    float m_light_range;

    // Draw Priority
    int m_draw_layer;

    ScriptableVector * emitter_pos;

    bool swallow;

public slots:

    void appendChild(DOMNode *);
    DOMNode * removeChild(QString);
    DOMNode * removeChild(DOMNode *);

private:

    //Connections
    QPointer <DOMNode> parent_node;
    QList <QPointer <DOMNode> > children_nodes;
    QPointer <DOMNode> left_sibling, right_sibling;

    static int next_uuid;
};

Q_DECLARE_METATYPE(QList <DOMNode *>)
#endif // DOMNODE_H

#ifndef ASSETSHADER_H
#define ASSETSHADER_H

#include <QtNetwork>
#include <QtGui>

#include "asset.h"
#include "mathutil.h"
#include "webasset.h"
#include "renderer.h"

class AssetShader : public Asset
{
    Q_OBJECT

public:

    AssetShader();

    void SetSrc(const QString & base, const QString & src, const QString & vertex_src);

    void Load();
    void Unload();

    QString GetVertexURL() const;
    QString GetVertexFullURL() const;

    void SetOverrideTexture(const bool b);
    bool GetOverrideTexture() const;

    void SetOverrideLightmap(const bool b);
    bool GetOverrideLightmap() const;

    bool GetLoaded() const;
    bool GetFinished() const;
    bool GetCompiled();
    void CompileShaderProgram();
    QPointer <ProgramHandle> GetProgramHandle();
    AbstractRenderComandShaderData GetARCData();
    AssetShader_Frame GetFrameUniforms();
    AssetShader_Room GetRoomUniforms();
    AssetShader_Object GetObjectUniforms();
    AssetShader_Material GetMaterialUniforms();

    void UpdateFrameUniforms(const AssetShader_Frame& p_room_uniforms);
    void UpdateRoomUniforms(const AssetShader_Room& p_room_uniforms);
    void UpdateObjectUniforms(const AssetShader_Object& p_room_uniforms);
    void UpdateMaterialUniforms(const AssetShader_Material& p_room_uniforms);

    void UpdateFrameUniforms();
    void UpdateRoomUniforms();
    void UpdateObjectUniforms();

    void SetRoomMatrix(const QMatrix4x4& p_iRoomMatrix);

    void SetConstColour(const QVector4D & c);
    void SetChromaKeyColour(const QVector4D & c);
    void SetRenderLeftEye(const bool b);
    void SetPlayerPosition(const QVector3D & p);
    void SetUseTextureAll(const bool b, const bool isHdr = false, const bool isFlipped = false);
    void SetUseTexture(const int index, const bool b, const bool isHdr = false, const bool isFlipped = false);
    void SetUseCubeTextureAll(const bool b);
    void SetUseCubeTexture0(const bool b);
    void SetUseCubeTexture1(const bool b);
    void SetUseCubeTexture2(const bool b);
    void SetUseLighting(const bool b);    
    bool GetUseLighting() const;
    void SetUseClipPlane(const bool b);
    bool GetUseClipPlane();
    void SetClipPlane(const QVector4D & v);        
    void SetFogEnabled(const bool b);
    void SetFogMode(const int i);
    void SetFogDensity(const float f);
    void SetFogStart(const float f);
    void SetFogEnd(const float f);
    void SetFogColour(const QColor & c);
    void SetUseSkelAnim(const bool b);
    void SetSkelAnimJoints(const QVector <QMatrix4x4> &m);
    void SetAmbient(const QVector3D v);
    void SetDiffuse(const QVector3D v);
    void SetSpecular(const QVector3D v);
    void SetShininess(const float f);
    void SetEmission(const QVector3D v);
	void SetLightmapScale(const QVector4D v);    
	void SetTiling(const QVector4D v);

    void SetProgramHandle(QPointer<ProgramHandle> p_shader_program);
    void SetDrawLayer(const int32_t p_draw_layer);
    void SetRoomSpacePositionAndDistance(const QVector4D p_room_space_position_and_distance);
    void SetDiffuse(const QVector4D v);
private:

    QUrl vertex_src_url;
    QString vertex_src_url_str;

    bool override_texture;
    bool override_lightmap;

    WebAsset vertex_webasset;

    QPointer<ProgramHandle> m_program_handle;

    AssetShader_Frame mFrame;
    AssetShader_Room mRoom;
    AssetShader_Object mObject;
    AssetShader_Material mMaterial;

    QTime time;

    bool GetCompiledNoCompile() const;
};


#endif // ASSETSHADER_H

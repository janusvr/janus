#include "assetshader.h"

AssetShader::AssetShader() :
    override_texture(false),
    override_lightmap(false),
    m_program_handle(nullptr)
{       
    props->SetType(TYPE_ASSETSHADER);
    time.start();
}

void AssetShader::SetSrc(const QString & base, const QString & src, const QString & vertex_src)
{    
    Asset::SetSrc(base, src);
    vertex_src_url_str = vertex_src;
    if (!vertex_src.isEmpty()) {
        vertex_src_url = QUrl(props->GetBaseURL()).resolved(vertex_src);
    }
//    qDebug() << "AssetShader::SetSrc" << this << GetS("_src_url") << vertex_src_url;
}

void AssetShader::Load()
{
//    qDebug() << "AssetShader::Load()" << this << GetS("_src_url");
    if (!props->GetSrcURL().isEmpty()) {
        WebAsset::Load(QUrl(props->GetSrcURL()));
    }
    else {
        SetLoaded(true);
    }

    if (vertex_src_url_str.length() > 0) {
        auto vertex_src_url_string = vertex_src_url.toString();
        vertex_webasset.Load(vertex_src_url);
    }
    else {
        vertex_webasset.SetLoaded(true);
    }
}

void AssetShader::Unload()
{
    WebAsset::Unload();
    vertex_webasset.Unload();
    m_program_handle.clear();
}

QString AssetShader::GetVertexURL() const
{
    return vertex_src_url_str;
}

QString AssetShader::GetVertexFullURL() const
{
    return vertex_src_url.toString();
}

void AssetShader::SetOverrideTexture(const bool b)
{
    override_texture = b;
}

bool AssetShader::GetOverrideTexture() const {
    return override_texture;
}

void AssetShader::SetOverrideLightmap(const bool b)
{
    override_lightmap = b;
}

bool AssetShader::GetOverrideLightmap() const
{
    return override_lightmap;
}

bool AssetShader::GetLoaded() const
{
    return WebAsset::GetLoaded() && vertex_webasset.GetLoaded();
}

bool AssetShader::GetFinished() const
{
    return WebAsset::GetLoaded() && vertex_webasset.GetLoaded();
}

bool AssetShader::GetCompiled()
{
    CompileShaderProgram();
    return (m_program_handle != nullptr);
}

bool AssetShader::GetCompiledNoCompile() const
{
    return (m_program_handle != nullptr);
}

void AssetShader::CompileShaderProgram()
{    
    if (!GetLoaded() || !vertex_webasset.GetLoaded() || GetCompiledNoCompile()) {
        return;
    }      
//    qDebug() << "AssetShader::CompileShaderProgram()"  << this;
    QByteArray fragment_src = QByteArray(GetData());
    QByteArray vertex_src = QByteArray(vertex_webasset.GetData());
    m_program_handle = Renderer::m_pimpl->CompileAndLinkShaderProgram(vertex_src, vertex_src_url_str, fragment_src, props->GetSrcURL());
}

void AssetShader::SetProgramHandle(QPointer<ProgramHandle> p_shader_program)
{
    m_program_handle = p_shader_program;
}

QPointer <ProgramHandle> AssetShader::GetProgramHandle()
{
    if (!GetCompiled())
    {
        CompileShaderProgram();
    }
    return m_program_handle;
}

AbstractRenderComandShaderData AssetShader::GetARCData()
{
    return AbstractRenderComandShaderData(GetProgramHandle(), mFrame, mRoom, mObject, mMaterial);
}

void AssetShader::SetRenderLeftEye(const bool b)
{
    mFrame.iLeftEye.setX(b ? 1.0f : 0.0f);
}

void AssetShader::SetConstColour(const QVector4D & c) {    
    mObject.iConstColour[0] = c.x();
    mObject.iConstColour[1] = c.y();
    mObject.iConstColour[2] = c.z();
    mObject.iConstColour[3] = c.w();
}

void AssetShader::SetChromaKeyColour(const QVector4D & c) {
    mObject.iChromaKeyColour[0] = c.x();
    mObject.iChromaKeyColour[1] = c.y();
    mObject.iChromaKeyColour[2] = c.z();
    mObject.iChromaKeyColour[3] = c.w();
}

void AssetShader::SetPlayerPosition(const QVector3D & p) {
    mRoom.iPlayerPosition = p;
    mRoom.iPlayerPosition.setW(1.0f);
}

void AssetShader::SetUseTextureAll(const bool b, const bool isHdr, const bool isFlipped)
{
    for (unsigned int i=0; i<ASSETSHADER_NUM_TEXTURES; ++i) {
		mMaterial.iUseTexture[i] = isHdr ? (b ? 2 : 0) : (b ? 1 : 0);
        if (b && isFlipped) {
            mMaterial.iUseTexture[i] += 2;
        }
    }
}

void AssetShader::SetUseTexture(const int index, const bool b, const bool isHdr, const bool isFlipped)
{
    mMaterial.iUseTexture[index] = isHdr ?  (b ? 2 : 0) : (b ? 1 : 0);
    if (b && isFlipped) {
        mMaterial.iUseTexture[index] += 2;
    }
}

void AssetShader::SetUseCubeTextureAll(const bool b)
{
    const unsigned int total_tex = ASSETSHADER_NUM_TEXTURES+ASSETSHADER_NUM_CUBEMAPS;
    for (unsigned int i=ASSETSHADER_NUM_TEXTURES; i<total_tex; ++i) {
        mMaterial.iUseTexture[i] = b ? 1 : 0;
    }
}

void AssetShader::SetUseCubeTexture0(const bool b)
{
    mMaterial.iUseTexture[ASSETSHADER_NUM_TEXTURES + 0] = b ? 1 : 0;
}

void AssetShader::SetUseCubeTexture1(const bool b)
{
    mMaterial.iUseTexture[ASSETSHADER_NUM_TEXTURES + 1] = b ? 1 : 0;
}

void AssetShader::SetUseCubeTexture2(const bool b)
{
    mMaterial.iUseTexture[ASSETSHADER_NUM_TEXTURES + 2] = b ? 1 : 0;
}

void AssetShader::SetUseClipPlane(const bool b)
{
    mRoom.iUseClipPlane.setX(b ? 1 : 0);
}

bool AssetShader::GetUseClipPlane()
{
    return mRoom.iUseClipPlane.x() > 0.5f;
}

void AssetShader::SetClipPlane(const QVector4D & v)
{
    mRoom.iClipPlane = v;
}

void AssetShader::SetUseLighting(const bool b)
{
    mObject.iUseFlags[1] = (b ? 1.0f : 0.0f);
}

bool AssetShader::GetUseLighting() const
{
    return (mObject.iUseFlags[1] == 1.0f);
}

void AssetShader::UpdateFrameUniforms()
{
    const float s = time.elapsed() * 0.001f;
    mFrame.iMouse = QVector4D(0,0,0,0); //TODO
    mFrame.iResolution = QVector4D(static_cast<float>(Renderer::m_pimpl->GetWindowWidth()), static_cast<float>(Renderer::m_pimpl->GetWindowHeight()), 0.0f, 0.0f);
    mFrame.iGlobalTime = QVector4D(s, s, s, s);
}

void AssetShader::UpdateFrameUniforms(const AssetShader_Frame& p_frame_uniforms)
{
    mFrame = p_frame_uniforms;
}

void AssetShader::UpdateRoomUniforms(const AssetShader_Room& p_room_uniforms)
{
    mRoom = p_room_uniforms;
}

void AssetShader::UpdateObjectUniforms(const AssetShader_Object& p_object_uniforms)
{
    mObject = p_object_uniforms;
}

void AssetShader::UpdateMaterialUniforms(const AssetShader_Material& p_material_uniforms)
{
    mMaterial = p_material_uniforms;
}

AssetShader_Frame AssetShader::GetFrameUniforms()
{
    return mFrame;
}

AssetShader_Room AssetShader::GetRoomUniforms()
{
    // TODO: Move the room "state" to be held in a single global variable
    // that can be queried for this struct rather than each shader holding
    // its own copy of the uniforms. The current setup makes it hard to keep
    // all shader instances in sync.
    SetRoomMatrix(MathUtil::RoomMatrix());
    return mRoom;
}

AssetShader_Object AssetShader::GetObjectUniforms()
{
    return mObject;
}

AssetShader_Material AssetShader::GetMaterialUniforms()
{
    return mMaterial;
}

void AssetShader::UpdateRoomUniforms()
{
    memcpy(mRoom.iRoomMatrix, MathUtil::RoomMatrix().constData(), 16 * sizeof(float));
}

void AssetShader::UpdateObjectUniforms()
{
    memcpy(mObject.iModelMatrix, MathUtil::ModelMatrix().constData(), 16 * sizeof(float));
}

void AssetShader::SetFogEnabled(const bool b)
{
    mRoom.iFogEnabled.setX(int(b));
}

void AssetShader::SetFogMode(const int i)
{
    mRoom.iFogMode.setX(i);
}

void AssetShader::SetFogDensity(const float f)
{
    mRoom.iFogDensity.setX(f);
}

void AssetShader::SetFogStart(const float f)
{
    mRoom.iFogStart.setX(f);
}

void AssetShader::SetFogEnd(const float f)
{
    mRoom.iFogEnd.setX(f);
}

void AssetShader::SetFogColour(const QColor & c)
{
    mRoom.iFogCol = QVector4D(c.redF(), c.greenF(), c.blueF(), c.alphaF());
}

void AssetShader::SetRoomMatrix(const QMatrix4x4 & p_iRoomMatrix)
{
    memcpy(mRoom.iRoomMatrix, p_iRoomMatrix.constData(), 16*sizeof(float));
}

void AssetShader::SetDrawLayer(const int32_t p_draw_layer)
{
    mObject.m_draw_layer = p_draw_layer;
}

void AssetShader::SetRoomSpacePositionAndDistance(const QVector4D p_room_space_position_and_distance)
{
    mObject.m_room_space_position_and_distance = p_room_space_position_and_distance;
}

void AssetShader::SetUseSkelAnim(const bool b)
{
    mObject.iUseFlags[0] = b ? 1.0f : 0.0f;
}

void AssetShader::SetSkelAnimJoints(const QVector <QMatrix4x4> & m)
{
    const int matrix_count = m.size();
    mObject.iSkelAnimJoints.clear();
    mObject.iSkelAnimJoints.resize(matrix_count * 16);

    // Annoyingly QMatrix4x4 has flags in it's data members which means
    // we can't just copy the entire array, we need to pick out only the matrix
    // floats from each QMatrix4x4
    for (int i = 0; i < matrix_count; i++)
    {
        memcpy(&(mObject.iSkelAnimJoints[i*16]), m[i].constData(), 16 * sizeof(float));
    }
}

void AssetShader::SetAmbient(const QVector3D v)
{    
    mMaterial.iAmbient.setX(v.x());
    mMaterial.iAmbient.setY(v.y());
    mMaterial.iAmbient.setZ(v.z());
}

void AssetShader::SetDiffuse(const QVector3D v)
{
    mMaterial.iDiffuse.setX(v.x());
    mMaterial.iDiffuse.setY(v.y());
    mMaterial.iDiffuse.setZ(v.z());
}

void AssetShader::SetDiffuse(const QVector4D v)
{
    mMaterial.iDiffuse.setX(v.x());
    mMaterial.iDiffuse.setY(v.y());
    mMaterial.iDiffuse.setZ(v.z());
    mMaterial.iDiffuse.setW(v.w());
}

void AssetShader::SetSpecular(const QVector3D v)
{
    mMaterial.iSpecular.setX(v.x());
    mMaterial.iSpecular.setY(v.y());
    mMaterial.iSpecular.setZ(v.z());
}

void AssetShader::SetShininess(const float f)
{
    mMaterial.iShininess.setX(f);
}

void AssetShader::SetEmission(const QVector3D v)
{
	mMaterial.iEmission.setX(v.x());
	mMaterial.iEmission.setY(v.y());
	mMaterial.iEmission.setZ(v.z());
}

void AssetShader::SetLightmapScale(const QVector4D v)
{
	mMaterial.iLightmapScale.setX(v.x());
	mMaterial.iLightmapScale.setY(v.y());
	mMaterial.iLightmapScale.setZ(v.z());
	mMaterial.iLightmapScale.setW(v.w());
}

void AssetShader::SetTiling(const QVector4D v)
{
	mMaterial.iTiling.setX(v.x());
	mMaterial.iTiling.setY(v.y());
	mMaterial.iTiling.setZ(v.z());
	mMaterial.iTiling.setW(v.w());
}

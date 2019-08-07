#include "mathutil.h"

QStringList MathUtil::img_extensions;
QStringList MathUtil::sound_extensions;
QStringList MathUtil::vid_extensions;
QStringList MathUtil::geom_extensions;
QStringList MathUtil::domain_extensions;

QString MathUtil::room_delete_code;

float MathUtil::_180_OVER_PI = 180.0f / 3.14159f;
float MathUtil::_PI_OVER_180 = 3.14159f / 180.0f;
float MathUtil::_PI = 3.14159f;
float MathUtil::_2_PI = 3.14159f * 2.0f;
float MathUtil::_PI_OVER_2 = 3.14159f / 2.0f;
float MathUtil::_PI_OVER_4 = 3.14159f / 4.0f;

QOpenGLExtraFunctions * MathUtil::glFuncs = nullptr;

QList <QMatrix4x4> MathUtil::modelmatrix_stack;
QMatrix4x4 MathUtil::m_roomMatrix;
QMatrix4x4 MathUtil::projectionmatrix;
QMatrix4x4 MathUtil::viewmatrix;
QString MathUtil::m_last_screenshot_path = QString("");
QStringList MathUtil::error_log_msgs;
QStringList MathUtil::error_log_msgs_temp;
QVariantList MathUtil::partymode_data;
uint64_t MathUtil::m_frame_limiter_render_thread = 1;
bool MathUtil::m_linear_framebuffer = false;
bool MathUtil::m_do_equi = false;

// Default constructor
AbstractRenderCommand::AbstractRenderCommand()
    : m_draw_id(0),
      m_camera_id(0),
      m_mesh_handle(nullptr),
      m_primitive_type(PrimitiveType::TRIANGLES),
      m_primitive_count(0),
      m_first_index(0),
      m_base_vertex(0),
      m_base_instance(0),
      m_shader(nullptr),
      m_texture_set(),
      m_frame_uniforms(),
      m_room_uniforms(),
      m_object_uniforms(),
      m_material_uniforms(),
      m_active_face_cull_mode(FaceCullMode::BACK),
      m_depth_func(DepthFunc::LEQUAL),
      m_depth_mask(DepthMask::DEPTH_WRITES_ENABLED),
      m_stencil_func(),
      m_stencil_op(),
      m_color_mask(ColorMask::COLOR_WRITES_ENABLED)
{
}

// Constructor
AbstractRenderCommand::AbstractRenderCommand(PrimitiveType p_primitive_type,
    GLuint p_primitive_count,
    GLuint p_first_index,
    GLuint p_base_vertex,
    GLuint p_base_instance,
    MeshHandle *p_mesh_handle,
    ProgramHandle *p_shader,
    AssetShader_Frame p_frame_uniforms,
    AssetShader_Room p_room_uniforms,
    AssetShader_Object p_object_uniforms,
    AssetShader_Material p_material_uniforms,
    TextureSet p_texture_set,
    FaceCullMode p_face_cull_mode,
    DepthFunc p_depth_func,
    DepthMask p_depth_mask,
    StencilFunc p_stencil_func,
    StencilOp p_stencil_op,
    ColorMask p_color_mask) :
    m_draw_id(0),
    m_camera_id(0),
    m_mesh_handle(p_mesh_handle),
    m_primitive_type(p_primitive_type),
    m_primitive_count(p_primitive_count),
    m_first_index(p_first_index),
    m_base_vertex(p_base_vertex),
    m_base_instance(p_base_instance),
    m_shader(p_shader),
    m_texture_set(p_texture_set),
    m_frame_uniforms(p_frame_uniforms),
    m_room_uniforms(p_room_uniforms),
    m_object_uniforms(p_object_uniforms),
    m_material_uniforms(p_material_uniforms),
    m_active_face_cull_mode(p_face_cull_mode),
    m_depth_func(p_depth_func),
    m_depth_mask(p_depth_mask),
    m_stencil_func(p_stencil_func),
    m_stencil_op(p_stencil_op),
    m_color_mask(p_color_mask)
{

}

// Copy constructor
AbstractRenderCommand::AbstractRenderCommand(const AbstractRenderCommand& p_copy) :
    m_draw_id(p_copy.m_draw_id),
    m_camera_id(p_copy.m_camera_id),
    m_primitive_type(p_copy.m_primitive_type),
    m_primitive_count(p_copy.m_primitive_count),
    m_first_index(p_copy.m_first_index),
    m_base_vertex(p_copy.m_base_vertex),
    m_base_instance(p_copy.m_base_instance),
    m_frame_uniforms(p_copy.m_frame_uniforms),
    m_room_uniforms(p_copy.m_room_uniforms),
    m_object_uniforms(p_copy.m_object_uniforms),
    m_material_uniforms(p_copy.m_material_uniforms),
    m_active_face_cull_mode(p_copy.m_active_face_cull_mode),
    m_depth_func(p_copy.m_depth_func),
    m_depth_mask(p_copy.m_depth_mask),
    m_stencil_func(p_copy.m_stencil_func),
    m_stencil_op(p_copy.m_stencil_op),
    m_color_mask(p_copy.m_color_mask)
{
    if (!p_copy.m_mesh_handle.isNull())
    {
        m_mesh_handle = p_copy.m_mesh_handle;
    }

    if (!p_copy.m_shader.isNull())
    {
        m_shader = p_copy.m_shader;
    }

    // Only copy the textures from the provided TextureSet for the valid iUseTexture indices
    // Making copies of QPointer is expensive so this loop saves us a lot of time
    for (int i = 0; i < ASSETSHADER_NUM_COMBINED_TEXURES; ++i)
    {
        if (m_material_uniforms.iUseTexture[i] != 0.0)
        {
            if (p_copy.m_texture_set.GetIs3DTexture() == false)
            {
                if (m_texture_set.GetTextureHandle(i, true) != p_copy.m_texture_set.GetTextureHandle(i, true))
                {
                    m_texture_set.SetTextureHandle(i, p_copy.m_texture_set.GetTextureHandle(i,true), 0); // both
                }
            }
            else
            {

                if (m_texture_set.GetTextureHandle(i, true) != p_copy.m_texture_set.GetTextureHandle(i, true))
                {
                    m_texture_set.SetTextureHandle(i, p_copy.m_texture_set.GetTextureHandle(i,true), 1); // Left
                }

                if (m_texture_set.GetTextureHandle(i, false) != p_copy.m_texture_set.GetTextureHandle(i, false))
                {
                    m_texture_set.SetTextureHandle(i, p_copy.m_texture_set.GetTextureHandle(i, false), 2); // Right
                }
            }
        }
    }
}

// Move constructor
AbstractRenderCommand::AbstractRenderCommand(AbstractRenderCommand&& p_move) :
    m_draw_id(std::move(p_move.m_draw_id)),
    m_camera_id(std::move(p_move.m_camera_id)),
    m_mesh_handle(std::move(p_move.m_mesh_handle)),
    m_primitive_type(std::move(p_move.m_primitive_type)),
    m_primitive_count(std::move(p_move.m_primitive_count)),
    m_first_index(std::move(p_move.m_first_index)),
    m_base_vertex(std::move(p_move.m_base_vertex)),
    m_base_instance(std::move(p_move.m_base_instance)),
    m_shader(std::move(p_move.m_shader)),
    m_texture_set(std::move(p_move.m_texture_set)),
    m_frame_uniforms(std::move(p_move.m_frame_uniforms)),
    m_room_uniforms(std::move(p_move.m_room_uniforms)),
    m_object_uniforms(std::move(p_move.m_object_uniforms)),
    m_material_uniforms(std::move(p_move.m_material_uniforms)),
    m_active_face_cull_mode(std::move(p_move.m_active_face_cull_mode)),
    m_depth_func(std::move(p_move.m_depth_func)),
    m_depth_mask(std::move(p_move.m_depth_mask)),
    m_stencil_func(std::move(p_move.m_stencil_func)),
    m_stencil_op(std::move(p_move.m_stencil_op)),
    m_color_mask(std::move(p_move.m_color_mask))
{

}

// Copy assignment
AbstractRenderCommand& AbstractRenderCommand::operator=(const AbstractRenderCommand& p_copy)
{
    m_primitive_type = p_copy.m_primitive_type;
    m_primitive_count = p_copy.m_primitive_count;
    m_first_index = p_copy.m_first_index;
    m_base_vertex = p_copy.m_base_vertex;
    m_base_instance = p_copy.m_base_instance;

    if (m_mesh_handle != p_copy.m_mesh_handle)
    {
        m_mesh_handle = p_copy.m_mesh_handle;
    }

    if (m_shader != p_copy.m_shader)
    {
        m_shader = p_copy.m_shader;
    }

    m_frame_uniforms = p_copy.m_frame_uniforms;
    m_room_uniforms = p_copy.m_room_uniforms;
    m_object_uniforms = p_copy.m_object_uniforms;
    m_material_uniforms = p_copy.m_material_uniforms;
    // Only copy the textures from the provided TextureSet for the valid iUseTexture indices
    // Making copies of QPointer is expensive so this loop saves us a lot of time
    for (int i = 0; i < ASSETSHADER_NUM_COMBINED_TEXURES; ++i)
    {
        if (m_material_uniforms.iUseTexture[i] != 0.0)
        {
            if (m_texture_set.GetTextureHandle(i, true) != p_copy.m_texture_set.GetTextureHandle(i, true))
            {
                m_texture_set.SetTextureHandle(i, p_copy.m_texture_set.GetTextureHandle(i,true), 1); // Left
            }

            if (m_texture_set.GetTextureHandle(i, false) != p_copy.m_texture_set.GetTextureHandle(i, false))
            {
                m_texture_set.SetTextureHandle(i, p_copy.m_texture_set.GetTextureHandle(i, false), 2); // Right
            }
        }
    }
    m_active_face_cull_mode = p_copy.m_active_face_cull_mode;
    m_depth_func = p_copy.m_depth_func;
    m_depth_mask = p_copy.m_depth_mask;
    m_stencil_func = p_copy.m_stencil_func;
    m_stencil_op = p_copy.m_stencil_op;
    m_color_mask = p_copy.m_color_mask;
    m_draw_id = p_copy.m_draw_id;
    m_camera_id = p_copy.m_camera_id;
    return *this;
}


// Move assignment
AbstractRenderCommand& AbstractRenderCommand::operator=(AbstractRenderCommand&& p_move)
{
    m_primitive_type = std::move(p_move.m_primitive_type);
    m_primitive_count = std::move(p_move.m_primitive_count);
    m_first_index = std::move(p_move.m_first_index);
    m_base_vertex = std::move(p_move.m_base_vertex);
    m_base_instance = std::move(p_move.m_base_instance);
    m_mesh_handle = std::move(p_move.m_mesh_handle);
    m_shader = std::move(p_move.m_shader);
    m_frame_uniforms = std::move(p_move.m_frame_uniforms);
    m_room_uniforms = std::move(p_move.m_room_uniforms);
    m_object_uniforms = std::move(p_move.m_object_uniforms);
    m_material_uniforms = std::move(p_move.m_material_uniforms);
    m_texture_set = std::move(p_move.m_texture_set);
    m_active_face_cull_mode = std::move(p_move.m_active_face_cull_mode);
    m_depth_func = std::move(p_move.m_depth_func);
    m_depth_mask = std::move(p_move.m_depth_mask);
    m_stencil_func = std::move(p_move.m_stencil_func);
    m_stencil_op = std::move(p_move.m_stencil_op);
    m_color_mask = std::move(p_move.m_color_mask);
    m_draw_id = std::move(p_move.m_draw_id);
    m_camera_id = std::move(p_move.m_camera_id);
    return *this;
}

// Destuctor
AbstractRenderCommand::~AbstractRenderCommand()
{

}

GLuint AbstractRenderCommand::GetPrimitiveCount() const
{
    return m_primitive_count;
}

GLuint AbstractRenderCommand::GetFirstIndex() const
{
    return m_first_index;
}

GLuint AbstractRenderCommand::GetBaseVertex() const
{
    return m_base_vertex;
}

void AbstractRenderCommand::SetObjectUniforms(AssetShader_Object p_object_uniforms)
{
    m_object_uniforms = p_object_uniforms;
}

QPointer <ProgramHandle> AbstractRenderCommand::GetShaderRef()
{
    return m_shader;
}

void AbstractRenderCommand::SetShader(QPointer <ProgramHandle> p_shader)
{
    m_shader = p_shader;
}

FaceCullMode AbstractRenderCommand::GetFaceCullMode() const
{
    return m_active_face_cull_mode;
}

TextureSet AbstractRenderCommand::GetTextureSet() const
{
    return m_texture_set;
}

DepthFunc AbstractRenderCommand::GetDepthFunc() const
{
    return m_depth_func;
}

DepthMask AbstractRenderCommand::GetDepthMask() const
{
    return m_depth_mask;
}

StencilFunc AbstractRenderCommand::GetStencilFunc() const
{
    return m_stencil_func;
}

StencilOp AbstractRenderCommand::GetStencilOp() const
{
    return m_stencil_op;
}

ColorMask AbstractRenderCommand::GetColorMask() const
{
    return m_color_mask;
}

VirtualCamera::VirtualCamera() :
    m_fov(90.0f),
    m_aspectRatio(1.0f),
    m_nearClip(0.0f),
    m_farClip(1.0f),
    m_is_left_eye(true)
{
    Initialize();
}

VirtualCamera::VirtualCamera(QMatrix4x4 p_viewMatrix, QVector4D p_viewport
                             , float p_aspectRatio, float p_fov, float p_nearClip, float p_farClip) :
    m_position(QVector3D(p_viewMatrix.column(3).toVector3D())),
    m_orientation(),
    m_scale(p_viewMatrix.column(0).length(), p_viewMatrix.column(1).length(), p_viewMatrix.column(2).length()),
    m_fov(p_fov),
    m_aspectRatio(p_aspectRatio),
    m_nearClip(p_nearClip),
    m_farClip(p_farClip),
    m_viewport(p_viewport),
    m_viewMatrix(p_viewMatrix)
{
    float x_scale = p_viewMatrix.column(0).toVector3D().length();
    float y_scale = p_viewMatrix.column(1).toVector3D().length();
    float z_scale = p_viewMatrix.column(2).toVector3D().length();

    m_scale.setX(x_scale);
    m_scale.setY(y_scale);
    m_scale.setZ(z_scale);

    m_position = p_viewMatrix.column(3).toVector3D();

    m_viewMatrix.setColumn(0, p_viewMatrix.column(0).normalized());
    m_viewMatrix.setColumn(1, p_viewMatrix.column(1).normalized());
    m_viewMatrix.setColumn(2, p_viewMatrix.column(2).normalized());

    QMatrix3x3 rotmatrix = m_viewMatrix.toGenericMatrix<3, 3>();

    m_orientation = QQuaternion::fromRotationMatrix(rotmatrix);

    Initialize();
    RecomputeViewMatrix();
    RecomputeProjectionMatrix();
}

VirtualCamera::VirtualCamera(QMatrix4x4 p_viewMatrix, QVector4D p_viewport, QMatrix4x4 p_projectionMatrix) :
    m_position(QVector3D(p_viewMatrix.column(3).toVector3D())),
    m_orientation(),
    m_scale(p_viewMatrix.column(0).length(), p_viewMatrix.column(1).length(), p_viewMatrix.column(2).length()),
    m_fov(90.0f),
    m_aspectRatio(1.0f),
    m_nearClip(1.0f),
    m_farClip(1000.0f),
    m_viewport(p_viewport),
    m_viewMatrix(p_viewMatrix),
    m_projectionMatrix(p_projectionMatrix)
{
    Initialize();
    // This is only used by the HMD's as they provide their custom projection matrix
}

VirtualCamera::VirtualCamera(QVector3D p_position, QQuaternion p_orientation, QVector3D p_scale, QVector4D p_viewport,
    float p_aspectRatio, float p_fov, float p_nearClip, float p_farClip) :
    m_position(p_position),
    m_orientation(p_orientation),
    m_scale(p_scale),
    m_fov(p_fov),
    m_aspectRatio(p_aspectRatio),
    m_nearClip(p_nearClip),
    m_farClip(p_farClip),
    m_viewport(p_viewport)
{
    Initialize();
    RecomputeViewMatrix();
    RecomputeProjectionMatrix();
}

VirtualCamera::~VirtualCamera()
{
}

void VirtualCamera::Initialize()
{
    for (int scope_enum = 0; scope_enum < static_cast<int>(RENDERER::RENDER_SCOPE::SCOPE_COUNT); ++scope_enum)
    {
        m_scope_mask[scope_enum] = true;
    }
    m_is_left_eye = true;
}

QVector3D VirtualCamera::GetPosition() const
{
    return m_position;
}

void VirtualCamera::SetPosition(QVector3D p_position)
{
    m_position = p_position;
    RecomputeViewMatrix();
}

QQuaternion VirtualCamera::GetOrientation() const
{
    return m_orientation;
}

void VirtualCamera::SetOrientation(QQuaternion p_orientation)
{
    m_orientation = p_orientation;
    RecomputeViewMatrix();
}

QVector3D VirtualCamera::GetScale() const
{
    return m_scale;
}

void VirtualCamera::SetScale(QVector3D p_scale)
{
    m_scale = p_scale;
    RecomputeViewMatrix();
}

float VirtualCamera::GetFOV() const
{
    return m_fov;
}

void VirtualCamera::SetFOV(float p_fov)
{
    m_fov = p_fov;
    RecomputeProjectionMatrix();
}

float VirtualCamera::GetAspectRatio() const
{
    return m_aspectRatio;
}

void VirtualCamera::SetAspectRatio(float p_aspectRatio)
{
    m_aspectRatio = p_aspectRatio;
    RecomputeProjectionMatrix();
}

float VirtualCamera::GetNearClip() const
{
    return m_nearClip;
}

void VirtualCamera::SetNearClip(float p_nearClip)
{
    m_nearClip = p_nearClip;
    RecomputeProjectionMatrix();
}

float VirtualCamera::GetFarClip() const
{
    return m_farClip;
}

void VirtualCamera::SetFarClip(float p_farClip)
{
    m_farClip = p_farClip;
    RecomputeProjectionMatrix();
}

QVector4D VirtualCamera::GetViewport() const
{
    return m_viewport;
}

void VirtualCamera::SetViewport(QVector4D p_viewport)
{
    m_viewport = p_viewport;
}

const QMatrix4x4& VirtualCamera::GetViewMatrix() const
{
    return m_viewMatrix;
}

const QMatrix4x4& VirtualCamera::GetProjectionMatrix() const
{
    return m_projectionMatrix;
}

bool VirtualCamera::GetScopeMask(RENDERER::RENDER_SCOPE const p_scope) const
{
    return m_scope_mask[static_cast<int>(p_scope)];
}

void VirtualCamera::SetScopeMask(RENDERER::RENDER_SCOPE const p_scope, bool const p_mask)
{
    if (p_scope == RENDERER::RENDER_SCOPE::ALL)
    {
        for (int i = 0; i < static_cast<int>(RENDERER::RENDER_SCOPE::SCOPE_COUNT); ++i)
        {
            m_scope_mask[i] = p_mask;
        }
    }
    else
    {
        m_scope_mask[static_cast<int>(p_scope)] = p_mask;
    }
}

bool VirtualCamera::GetLeftEye() const
{
    return m_is_left_eye;
}

void VirtualCamera::SetLeftEye(bool p_is_left_eye)
{
    m_is_left_eye = p_is_left_eye;
}

void VirtualCamera::RecomputeViewMatrix()
{
    m_viewMatrix.setToIdentity();
    m_viewMatrix.translate(m_position);
    m_viewMatrix.rotate(m_orientation);
    m_viewMatrix.scale(m_scale);
}

bool TextureSet::operator==(const TextureSet &rhs) const
{
    for (int i = 0; i < ASSETSHADER_NUM_COMBINED_TEXURES; ++i)
    {
        if (m_texture_handles_left[i] != rhs.m_texture_handles_left[i])
        {
            return false;
        }
    }

    for (int i = 0; i < ASSETSHADER_NUM_COMBINED_TEXURES; ++i)
    {
        if (m_texture_handles_right[i] != rhs.m_texture_handles_right[i])
        {
            return false;
        }
    }

    return true;
}

bool TextureSet::operator!=(const TextureSet &rhs) const
{
    return !(*this == rhs);
}

void TextureSet::SetTextureHandle(int p_index, TextureHandle *p_id, uint p_is_stereo_image)
{
    if (p_is_stereo_image == 0)
    {
        m_texture_handles_left[p_index] = p_id;
        m_texture_handles_right[p_index] = p_id;
        m_is_3D_texture = false;
    }
    else if (p_is_stereo_image == 1)
    {
        m_texture_handles_left[p_index] = p_id;
        m_is_3D_texture = true;
    }
    else
    {
        m_texture_handles_right[p_index] = p_id;
        m_is_3D_texture = true;
    }
}

TextureHandle::ALPHA_TYPE TextureSet::GetAlphaType(int p_index) const
{
    return (m_texture_handles_left[p_index]) ? m_texture_handles_left[p_index]->GetAlphaType() : TextureHandle::ALPHA_TYPE::NONE;
}

bool TextureSet::GetHasAlpha(int p_index) const
{
    return (m_texture_handles_left[p_index]) ? (m_texture_handles_left[p_index]->GetAlphaType() != TextureHandle::ALPHA_TYPE::NONE) : false;
}

QPointer<TextureHandle> TextureSet::GetTextureHandle(int p_index, bool p_is_left_texture) const
{
    return (p_is_left_texture) ? m_texture_handles_left[p_index] : m_texture_handles_right[p_index];
}

bool TextureSet::GetIs3DTexture() const
{
    return  m_is_3D_texture;
}

AssetShader_Object_Compact::AssetShader_Object_Compact()
{

}

AssetShader_Object_Compact::~AssetShader_Object_Compact()
{

}

AssetShader_Object_Compact::AssetShader_Object_Compact(const AssetShader_Object &rhs)
{
    for (int i = 0; i < 16; ++i)
    {
        iModelMatrix[i] = rhs.iModelMatrix[i];
        iViewMatrix[i] = rhs.iViewMatrix[i];
        iInverseViewMatrix[i] = rhs.iInverseViewMatrix[i];
        iProjectionMatrix[i] = rhs.iProjectionMatrix[i];
        iModelViewMatrix[i] = rhs.iModelViewMatrix[i];
        iModelViewProjectionMatrix[i] = rhs.iModelViewProjectionMatrix[i];
        iTransposeInverseModelMatrix[i] = rhs.iTransposeInverseModelMatrix[i];
        iTransposeInverseModelViewMatrix[i] = rhs.iTransposeInverseModelViewMatrix[i];
    }
}

AssetShader_Object::AssetShader_Object()
    : m_draw_layer(0),
      m_room_space_position_and_distance(0.0f, 0.0f, 0.0f, 1.0f)
{
    for (int i = 0; i < 4; ++i)
    {
        iConstColour[i] = 1.0f;
        iChromaKeyColour[i] = 0.0f;
        iUseFlags[i] = 0.0f;
        iMiscObjectData[i] = 0;
    }
}

AssetShader_Object::~AssetShader_Object()
{

}

AssetShader_Material::AssetShader_Material() :
    iAmbient(1.0f, 1.0f, 1.0f, 1.0f),
    iDiffuse(1.0f, 1.0f, 1.0f, 1.0f),
    iSpecular(0.35f, 0.35f, 0.35f, 1.0f),
    iShininess(20.0f, 0.0f, 0.0f, 0.0f),
    iEmission(0.0f, 0.0f, 0.0f, 0.0f),
    iTiling(1.0f, 1.0f, 0.0f, 0.0f),
    iLightmapScale(1.0f, 1.0f, 0.0f, 0.0f)
{
    for (int i = 0; i < 16; ++i)
    {
        iUseTexture[i] = 0.0f;
    }
}

AssetShader_Room::AssetShader_Room() :
    iPlayerPosition(0, 0, 0, 1),
    iUseClipPlane(0, 0, 0, 0),
    iClipPlane(0, 0, 0, 0),
    iFogEnabled(0, 0, 0, 0),
    iFogMode(1, 1, 1, 1),
    iFogDensity(1, 1, 1, 1),
    iFogStart(0, 0, 0, 0),
    iFogEnd(1, 1, 1, 1),
    iFogCol(0, 0, 0, 1)
{
    for (int i = 0; i < 16; ++i)
    {
        iRoomMatrix[i] = 0.0f;
        iMiscRoomData[i] = 0.0f;
    }
}

MathUtil::MathUtil()
{
}

MathUtil::~MathUtil()
{
    MathUtil::glFuncs = nullptr;
}

QByteArray MathUtil::loadFile(const QString fname)
{
    QFile aFile(fname);
    QByteArray data;
    if (!aFile.open(QIODevice::ReadOnly))
    {
        // Failed to open File
        qDebug() << "ERROR: Failed to open file" << fname;
    }
    else
    {
        data = aFile.readAll();
        aFile.close();

    }
    return data;
}

void MathUtil::printShaderError(GLuint const p_shader)
{
    GLint logLength = 0;
    MathUtil::glFuncs->glGetShaderiv(p_shader, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0)
    {
        GLchar *logMessage = new GLchar[logLength];
        if (MathUtil::glFuncs->glIsShader(p_shader) == GL_FALSE)
        {
            MathUtil::glFuncs->glGetProgramInfoLog(p_shader, logLength, &logLength, logMessage);
        }
        else
        {
            MathUtil::glFuncs->glGetShaderInfoLog(p_shader, logLength, &logLength, logMessage);
        }
        qDebug("Shader Info Log: /n%s", logMessage);
        delete[] logMessage;
    }
}

bool MathUtil::loadGLShaderFromFile(GLuint * const p_program, const QString vertName, const QString fragName)
{
    GLuint v = MathUtil::glFuncs->glCreateShader(GL_VERTEX_SHADER);
    GLuint f = MathUtil::glFuncs->glCreateShader(GL_FRAGMENT_SHADER);

    GLboolean programCheck = MathUtil::glFuncs->glIsShader(v);
    if (programCheck == GL_FALSE)
    {
        qDebug("ERROR: Pre-glShaderSource vertex Shader is not a valid shader: %u", v);
        return false;
    }
    programCheck = MathUtil::glFuncs->glIsShader(f);
    if (programCheck == GL_FALSE)
    {
        qDebug("ERROR: Pre-glShaderSource fragment shader is not a valid shader: %u", f);
        return false;
    }

    QByteArray vertexShaderSource = loadFile(vertName);
    QByteArray fragmentShaderSource = loadFile(fragName);
    GLint vlen = vertexShaderSource.size();
    GLint flen = fragmentShaderSource.size();
    char const * vs = vertexShaderSource.data();
    char const * fs = fragmentShaderSource.data();

    MathUtil::glFuncs->glShaderSource(v, 1, &vs, &vlen);
    MathUtil::glFuncs->glShaderSource(f, 1, &fs, &flen);

    programCheck = MathUtil::glFuncs->glIsShader(v);
    if (programCheck == GL_FALSE)
    {
        qDebug("ERROR: Post-glShaderSource vertex Shader is not a valid shader: %u", v);
        printShaderError(v);
        return false;
    }
    programCheck = MathUtil::glFuncs->glIsShader(f);
    if (programCheck == GL_FALSE)
    {
        qDebug("ERROR: Post-glShaderSource fragment shader is not a valid shader: %u", f);
        printShaderError(f);
        return false;
    }

    GLint compiled = 0;

    MathUtil::glFuncs->glCompileShader(v);
    MathUtil::glFuncs->glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
    if (compiled == static_cast<GLint>(GL_FALSE))
    {
        qDebug() << "ERROR: Vertex shader not compiled:" << vertName;
        printShaderError(v);
        return false;
    }
    else
    {
        qDebug() << "INFO: Vertex shader compiled sucessfully:" << vertName;
        printShaderError(v);
    }

    MathUtil::glFuncs->glCompileShader(f);
    MathUtil::glFuncs->glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);
    if (compiled == static_cast<GLint>(GL_FALSE))
    {
        qDebug() << "ERROR: Fragment shader not compiled:" << fragName;
        printShaderError(f);
        return false;
    }
    else
    {
        qDebug() << "INFO: Fragmentshader compiled sucessfully:" << fragName;
        printShaderError(f);
    }

    programCheck = MathUtil::glFuncs->glIsShader(v);
    if (programCheck == GL_FALSE)
    {
        qDebug("ERROR: Post-glCompileShader vertex Shader is not a valid shader: %u", v);
        printShaderError(v);
        return false;
    }
    programCheck = MathUtil::glFuncs->glIsShader(f);
    if (programCheck == GL_FALSE)
    {
        qDebug("ERROR: Post-glCompileShader fragment shader is not a valid shader: %u", f);
        printShaderError(f);
        return false;
    }

    GLuint p = MathUtil::glFuncs->glCreateProgram();
    programCheck = MathUtil::glFuncs->glIsProgram(p);
    if (programCheck == GL_FALSE)
    {
        qDebug("ERROR: Program is not a valid program: %u", p);
        printShaderError(p);
        return false;
    }

    MathUtil::glFuncs->glAttachShader(p, v);
    MathUtil::glFuncs->glAttachShader(p, f);

    MathUtil::glFuncs->glLinkProgram(p);
    GLint isLinked = static_cast<GLint>(GL_FALSE);
    MathUtil::glFuncs->glGetProgramiv(p, GL_LINK_STATUS, &isLinked);
    if (isLinked == static_cast<GLint>(GL_FALSE))
    {
        qDebug("ERROR: Program failed to link: %u", p);
        return false;
    }
    else
    {
        qDebug("INFO: Program linked sucessfully: %u", p);
    }

    MathUtil::glFuncs->glDetachShader(p, v);
    MathUtil::glFuncs->glDetachShader(p, f);

    (*p_program) = p;
    return true;
}

void MathUtil::Initialize()
{
    if (img_extensions.isEmpty()) {
        img_extensions.push_back("bmp");
        img_extensions.push_back("gif");
        img_extensions.push_back("jpg");
        img_extensions.push_back("jpeg");
        img_extensions.push_back("pbm");
        img_extensions.push_back("pgm");
        img_extensions.push_back("ppm");
        img_extensions.push_back("png");
        img_extensions.push_back("tif");
        img_extensions.push_back("xbm");
        img_extensions.push_back("xpm");
        img_extensions.push_back("dds");
        img_extensions.push_back("hdr");
    }

    if (geom_extensions.isEmpty()) {
        geom_extensions.push_back("obj");
        geom_extensions.push_back("dae");
        geom_extensions.push_back("ply");
        geom_extensions.push_back("fbx");
        geom_extensions.push_back("gltf");
        geom_extensions.push_back("glb");
        geom_extensions.push_back("obj.gz");
        geom_extensions.push_back("dae.gz");
        geom_extensions.push_back("ply.gz");
        geom_extensions.push_back("fbx.gz");
        geom_extensions.push_back("gltf.gz");
        geom_extensions.push_back("glb.gz");
    }

    if (vid_extensions.isEmpty()) {
        vid_extensions.push_back("mpg");
        vid_extensions.push_back("mp4");        
        vid_extensions.push_back("mkv");
        vid_extensions.push_back("avi");
        vid_extensions.push_back("ogv");
        vid_extensions.push_back("ogg");
        vid_extensions.push_back("mov");
        vid_extensions.push_back("wmv");
        vid_extensions.push_back("m4v");
        vid_extensions.push_back("m3u8");
        vid_extensions.push_back("webm");
        vid_extensions.push_back("mpeg");
    }

    if (sound_extensions.isEmpty()) {
        sound_extensions.push_back("mp3");
        sound_extensions.push_back("wav");
        sound_extensions.push_back("ogg");
        sound_extensions.push_back("oga");
        sound_extensions.push_back("m4a");
        sound_extensions.push_back("wma");
        sound_extensions.push_back("opus");
        sound_extensions.push_back("flac");
        sound_extensions.push_back("aac");
    }

    if (domain_extensions.isEmpty()) {
        domain_extensions.push_back("gov");
        domain_extensions.push_back("edu");
        domain_extensions.push_back("com");
        domain_extensions.push_back("org");
        domain_extensions.push_back("net");
        domain_extensions.push_back("ca");
        domain_extensions.push_back("uk");
        domain_extensions.push_back("us");
    }
}

bool MathUtil::InitializeGLContext()
{
	QOpenGLContext * c = QOpenGLContext::currentContext();
    qDebug() << "MathUtil::InitializeGLContext()" << c;

	if (c == NULL) {
//		QMessageBox::critical(NULL, "Error", "Cannot create platform OpenGL context!");
        qDebug() << "MathUtil::InitializeGLContext() - current context NULL, failed!";
		return false;
    }

    glFuncs = c->extraFunctions();

	if (glFuncs == NULL) {
//		QMessageBox::critical(NULL, "Error", "Could not obtain required OpenGL context (version 3.3).  Ensure your graphics hardware is capable of supporting OpenGL 3.3, and necessary drivers are installed.");
        qDebug() << "MathUtil::InitializeGLContext() - Could not obtain required OpenGL context (version 3.3).  Ensure your graphics hardware is capable of supporting OpenGL 3.3, and necessary drivers are installed.";
		return false;
	}

    glFuncs->initializeOpenGLFunctions();

	// If the debug callback extension is supported we wrangle the function pointer and enable it
	// This is used to catch GL errors as they occur. By setting a breakpoint in the callback
	// function we can also get stacktraces for any GL funcion that triggers an error as soon as it occurs
	if (QOpenGLContext::currentContext()->hasExtension(QByteArrayLiteral("GL_ARB_debug_output")))
	{
		PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB = NULL;
		glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)c->getProcAddress(QByteArrayLiteral("glDebugMessageCallbackARB"));

        if (glDebugMessageCallbackARB != NULL) {
            qDebug() << "MathUtil::InitializeGLContext() - Debug output supported";

            glDebugMessageCallbackARB((GLDEBUGPROCARB)&MathUtil::DebugCallback, NULL);
			glFuncs->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		}
        else {
            qDebug() << "MathUtil::InitializeGLContext() - Debug output not supported";
		}
	}

	return true;
}

float MathUtil::DegToRad(float f)
{
    return f * _PI_OVER_180;
}

float MathUtil::RadToDeg(float f)
{
    return f * _180_OVER_PI;
}

bool MathUtil::GetRayTriIntersect(const QVector3D & rayp, const QVector3D & rayd, const QVector3D & p0, const QVector3D & p1, const QVector3D & p2, QVector3D & ipt)
{

    QVector3D e1 = p1 - p0;
    QVector3D e2 = p2 - p0;

    // Find the cross product of edge2 and the ray direction
    QVector3D s1 = QVector3D::crossProduct(rayd, e2);

    // Find the divisor, if its zero, return false as the triangle is degenerate
    float a = QVector3D::dotProduct(s1, e1);
    if (a > -0.00001 && a < 0.00001) {
        return false;
    }

    // A inverted divisor, as multipling is faster then division
    float f = 1.0f / a;

    // Calculate the first barycentic coordinate. Barycentic coordinates
    // are between 0.0 and 1.0
    QVector3D s = rayp - p0;
    float u = f * QVector3D::dotProduct(s, s1);

    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    // Calculate the second barycentic coordinate
    QVector3D q = QVector3D::crossProduct(s, e1);
    float v = f * QVector3D::dotProduct(rayd, q);
    if (v < 0.0f || (u + v) > 1.0f) {
        return false;
    }
    float t = f * QVector3D::dotProduct(e2, q);
    ipt = rayp + rayd * t;
    return (t > 0.00001);

}

void MathUtil::ComputeBarycentric3D(const QVector3D p, const QVector3D a, const QVector3D b, const QVector3D c, float &u, float &v, float &w)
{
    const QVector3D v0 = b - a, v1 = c - a, v2 = p - a;
    const float d00 = QVector3D::dotProduct(v0, v0);
    const float d01 = QVector3D::dotProduct(v0, v1);
    const float d11 = QVector3D::dotProduct(v1, v1);
    const float d20 = QVector3D::dotProduct(v2, v0);
    const float d21 = QVector3D::dotProduct(v2, v1);
    const float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}

void MathUtil::FacePosDirsGL(const QVector3D & pos, const QVector3D & xdir, const QVector3D & ydir, const QVector3D & zdir)
{
    QMatrix4x4 mat;
    mat.setColumn(0, xdir.toVector4D());
    mat.setColumn(1, ydir.toVector4D());
    mat.setColumn(2, zdir.toVector4D());
    mat.setColumn(3, pos.toVector4D());
    mat.setRow(3, QVector4D(0,0,0,1));

    MathUtil::MultModelMatrix(mat);
}

QVector3D MathUtil::Slerp(QVector3D p1, QVector3D p2, float i)
{
    p1.normalize();
    p2.normalize();

    QVector3D c = QVector3D::crossProduct(p1, p2);
    if (c.length() < 0.001f)
        c = QVector3D(0, 1, 0);
    c.normalize();

    float angle = acosf(QVector3D::dotProduct(p1, p2));

    return GetRotatedAxis(i * angle, p1, c);
}

float MathUtil::CosInterp(const float p1, const float p2, const float i)
{
    float interp = (cosf(i * _PI) + 1.0f) / 2.0f;
    return p1 * interp + p2 * (1.0f - interp);
}

QVector3D MathUtil::CosInterp(QVector3D p1, QVector3D p2, float i)
{
    float interp = (cosf(i * _PI) + 1.0f) / 2.0f;
    return p1 * interp + p2 * (1.0f - interp);
}

float MathUtil::GetAngleBetweenRadians(const QVector3D & v1, const QVector3D & v2)
{

    return acosf(QVector3D::dotProduct(v1.normalized(), v2.normalized()));

}

float MathUtil::GetSignedAngleBetweenRadians(const QVector3D & v1, const QVector3D & v2)
{
    float angle = MathUtil::GetAngleBetweenRadians(v1, v2);
    QVector3D cross_prod = QVector3D::crossProduct(v1, v2);

    if (cross_prod.y() > 0.0f) {
        return angle;
    }
    else {
        return -angle;
    }
}

QVector3D MathUtil::GetRotatedAxis(const float anglerad, const QVector3D & vec, const QVector3D & axis)
{
    if (anglerad == 0.0f) {
        return vec;
    }

    const float sinAngle = sinf(anglerad);
    const float cosAngle = cosf(anglerad);
    const float oneSubCos = 1.0f - cosAngle;

    const float x = axis.x();
    const float y = axis.y();
    const float z = axis.z();

    QVector3D R1, R2, R3;   

    R1.setX( x * x + cosAngle * (1.0f - x * x) );
    R1.setY( x * y * oneSubCos - sinAngle * z );
    R1.setZ( x * z * oneSubCos + sinAngle * y );

    R2.setX(  x * y * oneSubCos + sinAngle * z );
    R2.setY(  y * y + cosAngle * (1.0f - y * y) );
    R2.setZ(  y * z * oneSubCos - sinAngle * x );

    R3.setX(  x * z * oneSubCos - sinAngle * y );
    R3.setY(  y * z * oneSubCos + sinAngle * x );
    R3.setZ(  z * z + cosAngle * (1.0f - z * z) );

    return QVector3D(QVector3D::dotProduct(vec, R1),
                   QVector3D::dotProduct(vec, R2),
                   QVector3D::dotProduct(vec, R3));
}

void MathUtil::SphereToCartesian(const float thetadeg, const float phideg, const float r, QVector3D & p)
{

    const float thetarad = MathUtil::DegToRad(thetadeg);
    const float phirad= MathUtil::DegToRad(phideg);

    p.setX( r * sinf(phirad) * cosf(thetarad) );
    p.setY( r * cosf(phirad) );
    p.setZ( r * sinf(phirad) * sinf(thetarad) );

}

void MathUtil::CartesianToSphere(const QVector3D & p, float & thetadeg, float & phideg, float & r)
{

    thetadeg = MathUtil::RadToDeg(atan2f(p.z(), p.x()));
    phideg = MathUtil::RadToDeg(acosf(p.y() / p.length()));
    r = p.length();

}

void MathUtil::NormSphereToCartesian(const float thetadeg, const float phideg, QVector3D & p)
{

    const float thetarad = MathUtil::DegToRad(thetadeg);
    const float phirad = MathUtil::DegToRad(phideg);

    p.setX( sinf(phirad) * cosf(thetarad) );
    p.setY( cosf(phirad) );
    p.setZ( sinf(phirad) * sinf(thetarad) );

}

void MathUtil::NormCartesianToSphere(const QVector3D & p, float & thetadeg, float & phideg)
{
    thetadeg = MathUtil::RadToDeg(atan2f(p.z(), p.x())) + 90.0f;
    phideg = MathUtil::RadToDeg(acosf(p.y() / p.length()));
}

QVector3D MathUtil::GetNormalColour(const QVector3D & n)
{
    return (n + QVector3D(1,1,1)) * 0.5f;
}

bool MathUtil::LinePlaneIntersection(const QVector3D & p0, const QVector3D & n, const QVector3D & l0, const QVector3D & l1, QVector3D & intersect)
{

    //http://en.wikipedia.org/wiki/Line-plane_intersection
    //if (QVector3D::dotProduct((l1-l0).normalized(), n) < 0.0001f) {
    //    return false;
    //}

    const float p0n = QVector3D::dotProduct(p0, n);
    const float l0n = QVector3D::dotProduct(l0, n);
    const float l1n = QVector3D::dotProduct(l1, n);

    const float interp = (p0n - l0n) / (l1n - l0n);
    //qDebug() << interp;
    intersect = l0 + (l1 - l0) * interp;

    return true;

}

bool isPointInsideTriangle(const Triangle3D & tri, const QVector3D & pt)
{
    QVector3D u = tri.p[1] - tri.p[0];
    QVector3D v = tri.p[2] - tri.p[0];
    QVector3D w = pt - tri.p[0];

    float uu = QVector3D::dotProduct(u, u);
    float uv = QVector3D::dotProduct(u, v);
    float vv = QVector3D::dotProduct(v, v);
    float wu = QVector3D::dotProduct(w, u);
    float wv = QVector3D::dotProduct(w, v);
    float d = uv * uv - uu * vv;

    if (fabsf(d) < 0.00001f) { //triangle is likely degenerate
        return false;
    }

    float invD = 1.0f / d;
    float s = (uv * wv - vv * wu) * invD;
    if (s < 0.0f || s > 1.0f) {
        return false;
    }
    float t = (uv * wu - uu * wv) * invD;
    if (t < 0.0f || (s + t) > 1.0f) {
        return false;
    }

    //qDebug() << "MathUtil::isPointInsideTriangle" << s << t << invD << d;
    return true;
}

bool MathUtil::testIntersectionSphereLine(const Sphere3D &_sphere, const QVector3D &_pt0, const QVector3D &_pt1, int & _nbInter, float & _inter1, float & _inter2)
{
    //nbInter - number of intersections with the sphere
    //inter1 - d value along ray for intersection point
    //inter2 - d value along ray for intersection point
    _nbInter = 0;

    float a, b, c, i;

    a = (_pt1.x() - _pt0.x())*(_pt1.x() - _pt0.x()) + (_pt1.y() - _pt0.y())*(_pt1.y() - _pt0.y()) + (_pt1.z() - _pt0.z())*(_pt1.z() - _pt0.z());
    b =  2.0f * ( (_pt1.x() - _pt0.x()) * (_pt0.x() - _sphere.cent.x()) + (_pt1.y() - _pt0.y()) * (_pt0.y() - _sphere.cent.y()) + (_pt1.z() - _pt0.z()) * (_pt0.z() - _sphere.cent.z()) ) ;
    c = (_sphere.cent.x())*(_sphere.cent.x()) + (_sphere.cent.y())*(_sphere.cent.y()) +
        (_sphere.cent.z())*(_sphere.cent.z()) + (_pt0.x())*(_pt0.x()) +
        (_pt0.y())*(_pt0.y()) + (_pt0.z())*(_pt0.z()) -
        2.0f * ( _sphere.cent.x() * _pt0.x() + _sphere.cent.y() * _pt0.y() + _sphere.cent.z() * _pt0.z() ) - (_sphere.rad)*(_sphere.rad) ;
    i =  b * b - 4 * a * c;

    if (i < 0) {
        return false;
    }

    if (i == 0) {
        _nbInter = 1;
        _inter1 = -b / (2.0f * a);
    }
    else {
        _nbInter = 2;
        _inter1 = (-b + sqrtf( b*b - 4.0f*a*c )) / (2.0f  * a);
        _inter2 = (-b - sqrtf( b*b - 4.0f*a*c )) / (2.0f  * a);
    }

    return true;
}

float MathUtil::distancePointToLine(const QVector3D &_point,
                          const QVector3D &_pt0,
                          const QVector3D &_pt1,
                          QVector3D & _linePt)
{
    QVector3D v = _point - _pt0;
    QVector3D s = _pt1 - _pt0;
    float lenSq = s.lengthSquared();
    float dot = QVector3D::dotProduct(v, s) / lenSq;
    QVector3D disp = s * dot;
    //if (_linePt) {
    _linePt = _pt0 + disp;
    v -= disp;

    return v.length();
}

bool MathUtil::testIntersectionLineLine(const QVector2D &_p1, const QVector2D &_p2, const QVector2D &_p3, const QVector2D &_p4, float & _t)
{
    QVector2D d1 = _p2 - _p1;
    QVector2D d2 = _p3 - _p4;

    float denom = d2.y()*d1.x() - d2.x()*d1.y();
    if (denom == 0.0f) {
        return false;
    }

    float dist = d2.x()*(_p1.y()-_p3.y()) - d2.y()*(_p1.x()-_p3.x());
    dist /= denom;
    _t = dist;

    return true;
}

bool MathUtil::testIntersectionTriSphere(const Triangle3D & tri, const QVector3D &_triNormal, const Sphere3D &_sphere, const QVector3D &_sphereVel, float & _distTravel, QVector3D & _reaction)
{

    _distTravel = FLT_MAX;

    QVector3D nvelo = _sphereVel.normalized();
    const float dot = QVector3D::dotProduct(_triNormal, nvelo);

    if (dot > -0.001f) {
        return false;
    }

    int col = -1;    

    Plane3D plane;
    plane.fromPointAndNormal(tri.p[0], _triNormal);

    // pass1: sphere VS plane
    float h = plane.dist( _sphere.cent );
    if (h < -_sphere.rad) {
        return false;
    }

    if (h > _sphere.rad) {

        h -= _sphere.rad;

//        if (dot != 0) { //bugfix: 32.5 ninja update (seems to stop people falling through faces)
            float t = -h / dot; //t is the distance until a collision occurs with this triangle/plane
            QVector3D onPlane = _sphere.cent + nvelo * t;
            //QVector3D onPlane = _sphere.cent + _sphereVel * t; //release 21.20 candidate correction
            if (isPointInsideTriangle(tri, onPlane)) { //this is the inside triangle test
                if (t < _distTravel) { //distTravel is FLT_MAX - so this is always true???
                    _distTravel = t;
                    _reaction = _triNormal;
                    col = 0;
                }
            }
//        }

    }


    // pass2: sphere VS triangle vertices

    if (col == -1) {
        for (int i = 0; i < 3; i++) {
            QVector3D seg_pt0 = tri.p[i];
            QVector3D seg_pt1 = seg_pt0 - nvelo;
            QVector3D v = seg_pt1 - seg_pt0;

            float inter1=FLT_MAX, inter2=FLT_MAX;
            int nbInter;
            bool res = testIntersectionSphereLine(_sphere, seg_pt0, seg_pt1, nbInter, inter1, inter2);
            if (res == false) {
                continue;
            }

            float t = inter1;
            if (inter2 < t) {
                t = inter2;
            }

            if (t < 0) {
                continue;
            }

            if (t < _distTravel) {
                _distTravel = t;
                QVector3D onSphere = seg_pt0 + v * t;
                _reaction = _sphere.cent - onSphere;
                col = 1;
            }
        }
    }

    // pass3: sphere VS triangle edges
    if (col == -1) {
        for (int i = 0; i < 3; i++) {

            QVector3D edge0 = tri.p[i];
            int j = (i + 1) % 3;
            QVector3D edge1 = tri.p[j];

            Plane3D plane;
            plane.fromPoints(edge0, edge1, edge1 - nvelo);
            float d = plane.dist(_sphere.cent);
            if (d > _sphere.rad || d < -_sphere.rad) {
                continue;
            }

            float srr = _sphere.rad * _sphere.rad;
            float r = sqrtf(srr - d*d);

            QVector3D pt0 = plane.project(_sphere.cent); // center of the sphere slice (a circle)

            QVector3D onLine;
            //float h = distancePointToLine(pt0, edge0, edge1, onLine);
            distancePointToLine(pt0, edge0, edge1, onLine);
            QVector3D v = (onLine - pt0).normalized();
            QVector3D pt1 = v * r + pt0; // point on the sphere that will maybe collide with the edge

            int a0 = 0;
            int a1 = 1;
            float pl_x = fabsf(plane.a);
            float pl_y = fabsf(plane.b);
            float pl_z = fabsf(plane.c);
            if (pl_x > pl_y && pl_x > pl_z) {
                a0 = 1;
                a1 = 2;
            }
            else {
                if (pl_y > pl_z) {
                    a0 = 0;
                    a1 = 2;
                }
            }

            QVector3D vv = pt1 + nvelo;

            float t;
            float pt1a0 = GetVectorComponent(pt1, a0);
            float pt1a1 = GetVectorComponent(pt1, a1);
            float vva0 = GetVectorComponent(vv, a0);
            float vva1 = GetVectorComponent(vv, a1);
            float edge0a0 = GetVectorComponent(edge0, a0);
            float edge0a1 = GetVectorComponent(edge0, a1);
            float edge1a0 = GetVectorComponent(edge1, a0);
            float edge1a1 = GetVectorComponent(edge1, a1);
            bool res = testIntersectionLineLine(QVector2D(pt1a0, pt1a1),
                                                QVector2D(vva0, vva1),
                                                QVector2D(edge0a0, edge0a1),
                                                QVector2D(edge1a0, edge1a1),
                                                t);
            if (!res || t < 0.0f) {
                continue;
            }

            QVector3D inter = pt1 + nvelo * t;

            QVector3D r1 = edge0 - inter;
            QVector3D r2 = edge1 - inter;
            if (QVector3D::dotProduct(r1, r2) > 0.0f) {
                continue;
            }

            if (t <= _distTravel) {
                _distTravel = t;
                _reaction = _sphere.cent - pt1;
                col = 2;
            }
        }
    }

    if (col != -1) {
        _reaction.normalize();
    }

//    if (col >= 0 && _distTravel < 0.5f) {
//        qDebug() << "MathUtil::testIntersectionTriSphere() - Collision type" << col << _distTravel << _reaction;
//    }

    return (col != -1);
}

float MathUtil::GetVectorComponent(const QVector3D & v, const int i)
{
    switch (i) {
    case 0:
        return v.x();
    case 1:
        return v.y();
    case 2:
        return v.z();
    }

    return 0.0f;

}

void MathUtil::FlushErrorLog()
{
    if (!error_log_msgs.isEmpty()) {
        QString filename = GetAppDataPath() + "error_log.txt";
        QFile file(filename);

        if (!file.open(QIODevice::Append | QIODevice::Text)) {
            qDebug() << "MathUtil::ErrorLog(): File " << filename << " can't be appended";
            return;
        }

        QTextStream ofs(&file);
        for (int i=0; i<error_log_msgs.size(); ++i) {
            ofs << error_log_msgs[i] << "\n";
        }
        file.close();
        error_log_msgs.clear();
    }
}

QStringList MathUtil::GetErrorLogTemp()
{
    return error_log_msgs_temp;
}

void MathUtil::ClearErrorLogTemp()
{
    error_log_msgs_temp.clear();
}

void MathUtil::ErrorLog(const QString line)
{
    error_log_msgs_temp.push_back(line);

    const QString s = QString("[") + QDateTime::currentDateTime().toString() + "] " + line;
    error_log_msgs.push_back(s);
//    qDebug() << "MathUtil::ErrorLog" << s;
}

QString MathUtil::GetCurrentDateTimeAsString() {
    return QDateTime::currentDateTime().toString("yyyy.MM.dd-hh.mm.ss");
}

QVector3D MathUtil::huecycle(double val)
{
    val *= 6;

    if (0 <= val && val < 1) {
        return QVector3D(0, val, 1.0);
    }
    else if (1 <= val && val < 2) {
        return QVector3D(0, 1, 1 - (val-1));
    }
    else if (2 <= val && val < 3) {
        return QVector3D(val-2, 1, 0);
    }
    else if (3 <= val && val < 4) {
        return QVector3D(1, 1 - (val-3), 0);
    }
    else if (4 <= val && val < 5) {
        return QVector3D(1, 0, val-4);
    }
    else if (5 <= val && val < 6) {
        return QVector3D(1 - (val-5), 0, 1);
    }
    else {
        return QVector3D(1, 1, 1);
    }
}

QVector3D MathUtil::GetOrthoVec(const QVector3D & v)
{

    if (fabsf(QVector3D::dotProduct(v,QVector3D(1,0,0))) > 0.9f) {
        return QVector3D::crossProduct(v, QVector3D(0,1,0)).normalized();
    }
    else {
        return QVector3D::crossProduct(v, QVector3D(1,0,0)).normalized();
    }

}


QString MathUtil::GetBoolAsString(const bool b)
{
    return b ? QString("\"true\"") : QString("\"false\"");
}

QString MathUtil::GetStringAsString(const QString & s)
{
    return "\"" + s + "\"";
}

QString MathUtil::GetFloatAsString(const float f)
{
    return "\"" + GetNumber(f) + "\"";
}

QString MathUtil::GetIntAsString(const int i)
{
    return "\"" + QString::number(i) + "\"";
}

QString MathUtil::GetVectorAsString(const QVector3D & v, const bool add_quotes)
{
    if (add_quotes) {
        return "\"" + GetNumber(v.x()) + " "
                + GetNumber(v.y()) + " "
                + GetNumber(v.z()) + "\"";
    }
    else {
        return GetNumber(v.x()) + " "
                + GetNumber(v.y()) + " "
                + GetNumber(v.z());
    }
}

QString MathUtil::GetVector4AsString(const QVector4D & v, const bool add_quotes)
{
    if (add_quotes) {
        return "\"" + GetNumber(v.x()) + " "
                + GetNumber(v.y()) + " "
                + GetNumber(v.z()) + " "
                + GetNumber(v.w()) + "\"";
    }
    else {
        return GetNumber(v.x()) + " "
                + GetNumber(v.y()) + " "
                + GetNumber(v.z()) + " "
                + GetNumber(v.w());
    }
}

QString MathUtil::GetColourAsString(const QColor & c, const bool add_quotes)
{
    QString s;
    if (c.alpha() == 255) {
        s = c.name();
    }
    else {
        s = "rgba(" + QString::number(c.red()) + ", "
                + QString::number(c.green()) + ", "
                + QString::number(c.blue()) + ", "
                + QString::number(c.alphaF(), 'g', 2) + ")";
    }

    if (add_quotes) {
        return "\"" + s + "\"";
    }
    else {
        return s;
    }
}
QString MathUtil::GetNumber(const float f)
{
    QString s = QString::number(f, 'f');

    while (!s.isEmpty() && s.contains(".") && QString::compare(s.right(1), "0") == 0) {
        s.truncate(s.length()-1);
    }

    if (!s.isEmpty() && QString::compare(s.right(1), ".") == 0) {
        s.truncate(s.length()-1);
    }

    return s;
}

QString MathUtil::GetAABBAsString(const QPair <QVector3D, QVector3D> & v, const bool add_quotes)
{
    if (add_quotes) {
        return "\""
                + GetNumber(v.first.x()) + " "
                + GetNumber(v.first.y()) + " "
                + GetNumber(v.first.z()) + " "
                + GetNumber(v.second.x()) + " "
                + GetNumber(v.second.y()) + " "
                + GetNumber(v.second.z()) + "\"";
    }
    else {
        return GetNumber(v.first.x()) + " "
                + GetNumber(v.first.y()) + " "
                + GetNumber(v.first.z()) + " "
                + GetNumber(v.second.x()) + " "
                + GetNumber(v.second.y()) + " "
                + GetNumber(v.second.z());
    }
}

QString MathUtil::GetRectangleAsString(const QRectF & r, const bool add_quotes)
{
    if (add_quotes) {
        return "\"" + GetNumber(r.x()) + " " +
                GetNumber(r.y()) + " " +
                GetNumber(r.x() + r.width()) + " " +
                GetNumber(r.y() + r.height()) + "\"";
    }
    else {
        return GetNumber(r.x()) + " " +
                GetNumber(r.y()) + " " +
                GetNumber(r.x() + r.width()) + " " +
                GetNumber(r.y() + r.height());
    }
}

QString MathUtil::GetEnumAsString(const GLenum e, const bool add_quotes)
{
    QString s;

    switch (e) {
    case GL_FRONT:
        s = "front";
        break;
    case GL_BACK:
        s = "back";
        break;
    case GL_FRONT_AND_BACK:
        s = "front_and_back";
        break;
    default:
        s = "none";
        break;
    }

    if (add_quotes) {
        s = "\"" + s + "\"";
    }

    return s;
}

QString MathUtil::GetRectAsString(const QRectF & r, const bool add_quotes)
{
    if (add_quotes) {
        return "\"" + GetNumber(r.x()) + " " +
                GetNumber(r.y()) + " " +
                GetNumber(r.x() + r.width()) + " " +
                GetNumber(r.y() + r.height()) + "\"";
    }
    else {
        return GetNumber(r.x()) + " " +
                GetNumber(r.y()) + " " +
                GetNumber(r.x() + r.width()) + " " +
                GetNumber(r.y() + r.height());
    }
}

QVector3D MathUtil::GetVectorFromQVariant(const QVariant v)
{
    ScriptableVector * v0 = qvariant_cast<ScriptableVector *>(v);
    return (v0 ? v0->toQVector3D() : GetStringAsVector(v.toString()));
}

QVector4D MathUtil::GetVector4FromQVariant(const QVariant v)
{
    ScriptableVector * v0 = qvariant_cast<ScriptableVector *>(v);
    return (v0 ? v0->toQVector4D() : GetStringAsVector4(v.toString()));
}

QVector4D MathUtil::GetColourFromQVariant(const QVariant v)
{
    ScriptableVector * v0 = qvariant_cast<ScriptableVector *>(v);
    return (v0 ? (v0->toQVector4D()) : MathUtil::GetColourAsVector4(GetStringAsColour(v.toString())));
}

QVector3D MathUtil::GetStringAsVector(const QString & s)
{
    QString s2 = s.trimmed();
	s2.remove("\"");
	QStringList sl = s2.split(" ");
	if (sl.size() >= 3) {
		return QVector3D(sl[0].toFloat(), sl[1].toFloat(), sl[2].toFloat());
	}
	else {
		return QVector3D(0, 0, 0);
	}
}

QVector4D MathUtil::GetStringAsVector4(const QString & s)
{
    QString s2 = s.trimmed();
	s2.remove("\"");
	QStringList sl = s2.split(" ");
	if (sl.size() >= 4) {
		return QVector4D(sl[0].toFloat(), sl[1].toFloat(), sl[2].toFloat(), sl[3].toFloat());
	}
	else {
		return QVector4D(0, 0, 0, 0);
	}
}

QColor MathUtil::GetVector4AsColour(const QVector4D v)
{
    return QColor(int(qMin(1.0f, qMax(0.0f, v.x())) * 255.0f),
                  int(qMin(1.0f, qMax(0.0f, v.y())) * 255.0f),
                  int(qMin(1.0f, qMax(0.0f, v.z())) * 255.0f),
                  int(qMin(1.0f, qMax(0.0f, v.w())) * 255.0f));
}

QVector4D MathUtil::GetColourAsVector4(const QColor c)
{
    return QVector4D(qMin(1.0, qMax(0.0, c.redF())), qMin(1.0, qMax(0.0, c.greenF())), qMin(1.0, qMax(0.0, c.blueF())), qMin(1.0, qMax(0.0, c.alphaF())));
}

QColor MathUtil::GetStringAsColour(const QString & s)
{
    QColor c = QColor(255,255,255,255);

    if (s.isEmpty()) {
        return c;
    }

    const QString s2 = s.trimmed();
    QString s3 = s2;
    s3.remove("\"");
    s3.remove("rgba");
    s3.remove("rgb");
    s3.remove("hsla");
    s3.remove("hsl");
    s3.remove("(");
    s3.remove(")");
    s3.remove("%");
    s3.replace(",", " ");
    QStringList sl = s3.split(" ", QString::SkipEmptyParts);

    if (s2.left(5) == "rgba(" && sl.size() >= 4) {
        c.setRed(qMin(255, qMax(0, sl[0].toInt())));
        c.setGreen(qMin(255, qMax(0, sl[1].toInt())));
        c.setBlue(qMin(255, qMax(0, sl[2].toInt())));
        c.setAlphaF(qMin(1.0f, qMax(0.0f, sl[3].toFloat())));
    }
    else if (s2.left(4) == "rgb(" && sl.size() >= 3) {
        c.setRed(qMin(255, qMax(0, sl[0].toInt())));
        c.setGreen(qMin(255, qMax(0, sl[1].toInt())));
        c.setBlue(qMin(255, qMax(0, sl[2].toInt())));
        c.setAlphaF(1.0f);
    }
    else if (s2.left(5) == "hsla(" && sl.size() >= 4) {
        c.setHslF(float(qMin(360, qMax(0, sl[0].toInt()))) / 360.0f,
                float(qMin(100, qMax(0, sl[1].toInt()))) / 100.0f,
                float(qMin(100, qMax(0, sl[2].toInt()))) / 100.0f);
        c.setAlphaF(qMin(1.0f, qMax(0.0f, sl[3].toFloat())));
    }
    else if (s2.left(4) == "hsl(" && sl.size() >= 3) {
        c.setHslF(float(qMin(360, qMax(0, sl[0].toInt()))) / 360.0f,
                float(qMin(100, qMax(0, sl[1].toInt()))) / 100.0f,
                float(qMin(100, qMax(0, sl[2].toInt()))) / 100.0f);
    }
    else {
        c.setNamedColor(s2);
        if (!c.isValid()) {
            if (sl.size() == 4) {
                QVector4D v(sl[0].toFloat(), sl[1].toFloat(), sl[2].toFloat(), sl[3].toFloat());
                c = QColor(int(qMin(1.0f, qMax(0.0f, v.x())) * 255.0f),
                           int(qMin(1.0f, qMax(0.0f, v.y())) * 255.0f),
                           int(qMin(1.0f, qMax(0.0f, v.z())) * 255.0f),
                           int(qMin(1.0f, qMax(0.0f, v.w())) * 255.0f));
            }
            else if (sl.size() == 3) {
                QVector3D v(sl[0].toFloat(), sl[1].toFloat(), sl[2].toFloat());
                c = QColor(int(qMin(1.0f, qMax(0.0f, v.x())) * 255.0f),
                           int(qMin(1.0f, qMax(0.0f, v.y())) * 255.0f),
                           int(qMin(1.0f, qMax(0.0f, v.z())) * 255.0f));                
            }
        }
    }
    return c;
}

QPair <QVector3D, QVector3D> MathUtil::GetStringAsAABB(const QString & s)
{
    QString s2 = s;
    s2 = s2.remove("\"").trimmed();
    QStringList sl = s2.split(" ");
    QPair <QVector3D, QVector3D> p;
    p.first = QVector3D(0,0,0);
    p.second= QVector3D(0,0,0);
    if (sl.size() >= 6) {
        p.first = QVector3D(sl[0].toFloat(), sl[1].toFloat(), sl[2].toFloat());
        p.second = QVector3D(sl[3].toFloat(), sl[4].toFloat(), sl[5].toFloat());
    }
    return p;
}

QVector3D MathUtil::GetStringAsDoubleVector(const QString & s)
{
    QString s2 = s;
    s2.remove("\"");
    QStringList sl = s2.split(" ");
    if (sl.size() >= 3) {
        return QVector3D(remainder(sl[0].toDouble(), MathUtil::_2_PI),
                remainder(sl[1].toDouble(), MathUtil::_2_PI),
                remainder(sl[2].toDouble(), MathUtil::_2_PI));
    }
    else {
        return QVector3D(0,0,0);
    }
}

bool MathUtil::GetStringAsBool(const QString & s) {
    return s.toLower() == "true";
}

QRectF MathUtil::GetStringAsRect(const QString & s)
{
    QString s2 = s;
    s2.remove("\"");
    QStringList sl = s2.split(" ");
    if (sl.size() >= 4) {
        return QRectF(sl[0].toFloat(), sl[1].toFloat(), sl[2].toFloat()-sl[0].toFloat(), sl[3].toFloat()-sl[1].toFloat());
    }
    else {
        return QRectF(0,0,0,0);
    }
}

QString MathUtil::GetTranslatorPath()
{
    return QString("http://downloads.janusvr.com/translator/");
}

QString MathUtil::GetPath_Util(const QString subdir)
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    if (paths.empty()) {
        return QCoreApplication::applicationDirPath();
    }
    else {
        paths[0] = paths[0] + subdir;
        QDir path_dir(paths.first());
        if (!path_dir.exists()) {
            path_dir.mkpath(".");
        }

        return paths.first();
    }
}

QString MathUtil::GetWorkspacePath()
{
    return GetPath_Util("/janusvr/workspaces/");
}

QString MathUtil::GetRecordingPath()
{
    return GetPath_Util("/janusvr/recordings/");
}

QString MathUtil::GetScreenshotPath()
{
    return GetPath_Util("/janusvr/screenshots/");
}

void MathUtil::SetLastScreenshotPath(QString p_last_screenshot_path)
{
    m_last_screenshot_path = p_last_screenshot_path;
}

QString MathUtil::GetLastScreenshotPath()
{
    return m_last_screenshot_path;
}

QString MathUtil::GetCachePath()
{
    return GetPath_Util("/janusvr/cache/");
}

QString MathUtil::GetAppDataPath()
{
    return GetPath_Util("/janusvr/appdata/");
}

QString MathUtil::GetApplicationPath()
{
    //    qDebug() << QCoreApplication::applicationDirPath() + "/";
    return QCoreApplication::applicationDirPath() + "/";
}

QString MathUtil::GetApplicationURL()
{
//    qDebug() << QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/").toString();
    //    qDebug() << QCoreApplication::applicationDirPath() + "/";
    return QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/").toString();
}

QByteArray MathUtil::Decompress(const QByteArray & compressData)
{
    //decompress GZIP data
    const int buffersize = 16384;
    quint8 buffer[buffersize];

    z_stream cmpr_stream;
    cmpr_stream.next_in = (unsigned char *)compressData.data();
    cmpr_stream.avail_in = compressData.size();
    cmpr_stream.total_in = 0;

    cmpr_stream.next_out = buffer;
    cmpr_stream.avail_out = buffersize;
    cmpr_stream.total_out = 0;

    cmpr_stream.zalloc = Z_NULL;
    cmpr_stream.zfree = Z_NULL;
    cmpr_stream.opaque = Z_NULL;

    if( inflateInit2(&cmpr_stream, 15 + 32) != Z_OK) {
        qDebug() << "MathUtil::Decompress() - cmpr_stream error!";
    }

    QByteArray uncompressed;
    do {
        int status = inflate( &cmpr_stream, Z_SYNC_FLUSH );

        if(status == Z_OK || status == Z_STREAM_END) {
            uncompressed.append(QByteArray::fromRawData((char *)buffer, buffersize - cmpr_stream.avail_out));
            cmpr_stream.next_out = buffer;
            cmpr_stream.avail_out = buffersize;
        } else {
            inflateEnd(&cmpr_stream);
            break;
        }

        if(status == Z_STREAM_END) {
            inflateEnd(&cmpr_stream);
            break;
        }

    } while (cmpr_stream.avail_out != 0);

    return uncompressed;
}

QString MathUtil::MD5Hash(const QString & s)
{
    return QString(QCryptographicHash::hash(s.toLatin1(), QCryptographicHash::Md5).toHex());
}

QString MathUtil::EncodeString(const QString & s)
{
    QString encoded = s;
    encoded.replace("\"", "^");
    //encoded.replace(" ", "~");
    encoded.replace("\n", ""); //note: used to be |
    encoded.replace("\r", ""); //note: used to be |
    encoded.replace("\t", " ");
    return encoded;
}

QString MathUtil::DecodeString(const QString & s)
{
    QString decoded = s;
    decoded.replace("^", "\"");
    //decoded.replace("~", " ");
    //decoded.replace("|", "\n");
    return decoded;
}

void MathUtil::DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei , const GLchar *message, GLvoid *)
{    
    if (type != GL_DEBUG_TYPE_OTHER_ARB)
    {

#ifdef _WIN64
#pragma warning( push )
#pragma warning( disable : 4996 )
#endif

        qDebug() << "MathUtil::DebugCallback" << source << type << id << severity << message;

#ifdef _WIN64
#pragma warning( pop )
#endif
    }
}

/* default error routine.  change this to change error handling */
bool MathUtil::rgbe_error(const int rgbe_error_code, const QString & msg)
{
  switch (rgbe_error_code) {
  case rgbe_read_error:
    qDebug() << "RGBE read error";
    break;
  case rgbe_write_error:
    qDebug() << "RGBE write error";
    break;
  case rgbe_format_error:
    qDebug() << "RGBE bad file format:" << msg;
    break;
  default:
  case rgbe_memory_error:
    qDebug() << "RGBE error: " << msg;
  }
  return false;
}

/* minimal header reading.  modify if you want to parse more information */
bool MathUtil::RGBE_ReadHeader(QBuffer & buffer, int *width, int *height, rgbe_header_info *info)
{
    const int buffer_size = 128;
    char buf[buffer_size];
    float tempf;
    unsigned int i;

    if (info) {
        info->valid = 0;
        info->programtype[0] = 0;
        info->gamma = info->exposure = 1.0;
    }
    if (buffer.readLine(buf,buffer_size) <= 0) {
        return rgbe_error(rgbe_read_error,"");
    }
    if ((buf[0] != '#')||(buf[1] != '?')) {
        return rgbe_error(rgbe_format_error,"bad initial token");
    }
    else if (info) {
        info->valid |= RGBE_VALID_PROGRAMTYPE;
        for (i=0;i<sizeof(info->programtype)-1;i++) {
            if ((buf[i+2] == 0) || isspace(buf[i+2])) {
                break;
            }
            info->programtype[i] = buf[i+2];
        }
        info->programtype[i] = 0;
        if (buffer.readLine(buf,buffer_size) <= 0) {
            return rgbe_error(rgbe_read_error,"");
        }
    }

    for(;;) {
        if ((buf[0] == 0)||(buf[0] == '\n')) {
            return rgbe_error(rgbe_format_error,"no FORMAT specifier found");
        }
        else if (strcmp(buf,"FORMAT=32-bit_rle_rgbe\n") == 0) {
            break;       /* format found so break out of loop */
        }
        else if (info && (sscanf(buf,"GAMMA=%g",&tempf) == 1)) {
            info->gamma = tempf;
            info->valid |= RGBE_VALID_GAMMA;
        }
        else if (info && (sscanf(buf,"EXPOSURE=%g",&tempf) == 1)) {
            info->exposure = tempf;
            info->valid |= RGBE_VALID_EXPOSURE;
        }

        if (buffer.readLine(buf,buffer_size) <= 0) {
            return rgbe_error(rgbe_read_error,"");
        }
    }

    if (buffer.readLine(buf,buffer_size) <= 0) {
        return rgbe_error(rgbe_read_error,NULL);
    }
    if (strcmp(buf,"\n") != 0) {
        return rgbe_error(rgbe_format_error, "missing blank line after FORMAT specifier");
    }
    if (buffer.readLine(buf,buffer_size) <= 0) {
        return rgbe_error(rgbe_read_error,NULL);
    }
    if (sscanf(buf,"-Y %d +X %d",height,width) < 2) {
        return rgbe_error(rgbe_format_error,"missing image size specifier");
    }
    return true;
}

/* simple read routine.  will not correctly handle run length encoding */
bool MathUtil::RGBE_ReadPixels(QBuffer & buffer, float *data, int numpixels)
{
  unsigned char rgbe[4];

  while(numpixels-- > 0) {
      if (buffer.read((char *)rgbe, 4) < 1) {
          return rgbe_error(rgbe_read_error,NULL);
      }
      rgbe2float(&data[RGBE_DATA_RED],&data[RGBE_DATA_GREEN],
                 &data[RGBE_DATA_BLUE],rgbe);
      data += RGBE_DATA_SIZE;
  }

  return true;
}

bool MathUtil::RGBE_ReadPixels_RLE(QBuffer & buffer, float *data, int scanline_width, int num_scanlines)
{
    unsigned char rgbe[4], *scanline_buffer, *ptr, *ptr_end;
    int i, count;
    unsigned char buf[2];

    if ((scanline_width < 8)||(scanline_width > 0x7fff)) {
        /* run length encoding is not allowed so read flat*/
        return RGBE_ReadPixels(buffer,data,scanline_width*num_scanlines);
    }
    scanline_buffer = NULL;
    /* read in each successive scanline */
    while(num_scanlines > 0) {
        if (buffer.read((char *)rgbe, 4) < 1) {
            free(scanline_buffer);
            return rgbe_error(rgbe_read_error,NULL);
        }
        if ((rgbe[0] != 2)||(rgbe[1] != 2)||(rgbe[2] & 0x80)) {
            /* this file is not run length encoded */
            rgbe2float(&data[0],&data[1],&data[2],rgbe);
            data += RGBE_DATA_SIZE;
            free(scanline_buffer);
            return RGBE_ReadPixels(buffer,data,scanline_width*num_scanlines-1);
        }
        if ((((int)rgbe[2])<<8 | rgbe[3]) != scanline_width) {
            free(scanline_buffer);
            return rgbe_error(rgbe_format_error,"wrong scanline width");
        }
        if (scanline_buffer == NULL)
            scanline_buffer = (unsigned char *)
                    malloc(sizeof(unsigned char)*4*scanline_width);
        if (scanline_buffer == NULL)
            return rgbe_error(rgbe_memory_error,"unable to allocate buffer space");

        ptr = &scanline_buffer[0];
        /* read each of the four channels for the scanline into the buffer */
        for(i=0;i<4;i++) {
            ptr_end = &scanline_buffer[(i+1)*scanline_width];
            while(ptr < ptr_end) {
                if (buffer.read((char *)buf, 2) < 1) {
                    free(scanline_buffer);
                    return rgbe_error(rgbe_read_error,NULL);
                }
                if (buf[0] > 128) {
                    /* a run of the same value */
                    count = buf[0]-128;
                    if ((count == 0)||(count > ptr_end - ptr)) {
                        free(scanline_buffer);
                        return rgbe_error(rgbe_format_error,"bad scanline data");
                    }
                    while(count-- > 0)
                        *ptr++ = buf[1];
                }
                else {
                    /* a non-run */
                    count = buf[0];
                    if ((count == 0)||(count > ptr_end - ptr)) {
                        free(scanline_buffer);
                        return rgbe_error(rgbe_format_error,"bad scanline data");
                    }
                    *ptr++ = buf[1];
                    if (--count > 0) {
                        if (buffer.read((char *)ptr, count) < 1) {
                            free(scanline_buffer);
                            return rgbe_error(rgbe_read_error,NULL);
                        }
                        ptr += count;
                    }
                }
            }
        }
        /* now convert data from buffer into floats */
        for(i=0;i<scanline_width;i++) {
            rgbe[0] = scanline_buffer[i];
            rgbe[1] = scanline_buffer[i+scanline_width];
            rgbe[2] = scanline_buffer[i+2*scanline_width];
            rgbe[3] = scanline_buffer[i+3*scanline_width];
            rgbe2float(&data[RGBE_DATA_RED],&data[RGBE_DATA_GREEN],&data[RGBE_DATA_BLUE],rgbe);
            data += RGBE_DATA_SIZE;
        }
        num_scanlines--;
    }
    free(scanline_buffer);
    return true;
}

void MathUtil::PushModelMatrix()
{
    if (modelmatrix_stack.isEmpty()) {
        modelmatrix_stack.push_back(QMatrix4x4());
    }
    else {
        modelmatrix_stack.push_back(modelmatrix_stack.last());
    }
}

void MathUtil::PopModelMatrix()
{
    if (!modelmatrix_stack.isEmpty()) {
        modelmatrix_stack.pop_back();
    }
}

void MathUtil::LoadModelIdentity()
{
    if (modelmatrix_stack.isEmpty()) {
        modelmatrix_stack.push_back(QMatrix4x4());
    }
    else {
        modelmatrix_stack.last() = QMatrix4x4();
    }
}

void MathUtil::LoadProjectionIdentity()
{
    projectionmatrix = QMatrix4x4();
}

void MathUtil::LoadViewIdentity()
{
    viewmatrix = QMatrix4x4();
}

void MathUtil::LoadModelMatrix(const QMatrix4x4 m)
{
    if (modelmatrix_stack.isEmpty()) {
        modelmatrix_stack.push_back(m);
    }
    else {
        modelmatrix_stack.last() = m;
    }
}

void MathUtil::LoadProjectionMatrix(const QMatrix4x4 m)
{
    projectionmatrix = m;
}

void MathUtil::LoadViewMatrix(const QMatrix4x4 m)
{
    viewmatrix = m;
}

void MathUtil::MultModelMatrix(const QMatrix4x4 m)
{
    if (modelmatrix_stack.isEmpty()) {
        modelmatrix_stack.push_back(m);
    }
    else {
        modelmatrix_stack.last() =  modelmatrix_stack.last() * m;
    }
}

QMatrix4x4 & MathUtil::ModelMatrix()
{
    if (modelmatrix_stack.isEmpty()) {
        modelmatrix_stack.push_back(QMatrix4x4());
    }

    return modelmatrix_stack.last();
}

void MathUtil::LoadRoomMatrix(const QMatrix4x4 m)
{
    m_roomMatrix = m;
}

QMatrix4x4 & MathUtil::RoomMatrix()
{
    return m_roomMatrix;
}

QMatrix4x4 & MathUtil::ProjectionMatrix()
{    
    return projectionmatrix;
}

QMatrix4x4 & MathUtil::ViewMatrix()
{
    return viewmatrix;
}

QMatrix4x4 MathUtil::getCurrentModelViewProjectionMatrix()
{    
    if (modelmatrix_stack.isEmpty()) {
        modelmatrix_stack.push_back(QMatrix4x4());
    }

    return projectionmatrix * modelmatrix_stack.last() * viewmatrix;
}

/** this conversion uses conventions as described on page:
*   http://www.euclideanspace.com/maths/geometry/rotations/euler/index.htm
*   Coordinate System: right hand
*   Positive angle: right hand
*   Order of euler angles: heading first, then attitude, then bank
*   matrix row column ordering:
*   [m00 m01 m02]
*   [m10 m11 m12]
*   [m20 m21 m22]*/
void MathUtil::MatrixToEulerAngles(const QMatrix4x4 & m, float & heading, float & attitude, float & bank)
{
    const float m00 = m.row(0).x();
    const float m02 = m.row(0).z();
    const float m10 = m.row(1).x();
    const float m11 = m.row(1).y();
    const float m12 = m.row(1).z();
    const float m20 = m.row(2).x();
    const float m22 = m.row(2).z();

    // Assuming the angles are in radians.
    if (m10 > 0.998) { // singularity at north pole
        heading = atan2f(m02,m22);
        attitude = static_cast<float>(M_PI/2);
        bank = 0;
        return;
    }
    if (m10 < -0.998) { // singularity at south pole
        heading = atan2f(m02,m22);
        attitude = static_cast<float>(-M_PI/2);
        bank = 0;
        return;
    }
    heading = atan2f(-m20,m00);
    bank = atan2f(-m12,m11);
    attitude = asinf(m10);
}

void MathUtil::EulerAnglesToMatrix(const float heading, const float attitude, const float bank, QMatrix4x4 & m)
{
    // Assuming the angles are in radians.
    const float ch = cosf(heading);
    const float sh = sinf(heading);
    const float ca = cosf(attitude);
    const float sa = sinf(attitude);
    const float cb = cosf(bank);
    const float sb = sinf(bank);

    m.setToIdentity();

    m.setRow(0, QVector4D(ch * ca, sh*sb - ch*sa*cb, ch*sa*sb + sh*cb, 0));
    m.setRow(1, QVector4D(sa, ca*cb, -ca*sb, 0));
    m.setRow(2, QVector4D(-sh*ca, sh*sa*cb + ch*sb, -sh*sa*sb + ch*cb, 0));
    m.setRow(3, QVector4D(0,0,0,1));
}

void MathUtil::UnProject(float x, float y, float z, const GLdouble modelMatrix[16], const GLdouble projMatrix[16], const GLint viewport[4], GLdouble * new_x, GLdouble * new_y, GLdouble * new_z)
{
    gluUnProject(x, y, z, modelMatrix, projMatrix, viewport, new_x, new_y, new_z);
}


// Shape1 and Shape2 must be CONVEX HULLS
bool MathUtil::GetConvexIntersection(const QVector <QVector3D> & s1_pts, const QVector <QVector3D> & s1_normals, const QVector <QVector3D> & s2_pts, const QVector <QVector3D> & s2_normals)
{
    //Algorithm uses separating axis theorem

    //Project verts onto axes of s1_normals to find overlap
    for(int i = 0; i<s1_normals.size(); i++)
    {
        float shape1Min, shape1Max, shape2Min, shape2Max ;
        GetConvexIntersection_SATtest(s1_normals[i], s1_pts, shape1Min, shape1Max);
        GetConvexIntersection_SATtest(s1_normals[i], s2_pts, shape2Min, shape2Max);
        //find non-overlapping projection axis
        if (!GetConvexIntersection_Overlaps(shape1Min, shape1Max, shape2Min, shape2Max)) {
            return false;
        }
    }

    //Project verts onto axes of s1_normals to find overlap
    for(int i = 0; i<s2_normals.size(); i++)
    {
        float shape1Min, shape1Max, shape2Min, shape2Max ;
        GetConvexIntersection_SATtest(s2_normals[i], s1_pts, shape1Min, shape1Max);
        GetConvexIntersection_SATtest(s2_normals[i], s2_pts, shape2Min, shape2Max);
        //find non-overlapping projection axis
        if (!GetConvexIntersection_Overlaps(shape1Min, shape1Max, shape2Min, shape2Max)) {
            return false;
        }
    }

    //We were unable to find an axis (and this a set of separating planes) along which they did not overlap, thus they intersect
    return true;
}

void MathUtil::GetConvexIntersection_SATtest(const QVector3D & axis, const QVector <QVector3D> & ptSet, float & minAlong, float & maxAlong)
{
    minAlong=FLT_MAX, maxAlong=-FLT_MAX;
    for( int i = 0 ; i < ptSet.size() ; i++ )
    {
        // just dot it to get the min/max along this axis.
        const float dotVal = QVector3D::dotProduct(ptSet[i], axis);
        if (dotVal < minAlong) {
            minAlong=dotVal;
        }
        if (dotVal > maxAlong) {
            maxAlong=dotVal;
        }
    }
}

bool MathUtil::GetConvexIntersection_Overlaps(const float min1, const float max1, const float min2, const float max2)
{
  return (min1 <= min2 && min2 <= max1) || (min2 <= min1 && min1 <= max2);
}

QMatrix4x4 MathUtil::InterpolateMatrices(const QMatrix4x4 & m0, const QMatrix4x4 & m1, const float t)
{    
    QMatrix3x3 m0_3x3 = m0.toGenericMatrix<3,3>();
    QMatrix3x3 m1_3x3 = m1.toGenericMatrix<3,3>();

    QQuaternion q0;
    QQuaternion q1;

    q0 = QQuaternion::fromRotationMatrix(m0_3x3);
    q1 = QQuaternion::fromRotationMatrix(m1_3x3);

    QQuaternion q2 = QQuaternion::nlerp(q0, q1, t); //nlerp faster than slerp

    QMatrix3x3 m_rot = q2.normalized().toRotationMatrix();

    QMatrix4x4 m_final(m_rot);

    const QVector3D p0 = m0.column(3).toVector3D();
    const QVector3D p1 = m1.column(3).toVector3D();
    m_final.setColumn(3, QVector4D(p0*(1.0f-t) + p1*t, 1));
    m_final.setRow(3, QVector4D(0,0,0,1));

    return m_final;
}

QOpenGLTexture* MathUtil::CreateTextureGL(QImage & img, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp)
{
    QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	//texture->create();
    texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
    texture->setSize(img.width(), img.height());

    //mipmap levels
    texture->setAutoMipMapGenerationEnabled(tex_mipmap);
    if (tex_mipmap) {
        int max_mip_level = log(qMin(img.width(), img.height()))/log(2);
        texture->setMipLevelRange(0, max_mip_level);
        texture->setMipLevels(max_mip_level);
    }
    else {
        texture->setMipLevelRange(0, 0);
    }

    //minification filter
    if (tex_mipmap) {
        if (tex_linear) {
            texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        }
        else {
            texture->setMinificationFilter(QOpenGLTexture::NearestMipMapLinear);
        }
    }
    else {
        if (tex_linear) {
            texture->setMinificationFilter(QOpenGLTexture::Linear);
        }
        else {
            texture->setMinificationFilter(QOpenGLTexture::Nearest);
        }
    }

    //magnification filterimg
    if (tex_linear) {
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
    }
    else {
        texture->setMagnificationFilter(QOpenGLTexture::Nearest);
    }

    //clamping
    if (tex_clamp) {
        texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    }
    else {
        texture->setWrapMode(QOpenGLTexture::Repeat);
    }

    //aniso
    texture->setMaximumAnisotropy(16);

    //set up data
    texture->allocateStorage();
    texture->setData(0, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, img.constBits());

    return texture;
}

QOpenGLTexture * MathUtil::CreateTextureGL(const QSize s, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp)
{
    QOpenGLTexture *texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
//    texture->create();
    texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
    texture->setSize(s.width(), s.height());

    //mipmap levels
    texture->setAutoMipMapGenerationEnabled(tex_mipmap);
    if (tex_mipmap) {
        int max_mip_level = log(qMin(s.width(), s.height()))/log(2);
        texture->setMipLevelRange(0, max_mip_level);
        texture->setMipLevels(max_mip_level);
    }
    else {
        texture->setMipLevelRange(0, 0);
    }

    //minification filter
    if (tex_mipmap) {
        if (tex_linear) {
            texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
        }
        else {
            texture->setMinificationFilter(QOpenGLTexture::NearestMipMapLinear);
        }
    }
    else {
        if (tex_linear) {
            texture->setMinificationFilter(QOpenGLTexture::Linear);
        }
        else {
            texture->setMinificationFilter(QOpenGLTexture::Nearest);
        }
    }

    //magnification filter
    if (tex_linear) {
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
    }
    else {
        texture->setMagnificationFilter(QOpenGLTexture::Nearest);
    }

    //clamping
    if (tex_clamp) {
        texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    }
    else {
        texture->setWrapMode(QOpenGLTexture::Repeat);
    }

    //aniso
    texture->setMaximumAnisotropy(16);

    //set up data
    texture->allocateStorage();

    void * mem = calloc(s.width() * s.height(), 4);
    texture->setData(0, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, mem);
    if (tex_mipmap) {
        texture->generateMipMaps();
    }
    free(mem);

    return texture;
}

void MathUtil::CropPixelData(QByteArray source, QByteArray destination, QSize sourceSize, uchar pixelSize, QRect sourceRect)
{
	int width = sourceSize.width();
	int height = sourceSize.height();

	int x = sourceRect.x();
	int y = sourceRect.y();
	int nwidth = sourceRect.width();
	int nheight = sourceRect.height();

	if (width == nwidth &&
		height == nheight)
	{
		// straight up copy the data
        const int sizeBytes = width * height * pixelSize;
        if (destination.size() < sizeBytes) {
            destination.resize(sizeBytes);
        }
        memcpy(destination.data(), source, sizeBytes);
		return;
	}

	switch (pixelSize)
	{
		case 3:
		{
            uchar* s = (uchar*)(source.data());
            uchar* d = (uchar*)(destination.data());

			int startIndex = (x + (y * width)) * 3;
			for (int y = 0; y < nheight; y++)
			{
				memcpy(&d[y * nwidth * 3], &s[startIndex + (y * width * 3)], nwidth * 3);
			}
			break;
		}
		case 4:
		{
            uchar* s = (uchar*)(source.data());
            uchar* d = (uchar*)(destination.data());

			int startIndex = (x + (y * width)) * 4;
			for (int y = 0; y < nheight; y++)
			{
				memcpy(&d[y * nwidth * 4], &s[startIndex + (y * width * 4)], nwidth * 4);
			}

			break;
		}
		case 12:
		{
            float* s = (float*)(source.data());
            float* d = (float*)(destination.data());

			int startIndex = (x + (y * width)) * 3;
			// copy by line
			int size = sizeof(float);

			for (int y = 0; y < nheight; y++)
			{
				memcpy(&d[y * nwidth * 3], &s[startIndex + (y * width * 3)], nwidth * 3 * size);
			}
			break;
		}
	}
}

QByteArray MathUtil::ScalePixelData(QByteArray data, QSize size, uchar pixelSize, QSize nsize)
{
    if (data.isEmpty())
	{
        return QByteArray();
	}

	int width = size.width();
	int height = size.height();

	int nwidth = nsize.width();
	int nheight = nsize.height();

	float xfactor = float(width) / float(nwidth);
	float yfactor = float(height) / float(nheight);

	switch (pixelSize)
	{
        case 3:
        {
            uchar* source = (uchar *)(data.data());
            QByteArray destination;
            destination.resize(nwidth * nheight * 3);

            for (int x = 0; x < nwidth; x++)
            {
                for (int y = 0; y < nheight; y++)
                {
                    int destIndex = (x + (y * nwidth)) * 3;
                    int index = (int(x * xfactor) + (int(y * yfactor) * width)) * 3;

                    uchar sourceR = source[index];
                    uchar sourceG = source[index + 1];
                    uchar sourceB = source[index + 2];

                    destination[destIndex] = sourceR;
                    destination[destIndex + 1] = sourceG;
                    destination[destIndex + 2] = sourceB;
                }
            }
            return destination;
        }
		case 4:
		{
            uchar* source = (uchar*)(data.data());
            QByteArray destination;
            destination.resize(nwidth * nheight * 4);

			for (int y = 0; y < nheight; y++)
			{
				for (int x = 0; x < nwidth; x++)
				{
					int destIndex = (x + (y * nwidth)) * 4;
					int index = (int(x * xfactor) + (int(y * yfactor) * width)) * 4;

					uchar sourceR = source[index];
					uchar sourceG = source[index + 1];
					uchar sourceB = source[index + 2];
					uchar sourceA = source[index + 3];

					destination[destIndex] = sourceR;
					destination[destIndex + 1] = sourceG;
					destination[destIndex + 2] = sourceB;
					destination[destIndex + 3] = sourceA;
				}
			}

			return destination;
        }
		case 12:
		{
            float* source = (float*)(data.data());
            QByteArray destination;
            destination.resize(nwidth * nheight * 3);

			for (int x = 0; x < nwidth; x++)
			{
				for (int y = 0; y < nheight; y++)
				{
					int destIndex = (x + (y * nwidth)) * 3;
					int index = (int(x * xfactor) + (int(y * yfactor) * width)) * 3;

					float sourceR = source[index];
					float sourceG = source[index + 1];
					float sourceB = source[index + 2];

					destination[destIndex] = sourceR;
					destination[destIndex + 1] = sourceG;
					destination[destIndex + 2] = sourceB;
				}
			}
			return destination;
		}
	}

    return QByteArray();
}

QByteArray MathUtil::ScaleToWidth(QByteArray data, QSize size, uchar pixelSize, int nwidth, QSize* outSize)
{
	int width = size.width();
	int height = size.height();

	int nheight = (int)((float(height) / float(width)) * float(nwidth));
	outSize->setWidth(nwidth);
	outSize->setHeight(nheight);

	return ScalePixelData(data, size, pixelSize, QSize(nwidth, nheight));
}

QByteArray MathUtil::ScaleToHeight(QByteArray data, QSize size, uchar pixelSize, int nheight, QSize* outSize)
{
	int width = size.width();
	int height = size.height();

	int nwidth = (int)((float(width) / float(height)) * float(nheight));
	outSize->setWidth(nwidth);
	outSize->setHeight(nheight);

	return ScalePixelData(data, size, pixelSize, QSize(nwidth, nheight));
}

QOpenGLTexture::TextureFormat MathUtil::GetTextureFormat(uchar pixelSize)
{
	switch (pixelSize)
	{
	case 3:
		return QOpenGLTexture::RGB8_UNorm;
	case 4:
		return QOpenGLTexture::RGBA8_UNorm;
	case 12:
		return QOpenGLTexture::RGB16F;
	}
	return QOpenGLTexture::NoFormat; // ????
}

void MathUtil::GetGLFormat(uchar pixelSize, GLenum* format, GLenum* type)
{
	switch (pixelSize)
	{
	case 3:
		*format = GL_RGB;
		*type = GL_UNSIGNED_BYTE;
		break;
	case 4:
		*format = GL_RGBA;
		*type = GL_UNSIGNED_BYTE;
		break;
	case 12:
		*format = GL_RGB;
		*type = GL_FLOAT;
		break;
	}
}

QString MathUtil::StripOutFilename(const QString s)
{
	QString s2 = s;
	if (s2.contains("\\")) {
		s2 = s2.split("\\", QString::SkipEmptyParts).last();
	}
	if (s2.contains("/")) {
		s2 = s2.split("/", QString::SkipEmptyParts).last();
	}
	return s2;
}

float MathUtil::GetSoundLevel(const QByteArray & b)
{
    unsigned long int accumulated = 0;
    for (int i=0; i<b.size(); i+=sizeof(signed short)) {
        accumulated += abs((signed short)(b.data()[i]));
//        qDebug() << i << abs((signed short)(b.data()[i]));
    }
    float f = float(accumulated) / float(b.size()) / 40.0f;
    return qMin(f, 1.0f);
}

QMatrix4x4 MathUtil::GetRotationMatrixFromEuler(const QVector3D rotation, const QString rotation_order)
{
//    qDebug() << "MathUtil::GetRotationMatrixFromEuler" << rotation << rotation_order;
    QMatrix4x4 rot_mat;
    if (rotation_order.length() != 3) {
        return rot_mat;
    }

    for (int i=0; i<3; ++i) {
        float angle = 0.0f;
        if (i == 0) {
            angle = rotation.x();
        }
        else if (i == 1) {
            angle = rotation.y();
        }
        else if (i == 2) {
            angle = rotation.z();
        }

        const char axis = rotation_order[i].toLower().toLatin1();
        if (axis == 'x') {
            rot_mat.rotate(angle, 1, 0 ,0);
        }
        else if (axis == 'y') {
            rot_mat.rotate(angle, 0, 1, 0);
        }
        else if (axis == 'z') {
            rot_mat.rotate(angle, 0, 0, 1);
        }
    }
    return rot_mat;
}

QVariantList & MathUtil::GetPartyModeData()
{
    return partymode_data;
}

quint64 MathUtil::hash(const QString & str)
{
    QByteArray string_byte_array((char const *)str.data(), str.length() * 2);
    QByteArray hashed_byte_array = QCryptographicHash::hash(string_byte_array, QCryptographicHash::Md5);
    QDataStream hashed_byte_stream(hashed_byte_array);
    quint64 a, b;
    hashed_byte_stream >> a >> b;
    return a ^ b;
}

QString MathUtil::GetSaveTimestampFilename()
{
    return MathUtil::GetWorkspacePath() + "out-" + MathUtil::GetCurrentDateTimeAsString() + ".html";
}

ElementType MathUtil::AssetTypeFromFilename(const QString filename)
{
    //make lowercase, strip the .gz if present
    QString s = filename.toLower().trimmed();
    s.replace(".gz", "");

    QFileInfo f(s);
    QString suffix = f.suffix();

//    qDebug() << "AssetTypeFromFilename testing suffix" << filename << suffix;
    if (MathUtil::img_extensions.contains(suffix)) {
        return TYPE_ASSETIMAGE;
    }
    else if (MathUtil::sound_extensions.contains(suffix)) {
        return TYPE_ASSETSOUND;
    }
    else if (MathUtil::vid_extensions.contains(suffix)) {
        return TYPE_ASSETVIDEO;
    }
    else if (MathUtil::geom_extensions.contains(suffix)) {
        return TYPE_ASSETOBJECT;
    }
    else if (suffix == "js") {
        return TYPE_ASSETSCRIPT;
    }
    else if (suffix == "txt") {
        return TYPE_ASSETGHOST;
    }
    else if (suffix == "frag" || suffix == "vert") {
        return TYPE_ASSETSHADER;
    }
    else if (suffix == "html" || suffix == "htm" || suffix == "pdf") {
        return TYPE_ASSETWEBSURFACE;
    }
    else if (suffix == "rec") {
        return TYPE_ASSETRECORDING;
    }    
    else { //if format is unrecognized or all else, assume an image
        return TYPE_ERROR;
    }
}

QByteArray MathUtil::LoadAssetFile(const QString path)
{
    QString p(GetApplicationPath() + path);
    QFile f(p);
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray b = f.readAll();
    f.close();
    return b;
}

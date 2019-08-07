#include "renderer.h"

Renderer * Renderer::m_pimpl = nullptr;

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

void VirtualCamera::RecomputeProjectionMatrix()
{
    if (m_fov > 0.0f && m_fov < 180.0f)
    {
        if (Renderer::m_pimpl->GetIsUsingEnhancedDepthPrecision() == true
            && Renderer::m_pimpl->GetIsEnhancedDepthPrecisionSupported() == true)
        {
            double fovY_radians = m_fov * 0.0174533;
            double f = 1.0 / std::tan(fovY_radians / 2.0);

            // Infinite FarClip
            /*QVector4D col0 = QVector4D(f / m_aspectRatio,   0.0f,   0.0f,       0.0f);
            QVector4D col1 = QVector4D(0.0f,                f,      0.0f,       0.0f);
            QVector4D col2 = QVector4D(0.0f,                0.0f,   0.0f,       -1.0f);
            QVector4D col3 = QVector4D(0.0f,                0.0f,   m_nearClip, 0.0f);*/
            // Normal FarClip
            QVector4D col0 = QVector4D(f / m_aspectRatio,   0.0f,   0.0f,       0.0f);
            QVector4D col1 = QVector4D(0.0f,                f,      0.0f,       0.0f);
            QVector4D col2 = QVector4D(0.0f,                0.0f,   (-1.0 * ((m_farClip) / (m_nearClip - m_farClip))) - 1,          -1.0f);
            QVector4D col3 = QVector4D(0.0f,                0.0f,   -1.0 * ((m_nearClip * m_farClip) / (m_nearClip - m_farClip)), 0.0f);

            QMatrix4x4 inv_depth_inf_far;
            inv_depth_inf_far.setColumn(0, col0);
            inv_depth_inf_far.setColumn(1, col1);
            inv_depth_inf_far.setColumn(2, col2);
            inv_depth_inf_far.setColumn(3, col3);
            m_projectionMatrix = inv_depth_inf_far;
        }
        else
        {
            m_projectionMatrix.perspective(m_fov, m_aspectRatio, m_nearClip, m_farClip);
        }
    }
    else if (m_fov == -1.0f)
    {
        m_projectionMatrix.setToIdentity();
    }
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

Renderer::Renderer()
    : m_current_scope(RENDERER::RENDER_SCOPE::NONE),
      m_abstractRenderer(nullptr)
{
}

Renderer::~Renderer()
{
    m_abstractRenderer.reset();
}

inline void Renderer::InitializeScopes()
{
    // Reserve rough estimates of draw calls needed for each scope to avoid
    // frametime spikes when a new scope is first used
    // There are two vectors for each scope.
    m_abstractRenderer->m_scoped_render_commands_cache.resize(3);
    const int cache_size = m_abstractRenderer->m_scoped_render_commands_cache.size();
    const int cmd_vec_size = 512;

    for (int cache_index = 0; cache_index < cache_size; ++cache_index)
    {
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CURRENT_ROOM_PORTAL_STENCILS].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CHILD_ROOM_SKYBOX].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS_OPAQUE].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS_CUTOUT].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS_BLENDED].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CHILD_ROOM_SKYBOX].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CHILD_ROOM_OBJECTS_OPAQUE].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CHILD_ROOM_OBJECTS_CUTOUT].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CHILD_ROOM_OBJECTS_BLENDED].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CURRENT_ROOM_PORTAL_DEPTH_REFRESH].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CURRENT_ROOM_PORTAL_DECORATIONS].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::MENU].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::AVATARS].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::CURSOR].reserve(cmd_vec_size);
        m_abstractRenderer->m_scoped_render_commands_cache[cache_index][(int)RENDERER::RENDER_SCOPE::OVERLAYS].reserve(cmd_vec_size);
    }

    m_abstractRenderer->m_scoped_light_containers_cache.resize(3);
}

void Renderer::Initialize()
{
    m_abstractRenderer = std::unique_ptr<AbstractRenderer>(new AbstractRenderer());

    InitializeScopes();
    m_abstractRenderer->Initialize();    
    Renderer::m_pimpl = this;
    m_abstractRenderer->InitializeGLObjects();

    qDebug() << "Renderer::Initialize()" << Renderer::m_pimpl << m_abstractRenderer.get();
}

void Renderer::InitializeState()
{
    m_abstractRenderer->InitializeState();
}

void Renderer::InitializeLightUBOs()
{
    m_abstractRenderer->InitializeLightUBOs();
}

void Renderer::InitializeHMDManager(QPointer<AbstractHMDManager> p_hmd_manager)
{
    m_abstractRenderer->InitializeHMDManager(p_hmd_manager);
}

QPointer<ProgramHandle> Renderer::CompileAndLinkShaderProgram(QByteArray * p_vertex_shader, QString p_vertex_shader_path, QByteArray * p_fragment_shader, QString p_fragment_shader_path)
{
//    qDebug() << "Renderer::CompileAndLinkShaderProgram" << this;
    return m_abstractRenderer->CompileAndLinkShaderProgram(p_vertex_shader, p_vertex_shader_path, p_fragment_shader, p_fragment_shader_path);
}

QPointer<ProgramHandle> Renderer::GetDefaultObjectShaderProgram()
{
    return m_abstractRenderer->GetDefaultObjectShaderProgram();
}

QPointer<ProgramHandle> Renderer::GetDefaultSkyboxShaderProgram()
{
    return m_abstractRenderer->GetDefaultSkyboxShaderProgram();
}

QPointer<ProgramHandle> Renderer::GetDefaultPortalShaderProgram()
{
    return m_abstractRenderer->GetDefaultPortalShaderProgram();
}

void Renderer::SetCameras(QVector<VirtualCamera> *p_cameras)
{
    m_abstractRenderer->SetCameras(p_cameras);
}

void Renderer::SetDefaultFontGlyphAtlas(QPointer<TextureHandle> p_handle)
{
    m_abstractRenderer->SetDefaultFontGlyphAtlas(p_handle);
}

TextureHandle* Renderer::GetDefaultFontGlyphAtlas()
{
    return m_abstractRenderer->GetDefaultFontGlyphAtlas();
}

QPointer<TextureHandle> Renderer::CreateCubemapTextureHandleFromAssetImages(QVector<QPointer<AssetImageData>>& p_skybox_image_data, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace)
{
    return m_abstractRenderer->CreateCubemapTextureHandleFromAssetImages(p_skybox_image_data, tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
}

QPointer<TextureHandle> Renderer::CreateTextureFromAssetImageData(QPointer<AssetImageData> data, bool is_left, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace)
{
    return m_abstractRenderer->CreateTextureFromAssetImageData(data, is_left, tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
}

QPointer<TextureHandle> Renderer::CreateTextureFromGLIData(const QByteArray & ba, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace)
{
    return m_abstractRenderer->CreateTextureFromGLIData(ba, tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
}

QPointer<TextureHandle> Renderer::CreateTextureQImage(const QImage & img, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace)
{
    return m_abstractRenderer->CreateTextureQImage(img, tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
}

QPointer<TextureHandle> Renderer::CreateCubemapTextureHandle(const uint32_t p_width, const uint32_t p_height, const TextureHandle::COLOR_SPACE p_color_space, const int32_t p_internal_texture_format, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace)
{
    return m_abstractRenderer->CreateCubemapTextureHandle(p_width, p_height, p_color_space, p_internal_texture_format, tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
}

QPointer<TextureHandle> Renderer::CreateCubemapTextureHandleFromTextureHandles(QVector<QPointer<AssetImageData> > &p_skybox_image_data, QVector<QPointer <TextureHandle> > &p_skybox_image_handles, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace)
{
    return m_abstractRenderer->CreateCubemapTextureHandleFromTextureHandles(p_skybox_image_data, p_skybox_image_handles, tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
}

void Renderer::GenerateEnvMapsFromCubemapTextureHandle(Cubemaps &p_cubemaps)
{
    m_abstractRenderer->GenerateEnvMapsFromCubemapTextureHandle(p_cubemaps);
}

void Renderer::UpdateTextureHandleData(TextureHandle* p_handle, uint const p_level, uint const p_x_offset, uint const p_y_offset, uint const p_width, uint const p_height, uint const p_pixel_size, void* const p_pixel_data)
{
	return m_abstractRenderer->UpdateTextureHandleData(p_handle, p_level, p_x_offset, p_y_offset, p_width, p_height, p_pixel_size, p_pixel_data);
}

void Renderer::UpdateTextureHandleData(TextureHandle* p_handle, uint const p_level, uint const p_x_offset, uint const p_y_offset, uint const p_width, uint const p_height, int const p_pixel_format, int const p_pixel_type, void* const p_pixel_data, uint32_t const p_data_size)
{
    return m_abstractRenderer->UpdateTextureHandleData(p_handle, p_level, p_x_offset, p_y_offset, p_width, p_height, p_pixel_format, p_pixel_type, p_pixel_data, p_data_size);
}

void Renderer::GenerateTextureHandleMipMap(TextureHandle* p_handle)
{
    return m_abstractRenderer->GenerateTextureHandleMipMap(p_handle);
}

void Renderer::CreateMeshHandleForGeomVBOData(GeomVBOData & p_VBO_data)
{
	m_abstractRenderer->CreateMeshHandleForGeomVBOData(p_VBO_data);
}

QPointer<MeshHandle> Renderer::CreateMeshHandle(VertexAttributeLayout p_layout)
{
    return m_abstractRenderer->CreateMeshHandle(p_layout);
}

void Renderer::BindMeshHandle(QPointer <MeshHandle> p_mesh_handle)
{
	m_abstractRenderer->BindMeshHandle(p_mesh_handle);
}

QVector<QPointer<BufferHandle>> Renderer::GetBufferHandlesForMeshHandle(QPointer<MeshHandle> p_mesh_handle)
{
	return m_abstractRenderer->GetBufferHandlesForMeshHandle(p_mesh_handle);
}

void Renderer::RemoveMeshHandleFromMap(QPointer<MeshHandle> p_handle)
{
    m_abstractRenderer->RemoveMeshHandleFromMap(p_handle);
}

QPointer<BufferHandle> Renderer::CreateBufferHandle(BufferHandle::BUFFER_TYPE const p_buffer_type, BufferHandle::BUFFER_USAGE const p_buffer_usage)
{
	return m_abstractRenderer->CreateBufferHandle(p_buffer_type, p_buffer_usage);
}

void Renderer::BindBufferHandle(QPointer<BufferHandle> p_buffer_handle, BufferHandle::BUFFER_TYPE const p_buffer_type)
{
	m_abstractRenderer->BindBufferHandle(p_buffer_handle, p_buffer_type);
}

void Renderer::BindBufferHandle(QPointer<BufferHandle> p_buffer_handle)
{
	m_abstractRenderer->BindBufferHandle(p_buffer_handle);
}

void Renderer::ConfigureBufferHandleData(QPointer<BufferHandle> p_buffer_handle, uint32_t const p_data_size, void* const p_data, BufferHandle::BUFFER_USAGE const p_buffer_usage)
{
	m_abstractRenderer->ConfigureBufferHandleData(p_buffer_handle, p_data_size, p_data, p_buffer_usage);
}

void Renderer::UpdateBufferHandleData(QPointer<BufferHandle> p_buffer_handle, uint32_t const p_offset, uint32_t const p_data_size, void* const p_data)
{
    m_abstractRenderer->UpdateBufferHandleData(p_buffer_handle, p_offset, p_data_size, p_data);
}

void Renderer::RemoveBufferHandleFromMap(QPointer <BufferHandle> p_handle)
{
    m_abstractRenderer->RemoveBufferHandleFromMap(p_handle);
}

bool Renderer::GetIsEnhancedDepthPrecisionSupported() const
{
    return m_abstractRenderer->GetIsEnhancedDepthPrecisionSupported();
}

bool Renderer::GetIsUsingEnhancedDepthPrecision() const
{
    return m_abstractRenderer->GetIsUsingEnhancedDepthPrecision();
}

void Renderer::SetIsUsingEnhancedDepthPrecision(bool const p_is_using)
{
    m_abstractRenderer->SetIsUsingEnhancedDepthPrecision(p_is_using);
}

void Renderer::SetDefaultFaceCullMode(FaceCullMode p_face_cull_mode)
{
    m_abstractRenderer->SetDefaultFaceCullMode(p_face_cull_mode);
}

FaceCullMode Renderer::GetDefaultFaceCullMode() const
{
    return m_abstractRenderer->GetDefaultFaceCullMode();
}

void Renderer::SetMirrorMode(bool p_mirror_mode)
{
    m_abstractRenderer->SetMirrorMode(p_mirror_mode);
}

bool Renderer::GetMirrorMode() const
{
    return m_abstractRenderer->GetMirrorMode();
}

void Renderer::SetDepthFunc(DepthFunc p_depth_func)
{
    m_abstractRenderer->SetDepthFunc(p_depth_func);
}

DepthFunc Renderer::GetDepthFunc() const
{
    return m_abstractRenderer->GetDepthFunc();
}

void Renderer::SetDepthMask(DepthMask p_depth_mask)
{
    m_abstractRenderer->SetDepthMask(p_depth_mask);
}

DepthMask Renderer::GetDepthMask() const
{
    return m_abstractRenderer->GetDepthMask();
}

void Renderer::SetStencilFunc(StencilFunc p_stencil_func)
{
    m_abstractRenderer->SetStencilFunc(p_stencil_func);
}

StencilFunc Renderer::GetStencilFunc() const
{
    return m_abstractRenderer->GetStencilFunc();
}

void Renderer::SetStencilOp(StencilOp p_stencil_op)
{
    m_abstractRenderer->SetStencilOp(p_stencil_op);
}

StencilOp Renderer::GetStencilOp() const
{
    return m_abstractRenderer->GetStencilOp();
}

void Renderer::SetColorMask(ColorMask p_color_mask)
{
    m_abstractRenderer->SetColorMask(p_color_mask);
}

ColorMask Renderer::GetColorMask() const
{
    return m_abstractRenderer->GetColorMask();
}

TextureSet Renderer::GetCurrentlyBoundTextures()
{
    return m_abstractRenderer->GetCurrentlyBoundTextures();
}

int Renderer::GetTextureWidth(TextureHandle* p_handle)
{
	return m_abstractRenderer->GetTextureWidth(p_handle);
}

int Renderer::GetTextureHeight(TextureHandle* p_handle)
{
	return m_abstractRenderer->GetTextureHeight(p_handle);
}

void Renderer::PreRender(QHash<int, QVector<AbstractRenderCommand> > & , QHash<StencilReferenceValue, LightContainer> & )
{
    //m_abstractRenderer->PreRender(p_scoped_render_commands, p_scoped_light_containers);
}

void Renderer::PostRender(QHash<int, QVector<AbstractRenderCommand> > & , QHash<StencilReferenceValue, LightContainer> & )
{
    //m_abstractRenderer->PostRender(p_scoped_render_commands, p_scoped_light_containers);
}

void Renderer::BindTextureHandle(uint32_t p_slot_index, TextureHandle* p_id)
{
    m_abstractRenderer->BindTextureHandle(m_abstractRenderer->m_texture_handle_to_GL_ID, p_slot_index, p_id);
}

uint64_t Renderer::GetLastSubmittedFrameID()
{
    return m_abstractRenderer->m_submitted_frame_id;
}

void Renderer::SubmitFrame()
{
    // Sort commands on the main-thread to avoid stutters or frametime impact on the render-thread
    //SortRenderCommandsByDistance(m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_current_submission_index][static_cast<int>(RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS_OPAQUE)], false);
    //SortRenderCommandsByDistance(m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_current_submission_index][static_cast<int>(RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS_CUTOUT)], false);
    SortRenderCommandsByDistance(m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_current_submission_index][static_cast<int>(RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS_BLENDED)], true);
    SortRenderCommandsByDistance(m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_current_submission_index][static_cast<int>(RENDERER::RENDER_SCOPE::CURRENT_ROOM_PORTAL_DECORATIONS)], true);
    //SortRenderCommandsByDistance(m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_current_submission_index][static_cast<int>(RENDERER::RENDER_SCOPE::CHILD_ROOM_OBJECTS_OPAQUE)], false);
    //SortRenderCommandsByDistance(m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_current_submission_index][static_cast<int>(RENDERER::RENDER_SCOPE::CHILD_ROOM_OBJECTS_CUTOUT)], false);
    SortRenderCommandsByDistance(m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_current_submission_index][static_cast<int>(RENDERER::RENDER_SCOPE::CHILD_ROOM_OBJECTS_BLENDED)], true);

    // Swap our submission indices round
    qSwap(m_abstractRenderer->m_current_submission_index, m_abstractRenderer->m_completed_submission_index);
    m_abstractRenderer->m_submitted_frame_id++;
    m_abstractRenderer->m_draw_id = 0;

    // Erase the existing contents of the vector we are now going to be pushing AbstractRenderCommands into
    // This shouldn't deallocate the memory, only call destructors and change the size member of the vector
    // which is what we want as allocating/deallocating every frame would be costly.
    for (const RENDERER::RENDER_SCOPE scope :m_abstractRenderer->m_scopes)
    {
        m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_current_submission_index][static_cast<int>(scope)].erase(
                    m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_current_submission_index][static_cast<int>(scope)].begin(),
                    m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_current_submission_index][static_cast<int>(scope)].end());
    }
}

void Renderer::RequestScreenShot(uint32_t const p_width, uint32_t const p_height, uint32_t const p_sample_count, bool const p_is_equi, uint64_t p_frame_index)
{
    m_abstractRenderer->RequestScreenShot(p_width, p_height, p_sample_count, p_is_equi, p_frame_index);
}

void Renderer::Render()
{
    m_abstractRenderer->Render(&(m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_completed_submission_index]),
                               &(m_abstractRenderer->m_scoped_light_containers_cache[m_abstractRenderer->m_completed_submission_index]));
}

void Renderer::SortRenderCommandsByDistance(QVector<AbstractRenderCommand>& render_command_vector, bool const p_is_transparent)
{
    int command_count = render_command_vector.size();
    m_sorted_command_indices.clear();
    m_sorted_command_indices.reserve(command_count);

    // Build compacted data structure for cheaper sorting
    for (int index = 0; index < command_count; ++index)
    {
        m_sorted_command_indices.push_back(AbstractRenderCommand_sort(render_command_vector[index], index));
    }

    // Sort the compacted info
    if (p_is_transparent == false)
    {
        std::sort(m_sorted_command_indices.begin(), m_sorted_command_indices.end(),
                    []
                    (AbstractRenderCommand_sort& a, AbstractRenderCommand_sort& b)
                    {
                        // The by draw layer
                        if (a.m_draw_layer != b.m_draw_layer)
                        {
                          return (a.m_draw_layer < b.m_draw_layer);
                        }
                        // Then by distance in front-to-back ordering
                        else if (a.m_room_space_distance != b.m_room_space_distance)
                        {
                          return (a.m_room_space_distance < b.m_room_space_distance);
                        }
                        // Then by draw_id to group draws of the same object together
                        else  if (a.m_draw_id != b.m_draw_id)
                        {
                          return (a.m_draw_id < b.m_draw_id);
                        }
                        // Finally by camera_id so that sequential commands match expected camera indices
                        else
                        {
                          return (a.m_camera_id < b.m_camera_id);
                        }
                    });
    }
    else
    {
        // Need to sort by ref as multiple child rooms exist in this vector and can't be reordered
        std::sort(m_sorted_command_indices.begin(), m_sorted_command_indices.end(),
                    []
                    (AbstractRenderCommand_sort& a, AbstractRenderCommand_sort& b)
                    {
                        // Sort first by room
                        if (a.m_stencil_ref_value != b.m_stencil_ref_value)
                        {
                          return (a.m_stencil_ref_value < b.m_stencil_ref_value);
                        }
                        // The by draw layer
                        else if (a.m_draw_layer != b.m_draw_layer)
                        {
                          return (a.m_draw_layer < b.m_draw_layer);
                        }
                        // Then by distance in back-to-front ordering
                        else if (a.m_room_space_distance != b.m_room_space_distance)
                        {
                          return (a.m_room_space_distance > b.m_room_space_distance);
                        }
                        // Then by draw_id to group draws of the same object together
                        else  if (a.m_draw_id != b.m_draw_id)
                        {
                          return (a.m_draw_id < b.m_draw_id);
                        }
                        // Finally by camera_id so that sequential commands match expected camera indices
                        else
                        {
                          return (a.m_camera_id < b.m_camera_id);
                        }
                    });
    }

    // Use the indices sorted in our now re-ordered vector to in-place alter the original unsorted vector
    // This ensures the minimal number of moves of the expensive shared pointers and is likely to cause no moves
    // in the subsequent frames that re-use this same command vector
    {
        int i_sort, j_sort, k_sort;
        AbstractRenderCommand t;
        for (i_sort = 0; i_sort < command_count; i_sort++)
        {
            if (i_sort != m_sorted_command_indices[i_sort].m_original_index)
            {
                t = render_command_vector[i_sort];
                k_sort = i_sort;
                while (i_sort != (j_sort = m_sorted_command_indices[k_sort].m_original_index))
                {
                    // every move places a value in it's final location
                    render_command_vector[k_sort] = render_command_vector[j_sort];
                    m_sorted_command_indices[k_sort].m_original_index = k_sort;
                    k_sort = j_sort;
                }
                render_command_vector[k_sort] = t;
                m_sorted_command_indices[k_sort].m_original_index = k_sort;
            }
        }
    }
}

void Renderer::PushAbstractRenderCommand(AbstractRenderCommand& p_object_render_command)
{
    // To remove any special case code for objects viewed in a mirror throughout the rest of the codebase
    // I do the face cull mode flipping here at the point where they are submitted to the renderer.
    if (GetMirrorMode() == true)
    {
        switch (p_object_render_command.m_active_face_cull_mode)
        {
        case FaceCullMode::BACK:
            p_object_render_command.m_active_face_cull_mode = FaceCullMode::FRONT;
            break;
        case FaceCullMode::FRONT:
            p_object_render_command.m_active_face_cull_mode = FaceCullMode::BACK;
            break;
        default:
            break;
        }
    }

    QPointer <ProgramHandle> shader = p_object_render_command.GetShaderRef();
    if (((shader) != nullptr) && (p_object_render_command.GetMeshHandle() != nullptr))
    {

        bool const scope_has_transparency_pass = (   m_current_scope == RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS
                                                     || m_current_scope == RENDERER::RENDER_SCOPE::CHILD_ROOM_OBJECTS
                                                     );

        bool const is_command_material_transparent = p_object_render_command.IsMaterialTransparent();
        bool const is_current_room = p_object_render_command.GetStencilFunc().GetStencilReferenceValue() == 0.0;

        RENDERER::RENDER_SCOPE command_vector_scope = m_current_scope;

        if (m_current_scope == RENDERER::RENDER_SCOPE::CURSOR)
        {
            // Force cursor to use linear alpha shader
            p_object_render_command.SetShader(m_abstractRenderer->m_default_object_shader_linear_alpha);
        }
        if (m_current_scope == RENDERER::RENDER_SCOPE::OVERLAYS)
        {
            // Force cursor to use linear alpha shader
            p_object_render_command.SetShader(m_abstractRenderer->m_default_object_shader_linear_alpha);
        }


        // If we need discards alter the shader to have them
        // Not having discard calls in the default object shader is a significant performance increase
        if (is_command_material_transparent == true
                && scope_has_transparency_pass == true)
        {
            TextureHandle::ALPHA_TYPE material_alpha_type = p_object_render_command.GetAlphaType();
            switch (material_alpha_type)
            {
            // Cutouts only, opaque pass, requires discards
            case TextureHandle::ALPHA_TYPE::CUTOUT:
                if (shader->m_UUID.m_UUID == m_abstractRenderer->m_default_object_shader->m_UUID.m_UUID)
                {
                    p_object_render_command.SetShader(m_abstractRenderer->m_default_object_shader_binary_alpha);
                }
                command_vector_scope = (is_current_room) ? RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS_CUTOUT : RENDERER::RENDER_SCOPE::CHILD_ROOM_OBJECTS_CUTOUT;
                break;
                // Alpha blending via object/material constants, transparency pass required
                // For textures with a mix of full-opacity and blended alpha we still put it into the transparency pass
                // so that it can blend with the opaques but keep depth testing enabled so that it doesn't have ordering
                // issues with the fully opaque parts of itself.
            case TextureHandle::ALPHA_TYPE::NONE:
                // Alpha blending via texture, transparency pass
            case TextureHandle::ALPHA_TYPE::BLENDED:
                if (shader->m_UUID.m_UUID == m_abstractRenderer->m_default_object_shader->m_UUID.m_UUID)
                {
                    p_object_render_command.SetShader(m_abstractRenderer->m_default_object_shader_linear_alpha);
                    p_object_render_command.SetDepthMask(DepthMask::DEPTH_WRITES_DISABLED);
                }
                command_vector_scope = (is_current_room) ? RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS_BLENDED : RENDERER::RENDER_SCOPE::CHILD_ROOM_OBJECTS_BLENDED;
                break;
            case TextureHandle::ALPHA_TYPE::MIXED:
                if (shader->m_UUID.m_UUID == m_abstractRenderer->m_default_object_shader->m_UUID.m_UUID)
                {
                    p_object_render_command.SetShader(m_abstractRenderer->m_default_object_shader_linear_alpha);
                }
                command_vector_scope = (is_current_room) ? RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS_BLENDED : RENDERER::RENDER_SCOPE::CHILD_ROOM_OBJECTS_BLENDED;
                break;
            default:
                break;
            }
        }
        else if (scope_has_transparency_pass == true && is_command_material_transparent == false)
        {
            command_vector_scope = (is_current_room) ? RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS_OPAQUE : RENDERER::RENDER_SCOPE::CHILD_ROOM_OBJECTS_OPAQUE;
        }

        auto & command_vector = m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_current_submission_index][(int)command_vector_scope];

        const int camera_count = m_abstractRenderer->m_scoped_cameras_cache[m_abstractRenderer->m_current_submission_index][(int)command_vector_scope].size();

        p_object_render_command.m_draw_id = m_abstractRenderer->m_draw_id;

        for (int camera_index = 0; camera_index < camera_count; ++camera_index)
        {
            p_object_render_command.m_camera_id = static_cast<uint32_t>(camera_index);
            command_vector.push_back(p_object_render_command);
        }
    }
    m_abstractRenderer->m_draw_id++;
}

void Renderer::RenderObjects()
{
    m_abstractRenderer->Render(&(m_abstractRenderer->m_scoped_render_commands_cache[m_abstractRenderer->m_rendering_index]), &(m_abstractRenderer->m_scoped_light_containers_cache[m_abstractRenderer->m_rendering_index]));
}

void Renderer::PushLightContainer(LightContainer const * p_light_container, StencilReferenceValue p_room_stencil_ref)
{
    m_abstractRenderer->m_scoped_light_containers_cache[m_abstractRenderer->m_current_submission_index][p_room_stencil_ref] = *p_light_container;
}

void Renderer::BeginScope(RENDERER::RENDER_SCOPE p_scope)
{
    m_current_scope = p_scope;
    //qDebug() << "Renderer::BeginScope" << m_current_scope;
}

void Renderer::EndCurrentScope()
{
    //qDebug() << "Renderer::EndCurrentScope" << m_current_scope;
    m_current_scope = RENDERER::RENDER_SCOPE::NONE;
}

RENDERER::RENDER_SCOPE Renderer::GetCurrentScope()
{
    return m_current_scope;
}

QVector<GLuint64> & Renderer::GetGPUTimeQueryResults()
{
    return m_abstractRenderer->GetGPUTimeQueryResults();
}

QVector<uint64_t> & Renderer::GetCPUTimeQueryResults()
{
    return m_abstractRenderer->GetCPUTimeQueryResults();
}

int64_t Renderer::GetFrameCounter()
{
    return (m_abstractRenderer ? m_abstractRenderer->GetFrameCounter() : 0);
}

int Renderer::GetNumTextures() const
{
    return (m_abstractRenderer ? m_abstractRenderer->GetNumTextures() : 0);
}

QString Renderer::GetRendererName() const
{
    return (m_abstractRenderer ? m_abstractRenderer->GetRendererName() : QString());
}

int Renderer::GetRendererMajorVersion() const
{
    return (m_abstractRenderer ? m_abstractRenderer->GetRendererMajorVersion() : 0);
}

int Renderer::GetRendererMinorVersion() const
{
    return (m_abstractRenderer ? m_abstractRenderer->GetRendererMinorVersion() : 0);
}

QPointer<MeshHandle> Renderer::GetSkyboxCubeVAO()
{
    return m_abstractRenderer->m_skycube_vao;
}

GLuint Renderer::GetSkyboxCubePrimCount() const
{
    return 36;
}

QPointer<MeshHandle> Renderer::GetTexturedCubeVAO()
{
    return m_abstractRenderer->m_slab_vao;
}

GLuint Renderer::GetTexturedCubePrimCount() const
{
    return 36;
}

QPointer<MeshHandle> Renderer::GetTexturedCube2VAO()
{
    return m_abstractRenderer->m_cube_vao;
}

GLuint Renderer::GetTexturedCube2PrimCount() const
{
    return 36;
}

QPointer<MeshHandle> Renderer::GetTexturedCube3VAO()
{
    return m_abstractRenderer->m_cube3_vao;
}

GLuint Renderer::GetTexturedCube3PrimCount() const
{
    return 36;
}

QPointer<MeshHandle> Renderer::GetPortalStencilCylinderVAO()
{
    return m_abstractRenderer->m_portal_stencil_cylinder_vao;
}

GLuint Renderer::GetPortalStencilCylinderPrimCount() const
{
    return 72*3;
}

QPointer<MeshHandle> Renderer::GetPortalStencilCubeVAO()
{
    return m_abstractRenderer->m_portal_stencil_cube_vao;
}

GLuint Renderer::GetPortalStencilCubePrimCount() const
{
    return 30;
}

QPointer<MeshHandle> Renderer::GetPlaneVAO()
{
    return m_abstractRenderer->m_plane_vao;
}

GLuint Renderer::GetPlanePrimCount() const
{
    return 6;
}

QPointer<MeshHandle> Renderer::GetDiscVAO()
{
    return m_abstractRenderer->m_disc_vao;
}

GLuint Renderer::GetDiscPrimCount() const
{
    return 72;
}

QPointer<MeshHandle> Renderer::GetConeVAO()
{
    return m_abstractRenderer->m_cone_vao;
}

GLuint Renderer::GetConePrimCount() const
{
    return 72*6;
}

QPointer<MeshHandle> Renderer::GetCone2VAO()
{
    return m_abstractRenderer->m_cone2_vao;
}

GLuint Renderer::GetCone2PrimCount() const
{
    return 72*3;
}

QPointer<MeshHandle> Renderer::GetPyramidVAO()
{
    return m_abstractRenderer->m_pyramid_vao;
}

GLuint Renderer::GetPyramidPrimCount() const
{
    return 24;
}

void Renderer::ConfigureFramebuffer(const uint32_t p_window_width, const uint32_t p_window_height, const uint32_t p_msaa_count)
{
    m_abstractRenderer->ConfigureFramebuffer(p_window_width, p_window_height, p_msaa_count);
}

void Renderer::ConfigureWindowSize(const uint32_t p_window_width, const uint32_t p_window_height)
{
    m_abstractRenderer->ConfigureWindowSize(p_window_width, p_window_height);
}

void Renderer::ConfigureSamples(const uint32_t p_msaa_count)
{
    m_abstractRenderer->ConfigureSamples(p_msaa_count);
}

uint32_t Renderer::GetTextureID(const FBO_TEXTURE_ENUM p_texture_index, const bool p_multisampled) const
{
    return m_abstractRenderer->GetTextureID(p_texture_index, p_multisampled);
}

QVector<uint32_t> Renderer::BindFBOToRead(const FBO_TEXTURE_BITFIELD_ENUM p_textures_bitmask, const bool p_bind_multisampled) const
{
    return m_abstractRenderer->BindFBOToRead(p_textures_bitmask, p_bind_multisampled);
}

QVector<uint32_t> Renderer::BindFBOToDraw(const FBO_TEXTURE_BITFIELD_ENUM p_textures_bitmask, const bool p_bind_multisampled) const
{
    return m_abstractRenderer->BindFBOToDraw(p_textures_bitmask, p_bind_multisampled);
}

void Renderer::BlitMultisampledFramebuffer(const FBO_TEXTURE_BITFIELD_ENUM p_textures_bitmask, int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1, int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1) const
{
    m_abstractRenderer->BlitMultisampledFramebuffer(p_textures_bitmask, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1);
}

void Renderer::BlitMultisampledFramebuffer(const FBO_TEXTURE_BITFIELD_ENUM p_textures_bitmask) const
{
    m_abstractRenderer->BlitMultisampledFramebuffer(p_textures_bitmask);
}

uint32_t Renderer::GetWindowWidth() const
{
    return m_abstractRenderer->GetWindowWidth();
}

uint32_t Renderer::GetWindowHeight() const
{
    return m_abstractRenderer->GetWindowHeight();
}

uint32_t Renderer::GetMSAACount() const
{
    return m_abstractRenderer->GetMSAACount();
}

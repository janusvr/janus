#include "rendererinterface.h"

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
        if (RendererInterface::m_pimpl->GetIsUsingEnhancedDepthPrecision() == true
            && RendererInterface::m_pimpl->GetIsEnhancedDepthPrecisionSupported() == true)
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

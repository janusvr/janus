#include "renderer.h"

RendererInterface* RendererInterface::m_pimpl = nullptr;

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
	RendererInterface::m_pimpl = (RendererInterface*)this;    
    m_abstractRenderer->InitializeGLObjects();

    qDebug() << "Renderer::Initialize()" << RendererInterface::m_pimpl << m_abstractRenderer.get();
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

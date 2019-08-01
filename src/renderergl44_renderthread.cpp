#include "renderergl44_renderthread.h"

#if !defined(__APPLE__)
RendererGL44_RenderThread::RendererGL44_RenderThread()
    : m_max_compute_work_groups(0),
      m_gl_surface(nullptr),
      m_gl_context(nullptr),
      m_gl_funcs(nullptr),
      m_main_thread_fbo(0),
      m_equi_cubemap_face_size(0),
      m_is_rendering(false),
      m_is_initialized(false),
      m_hmd_initialized(false),
      m_capture_frame(false),
      m_frame_time(0),
      m_main_thread_renderer(nullptr),            
      m_screenshot_pbo_pending(false),
      m_screenshot_pbo(0),
      m_frame_vector_sorted(false),
      m_object_buffer_dirty(true),
      m_material_buffer_dirty(true),
      m_camera_buffer_dirty(true),
      m_shutting_down(false)
{
}

RendererGL44_RenderThread::~RendererGL44_RenderThread()
{
}

void RendererGL44_RenderThread::Initialize()
{
    PostConstructorInitialize();
}

void RendererGL44_RenderThread::CreateMeshHandleForGeomVBODataMIRRORCOPY(AbstractRenderer * p_main_thread_renderer, GeomVBOData * p_VBO_data)
{
    // Calling CreateMeshHandleForGeomVBOData here will cause the VAO to be created on the render-thread's GL Context, any attempt to bind a
    // VAO obtained from the MeshHandles stored in GeomVBOData on the main-thread will cause wierd behaviour
    // as those VAO ID are not guaranteed to even exist in the main-thread GL context
    //CreateMeshHandleForGeomVBOData(p_VBO_data);

    VertexAttributeLayout layout;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].element_count = 3;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].element_type = GL_FLOAT;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].stride_in_bytes = 4 * sizeof(float);
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::NORMAL].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::NORMAL].element_count = 3;
    layout.attributes[(uint32_t)VAO_ATTRIB::NORMAL].element_type = GL_FLOAT;
    layout.attributes[(uint32_t)VAO_ATTRIB::NORMAL].stride_in_bytes = 4 * sizeof(float);
    layout.attributes[(uint32_t)VAO_ATTRIB::NORMAL].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].element_count = 2;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].element_type = GL_FLOAT;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].stride_in_bytes = 4 * sizeof(float);
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].buffer_id = VAO_ATTRIB::TEXCOORD0;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].element_count = 2;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].element_type = GL_FLOAT;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].stride_in_bytes = 4 * sizeof(float);
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].offset_in_bytes = 2 * sizeof(float);

    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].element_count = 4;
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].element_type = GL_FLOAT;
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].stride_in_bytes = 4 * sizeof(float);
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].in_use = p_VBO_data->use_skelanim ? true : false;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].element_count = 4;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].element_type = GL_UNSIGNED_BYTE;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].is_normalized = false;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].is_float_attrib = false; // We want a uvec4 not a vec4
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].stride_in_bytes = 4 * sizeof(uint8_t);
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS].in_use = p_VBO_data->use_skelanim ? true : false;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS].element_count = 4;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS].element_type = GL_FLOAT;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS].stride_in_bytes = 4 * sizeof(float);
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].element_count = 1;
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].element_type = GL_UNSIGNED_INT;
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].stride_in_bytes = 1 * sizeof(uint32_t);
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].offset_in_bytes = 0;

    CreateMeshHandle(p_main_thread_renderer, &p_VBO_data->m_mesh_handle,
                     layout);
    p_main_thread_renderer->BindMeshHandle(p_VBO_data->m_mesh_handle.get());

    auto buffer_handles = p_main_thread_renderer->GetBufferHandlesForMeshHandle(p_VBO_data->m_mesh_handle.get());

    p_main_thread_renderer->BindBufferHandle((*buffer_handles)[(uint32_t)VAO_ATTRIB::INDICES].get());
    MathUtil::glFuncs->glBufferData(GL_ELEMENT_ARRAY_BUFFER, p_VBO_data->m_indices.size() * sizeof(uint32_t), p_VBO_data->m_indices.data(), GL_STATIC_DRAW);

    p_main_thread_renderer->BindBufferHandle((*buffer_handles)[(uint32_t)VAO_ATTRIB::POSITION].get());
    MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data->m_positions.size() * sizeof(float), p_VBO_data->m_positions.data(), GL_STATIC_DRAW);

    p_main_thread_renderer->BindBufferHandle((*buffer_handles)[(uint32_t)VAO_ATTRIB::NORMAL].get());
    MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data->m_normals.size() * sizeof(float), p_VBO_data->m_normals.data(), GL_STATIC_DRAW);

    p_main_thread_renderer->BindBufferHandle((*buffer_handles)[(uint32_t)VAO_ATTRIB::TEXCOORD0].get());
    MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data->m_tex_coords.size() * sizeof(float), p_VBO_data->m_tex_coords.data(), GL_STATIC_DRAW);

    p_main_thread_renderer->BindBufferHandle((*buffer_handles)[(uint32_t)VAO_ATTRIB::COLOR].get());
    MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data->m_colors.size() * sizeof(float), p_VBO_data->m_colors.data(), GL_STATIC_DRAW);

    if (p_VBO_data->use_skelanim)
    {
        p_main_thread_renderer->BindBufferHandle((*buffer_handles)[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].get());
        MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data->m_skel_anim_indices.size() * sizeof(uint8_t), p_VBO_data->m_skel_anim_indices.data(), GL_STATIC_DRAW);

        p_main_thread_renderer->BindBufferHandle((*buffer_handles)[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS].get());
        MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data->m_skel_anim_weights.size() * sizeof(float), p_VBO_data->m_skel_anim_weights.data(), GL_STATIC_DRAW);
    }

    // I call this to ensure that multiple queued up calls of this function don't cause large gaps between DecoupledRender calls.
    DecoupledRender();
}

void RendererGL44_RenderThread::CreateMeshHandle(AbstractRenderer *p_main_thread_renderer, std::shared_ptr<MeshHandle> *p_handle, VertexAttributeLayout p_layout)
{
    // Calling CreateMeshHandle here will cause the VAO to be created on the render-thread's GL Context, any attempt to bind a
    // VAO obtained from the MeshHandles stored in GeomVBOData on the main-thread will cause wierd behaviour
    // as those VAO ID are not guaranteed to even exist in the main-thread GL context
    uint32_t VAO_id = 0;
    MathUtil::glFuncs->glGenVertexArrays(1, &VAO_id);

    *p_handle = p_main_thread_renderer->CreateMeshHandle(p_layout, VAO_id);

    // I call this to ensure that multiple queued up calls of this function don't cause large gaps between DecoupledRender calls.
    DecoupledRender();
}

void RendererGL44_RenderThread::Render(QHash<size_t, QVector<AbstractRenderCommand>> * p_scoped_render_commands,
                                 QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers)
{
    Q_UNUSED(p_scoped_render_commands)
    Q_UNUSED(p_scoped_light_containers)
}

void RendererGL44_RenderThread::PreRender(QHash<size_t, QVector<AbstractRenderCommand> > * p_scoped_render_commands, QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers)
{
    Q_UNUSED(p_scoped_light_containers)
    UpdatePerObjectData(p_scoped_render_commands);
}

void RendererGL44_RenderThread::PostRender(QHash<size_t, QVector<AbstractRenderCommand> > * p_scoped_render_commands, QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers)
{
    Q_UNUSED(p_scoped_render_commands)
    Q_UNUSED(p_scoped_light_containers)
}

void RendererGL44_RenderThread::UpgradeShaderSource(QByteArray & p_shader_source, bool p_is_vertex_shader)
{
    // Change iMiscObjectData to an ivec to avoid the need for type casting in the shader
    p_shader_source.replace("uniform mat4 iMiscObjectData;",
                            "uniform ivec4 iMiscObjectData;");

    // Remove unneeded ojbect/instance uniform delcarations
    p_shader_source.replace("uniform vec4 iUseLighting;",                      "");
    p_shader_source.replace("uniform mat4 iModelMatrix;",                      "");
    p_shader_source.replace("uniform mat4 iViewMatrix;",                       "");
    p_shader_source.replace("uniform mat4 iProjectionMatrix;",                 "");
    p_shader_source.replace("uniform mat4 iInverseViewMatrix;",                "");
    p_shader_source.replace("uniform mat4 iModelViewMatrix;",                  "");
    p_shader_source.replace("uniform mat4 iModelViewProjectionMatrix;",        "");
    p_shader_source.replace("uniform mat4 iTransposeInverseModelMatrix;",      "");
    p_shader_source.replace("uniform mat4 iTransposeInverseModelViewMatrix;",  "");
    p_shader_source.replace("uniform vec4 iConstColour;",                 "");
    p_shader_source.replace("uniform vec4 iChromaKeyColour;",                 "");
    p_shader_source.replace("uniform vec4 iUseSkelAnim;",  "");
    p_shader_source.replace("uniform vec4 iObjectPickID;",  "");

    // Remove unneeded material uniform declarations
    p_shader_source.replace("uniform vec4 iAmbient;",           "");
    p_shader_source.replace("uniform vec4 iDiffuse;",           "");
    p_shader_source.replace("uniform vec4 iSpecular;",          "");
    p_shader_source.replace("uniform vec4 iShininess;",         "");
    p_shader_source.replace("uniform vec4 iEmission;",          "");
    p_shader_source.replace("uniform vec4 iLightmapScale;",     "");
    p_shader_source.replace("uniform vec4 iTiling;",            "");
    p_shader_source.replace("uniform vec4 iUseTexture[4];",     "");

    // Per Camera
    p_shader_source.replace("iViewMatrix",                          "per_camera_data_array[__CAMERA_INDEX].iViewMatrix");
    p_shader_source.replace("iInverseViewMatrix",                   "per_camera_data_array[__CAMERA_INDEX].iInverseViewMatrix");
    p_shader_source.replace("iProjectionMatrix",                    "per_camera_data_array[__CAMERA_INDEX].iProjectionMatrix");

    // Per Material
    p_shader_source.replace("iAmbient",                             "per_material_data_array[__MATERIAL_INDEX].iAmbient");
    p_shader_source.replace("iDiffuse",                             "per_material_data_array[__MATERIAL_INDEX].iDiffuse");
    p_shader_source.replace("iSpecular",                            "per_material_data_array[__MATERIAL_INDEX].iSpecular");
    p_shader_source.replace("iShininess",                           "per_material_data_array[__MATERIAL_INDEX].iShininess");
    p_shader_source.replace("iEmission",                            "per_material_data_array[__MATERIAL_INDEX].iEmission");
    p_shader_source.replace("iLightmapScale",                       "per_material_data_array[__MATERIAL_INDEX].iLightmapScale");
    p_shader_source.replace("iTiling",                              "per_material_data_array[__MATERIAL_INDEX].iTiling");
    p_shader_source.replace("iUseTexture",                          "per_material_data_array[__MATERIAL_INDEX].iUseTexture");

    // Per Object
    p_shader_source.replace("iConstColour",                         "per_object_data_array[__OBJECT_INDEX].iConstColour");
    p_shader_source.replace("iChromaKeyColour",                     "per_object_data_array[__OBJECT_INDEX].iChromaKeyColour");
    p_shader_source.replace("iUseSkelAnim.x",                       "per_object_data_array[__OBJECT_INDEX].iUseFlags.x");
    p_shader_source.replace("iUseSkelAnim[0]",                      "per_object_data_array[__OBJECT_INDEX].iUseFlags.x");
    p_shader_source.replace("iUseLighting.x",                       "per_object_data_array[__OBJECT_INDEX].iUseFlags.y");
    p_shader_source.replace("iUseLighting[0]",                      "per_object_data_array[__OBJECT_INDEX].iUseFlags.y");

    // Per Instance
    p_shader_source.replace("iModelMatrix",                         "per_instance_data_array[__INSTANCE_INDEX].iModelMatrix");
    p_shader_source.replace("iTransposeInverseModelMatrix",         "per_instance_data_array[__INSTANCE_INDEX].iTransposeInverseModelMatrix");
    p_shader_source.replace("iModelViewMatrix",                     "per_instance_data_array[__INSTANCE_INDEX].iModelViewMatrix");
    p_shader_source.replace("iModelViewProjectionMatrix",           "per_instance_data_array[__INSTANCE_INDEX].iModelViewProjectionMatrix");
    p_shader_source.replace("iTransposeInverseModelViewMatrix",     "per_instance_data_array[__INSTANCE_INDEX].iTransposeInverseModelViewMatrix");

    if (p_is_vertex_shader)
    {
        // Add in new vertex shader outputs
        p_shader_source.replace("void main",
                                "flat out ivec4 obj_cam_mat_inst;\n"
                                "void main"
                                );

        // Add in new vertex shader viewport and instance ID code
        p_shader_source.replace("gl_Position",
                                "obj_cam_mat_inst.x = __OBJECT_INDEX;\n"
                                "obj_cam_mat_inst.y = __CAMERA_INDEX;\n"
                                "obj_cam_mat_inst.z = __MATERIAL_INDEX;\n"
                                "obj_cam_mat_inst.w = __INSTANCE_INDEX;\n"
                                "gl_ViewportIndex = (gl_InstanceID + int(iViewportCount.y)) % int(iViewportCount.x);\n"
                                "gl_Position"
                                );


        // Per Object
        p_shader_source.replace("__OBJECT_INDEX",                       "per_instance_data_array[__INSTANCE_INDEX].m_obj_cam_mat_indices[0]");

        // Per Camera
        p_shader_source.replace("__CAMERA_INDEX",                       "per_instance_data_array[__INSTANCE_INDEX].m_obj_cam_mat_indices[1]");

        // Per Material
        p_shader_source.replace("__MATERIAL_INDEX",                     "per_instance_data_array[__INSTANCE_INDEX].m_obj_cam_mat_indices[2]");

        // Per Instance
        p_shader_source.replace("__INSTANCE_INDEX",                     "(gl_InstanceID + iMiscObjectData.w)");
    }
    else
    {
        // Add gamma correction step as we output to a linear texture
        prependDataInShaderMainFunction(p_shader_source, g_gamma_correction_GLSL);

        // Add in new fragment shader inputs
        p_shader_source.replace("void main",
                                "flat in ivec4 obj_cam_mat_inst;\n"
                                "void main"
                                );

        // Per Object
        p_shader_source.replace("__OBJECT_INDEX",                       "obj_cam_mat_inst.x");

        // Per Camera
        p_shader_source.replace("__CAMERA_INDEX",                       "obj_cam_mat_inst.y");

        // Per Material
        p_shader_source.replace("__MATERIAL_INDEX",                     "obj_cam_mat_inst.z");

        // Per Instance
        p_shader_source.replace("__INSTANCE_INDEX",                     "obj_cam_mat_inst.w");
    }

    // Replace GLSL 330 define with GLSL 440 define, extensions, and SSBO info
    p_shader_source.replace("#version 330 core",
                            "#version 440 core\n"
                            "#extension GL_AMD_vertex_shader_viewport_index : enable\n"
                            "#extension GL_NV_viewport_array2 : enable\n"
                            "uniform vec4 iViewportCount;\n"
                            "struct Per_object_data\n"
                            "{\n"
                            "    vec4 iConstColour;\n"
                            "    vec4 iChromaKeyColour;\n"
                            "    vec4 iUseFlags;\n"
                            "};\n"
                            "layout(std430, binding = 1) buffer per_object_data\n"
                            "{\n"
                            "    Per_object_data per_object_data_array[];\n"
                            "};\n"
                            "struct Per_instance_data\n"
                            "{\n"
                            "    ivec4 m_obj_cam_mat_indices;\n"
                            "    mat4 iModelMatrix;\n"
                            "    mat4 iTransposeInverseModelMatrix;\n"
                            "    mat4 iModelViewMatrix;\n"
                            "    mat4 iModelViewProjectionMatrix;\n"
                            "    mat4 iTransposeInverseModelViewMatrix;\n"
                            "};\n"
                            "layout(std430, binding = 2) buffer per_instance_data\n"
                            "{\n"
                            "    Per_instance_data per_instance_data_array[];\n"
                            "};\n"
                            "struct Per_camera_data\n"
                            "{\n"
                            "    mat4 iViewMatrix;\n"
                            "    mat4 iProjectionMatrix;\n"
                            "    mat4 iInverseViewMatrix;\n"
                            "};\n"
                            "layout(std430, binding = 3) buffer per_camera_data\n"
                            "{\n"
                            "    Per_camera_data per_camera_data_array[];\n"
                            "};\n"
                            "struct Per_material_data\n"
                            "{\n"
                            "    vec4 iAmbient;\n"
                            "    vec4 iDiffuse;\n"
                            "    vec4 iSpecular;\n"
                            "    vec4 iShininess;\n"
                            "    vec4 iEmission;\n"
                            "    vec4 iTiling;\n"
                            "    vec4 iLightmapScale;\n"
                            "    vec4 iUseTexture[4];\n"
                            "};\n"
                            "layout(std430, binding = 4) buffer per_material_data\n"
                            "{\n"
                            "    Per_material_data per_material_data_array[];\n"
                            "};\n"
                            );
    /*QFile file("D:/Janus-vr_utils/janusvr_utils/assets/shaders/vert440.vert");
    bool does_exist = file.exists();
    file.open(QIODevice::WriteOnly);
    file.write(p_shader_source);
    file.close();*/
}

void RendererGL44_RenderThread::UpdatePerObjectData(QHash<size_t, QVector<AbstractRenderCommand>> * p_scoped_render_commands)
{
    QMatrix4x4 temp_matrix;
    PerCameraSSBOData testCam;
    PerObjectSSBOData testObj;
    //PerMaterialSSBOData testMat;
    int32_t object_SSBO_offset = -1;
    int32_t instance_SSBO_offset = -1;
    int32_t material_SSBO_offset = -1;
    //int32_t camera_SSBO_offset = -1;
    //AssetShader_Material const * material = nullptr;
    // These clear calls do not deallocate, only call cheap destructors and set size to 0
    m_per_instance_data.clear();
    m_per_camera_data.clear();
    if (m_object_buffer_dirty == true)
    {
         m_per_object_data.clear();
    }

    // Bind the next chunk for PerInstanceData
    //m_per_instance_buffer.bindNextChunk();
    PerInstanceSSBOData* per_instance_data = (PerInstanceSSBOData*) (m_per_instance_buffer.m_ptr + m_per_instance_buffer.m_offset);

    // Resize to this frame's camera vectors to fit the current frame's cameras for each scope
    for (const RENDERER::RENDER_SCOPE scope : m_scopes)
    {
        size_t const camera_count_this_scope = m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][static_cast<size_t>(scope)].size();
        m_per_frame_scoped_cameras_view_matrix[static_cast<size_t>(scope)].resize(camera_count_this_scope);
        m_per_frame_scoped_cameras_is_left_eye[static_cast<size_t>(scope)].resize(camera_count_this_scope);
    }

    // Generate the view matrices and is_left_eye data for this frame's cameras
    if (m_main_thread_renderer->m_hmd_manager != nullptr && m_main_thread_renderer->m_hmd_manager->GetEnabled() == true)
    {
        const QMatrix4x4& eye_view_matrix_L = m_main_thread_renderer->m_hmd_manager->GetEyeViewMatrix(0);
        const QMatrix4x4& eye_view_matrix_R = m_main_thread_renderer->m_hmd_manager->GetEyeViewMatrix(1);

        for (const RENDERER::RENDER_SCOPE scope : m_scopes)
        {
            size_t const camera_count_this_scope = m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][static_cast<size_t>(scope)].size();
            for (size_t camera_index = 0; camera_index < camera_count_this_scope; ++camera_index)
            {
                QMatrix4x4 composited_view_matrix = m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][static_cast<size_t>(scope)][camera_index].GetViewMatrix();
                m_per_frame_scoped_cameras_is_left_eye[static_cast<size_t>(scope)][camera_index] = m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][static_cast<size_t>(scope)][camera_index].GetLeftEye();
                composited_view_matrix = ((m_per_frame_scoped_cameras_is_left_eye[static_cast<size_t>(scope)][camera_index]) ? eye_view_matrix_L : eye_view_matrix_R) * composited_view_matrix;
                m_per_frame_scoped_cameras_view_matrix[static_cast<size_t>(scope)][camera_index] = composited_view_matrix;

                // Update camera viewports, this takes into account things like dynamic resolution scaling
                m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][static_cast<size_t>(scope)][camera_index].SetViewport(
                            (m_per_frame_scoped_cameras_is_left_eye[static_cast<size_t>(scope)][camera_index] == true)
                        ? m_main_thread_renderer->m_hmd_manager->m_eye_viewports[0]
                        : m_main_thread_renderer->m_hmd_manager->m_eye_viewports[1]);
            }
        }
    }
    else
    {
        for (const RENDERER::RENDER_SCOPE scope : m_scopes)
        {
            size_t const camera_count_this_scope = m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][static_cast<size_t>(scope)].size();
            for (size_t camera_index = 0; camera_index < camera_count_this_scope; ++camera_index)
            {
                m_per_frame_scoped_cameras_view_matrix[static_cast<size_t>(scope)][camera_index] = m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][static_cast<size_t>(scope)][camera_index].GetViewMatrix();
                m_per_frame_scoped_cameras_is_left_eye[static_cast<size_t>(scope)][camera_index] = m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][static_cast<size_t>(scope)][camera_index].GetLeftEye();
            }
        }
    }
    m_camera_to_camera_SSBO.clear();
    for (const RENDERER::RENDER_SCOPE scope : m_scopes)
    {
        QVector<AbstractRenderCommand> & render_command_vector = (*p_scoped_render_commands)[static_cast<size_t>(scope)];

        auto const command_count(render_command_vector.size());
        auto const camera_count_this_scope(m_per_frame_scoped_cameras_view_matrix[static_cast<size_t>(scope)].size());

        // Build per_camera SSBO data
        size_t camera_index = 0;
        //size_t const camera_scope_offset = (camera_SSBO_offset + 1);

        m_camera_to_camera_SSBO.resize(camera_count_this_scope);
        for (camera_index = 0; camera_index < camera_count_this_scope; ++camera_index)
        {
            VirtualCamera const & camera = m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][static_cast<size_t>(scope)][camera_index];
            QMatrix4x4 const & view_matrix = m_per_frame_scoped_cameras_view_matrix[static_cast<size_t>(scope)][camera_index];

            std::memcpy(testCam.m_viewmatrix, view_matrix.constData(), 16 * sizeof(float));

            temp_matrix = view_matrix.inverted();
            std::memcpy(testCam.m_inverseViewMatrix, temp_matrix.constData(), 16 * sizeof(float));

            std::memcpy(testCam.m_projectionMatrix, camera.GetProjectionMatrix().constData(), 16 * sizeof(float));

            auto result = std::find(m_per_camera_data.begin(), m_per_camera_data.end(), testCam);
            if (result == m_per_camera_data.end())
            {
                m_camera_to_camera_SSBO[camera_index] = static_cast<int32_t>(m_per_camera_data.size());
                m_per_camera_data.push_back(testCam);
            }
            else
            {
                m_camera_to_camera_SSBO[camera_index] = static_cast<int32_t>(std::distance(m_per_camera_data.begin(), result));
            }
        }

        // For each command
        for (uint32_t command_index = 0; command_index < command_count; command_index += camera_count_this_scope)
        {
            AssetShader_Material const * material = render_command_vector[command_index].GetMaterialUniformsPointer();

            // Only copy material data if its not already in the material dictionary
            // this helps to cut down on the amount of data needed to transfer each new unique frame
            if (m_object_buffer_dirty == true)
            {
                auto result = std::find(m_per_material_data.begin(), m_per_material_data.end(), (*(PerMaterialSSBOData*)material));
                if (result == m_per_material_data.end())
                {
                    //memcpy(&testMat, (float*)material, 4 * sizeof(AssetShader_Material));
                    material_SSBO_offset = m_per_material_data.size();
                    m_per_material_data.push_back((*(PerMaterialSSBOData*)material));
                    m_material_buffer_dirty = true;
                }
                else
                {
                    material_SSBO_offset = std::distance(m_per_material_data.begin(), result);
                }
            }

            // Recompute per instance data for each command in this scope
            size_t camera_index = 0;
            for (camera_index = 0; camera_index < camera_count_this_scope; ++camera_index)
            {
                //const VirtualCamera& camera = m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][static_cast<size_t>(scope)][camera_index];

                AbstractRenderCommand & render_command = render_command_vector[command_index + camera_index];
                AssetShader_Object & new_object_uniforms = render_command.GetObjectUniformsReference();

                //memcpy((char*)model_matrix.constData(), new_object_uniforms.iModelMatrix, 16 * sizeof(float));
                //model_matrix.optimize(); //56.0 - call optimize so internal type is not identity and inverse does nothing

                // Create PerObject data for the first camera
                if (camera_index == 0 && m_object_buffer_dirty == true)
                {
                    // This copies iConstColour + iChromaKeyColour + iUseFlags, they are back to back in memory
                    std::memcpy(&testObj.m_constColour[0], &new_object_uniforms.iConstColour[0], 12 * sizeof(float));

                    auto result = std::find(m_per_object_data.begin(), m_per_object_data.end(), testObj);
                    if (result == m_per_object_data.end())
                    {
                        object_SSBO_offset = static_cast<int32_t>(m_per_object_data.size());
                        m_per_object_data.push_back(testObj);
                    }
                    else
                    {
                        object_SSBO_offset = static_cast<int32_t>(std::distance(m_per_object_data.begin(), result));
                    }
                }

                //QMatrix4x4 const & view_matrix = m_per_frame_scoped_cameras_view_matrix[static_cast<size_t>(scope)][camera_index];

                instance_SSBO_offset++;
                // This contains the base offset which the instanceID is added to to get the instance_SSBO_offset
                new_object_uniforms.iMiscObjectData[0] = (m_object_buffer_dirty == true) ? static_cast<int>(object_SSBO_offset) : new_object_uniforms.iMiscObjectData[0];
                new_object_uniforms.iMiscObjectData[1] = static_cast<int>(m_camera_to_camera_SSBO[camera_index]);
                new_object_uniforms.iMiscObjectData[2] = (m_object_buffer_dirty == true) ? static_cast<int>(material_SSBO_offset) : new_object_uniforms.iMiscObjectData[2];
                new_object_uniforms.iMiscObjectData[3] = static_cast<int>(instance_SSBO_offset);

                // Cam index updates every frame regardless
                std::memcpy(per_instance_data[instance_SSBO_offset].m_obj_cam_mat_indices, new_object_uniforms.iMiscObjectData, 4 * sizeof(float));
                std::memcpy(per_instance_data[instance_SSBO_offset].m_modelMatrix, new_object_uniforms.iModelMatrix, 16 * sizeof(float));
            }
        }
    }

    // Push the vectors with both eye's data into the SSBOs for use this frame
    //m_per_instance_buffer.updateData((float*)m_per_instance_data.data(), m_per_instance_data.size() * sizeof(PerInstanceSSBOData));
    //m_per_camera_buffer.updateData((float*)m_per_camera_data.data(), m_per_camera_data.size() * sizeof(PerCameraSSBOData));

    if (m_object_buffer_dirty == true)
    {
        m_per_object_buffer.UpdateData((float*)m_per_object_data.data(), m_per_object_data.size() * sizeof(PerObjectSSBOData));
        //m_per_object_buffer.bindNextChunk((object_SSBO_offset + 1) * sizeof(PerObjectSSBOData));
        m_object_buffer_dirty = false;
    }
    else
    {
        m_per_object_buffer.Bind();
    }

    if (instance_SSBO_offset != -1)
    {
        m_per_instance_buffer.BindNextChunk((instance_SSBO_offset + 1) * sizeof(PerInstanceSSBOData));
    }

    if (m_per_camera_data.size() != 0)
    {
        m_per_camera_buffer.UpdateData((float*)m_per_camera_data.data(), m_per_camera_data.size() * sizeof(PerCameraSSBOData));
    }

    if (m_material_buffer_dirty == true)
    {
        m_per_material_buffer.UpdateData((float*)m_per_material_data.data(), m_per_material_data.size() * sizeof(PerMaterialSSBOData));
        m_material_buffer_dirty = false;
    }
    else
    {
        m_per_material_buffer.Bind();
    }

    if (instance_SSBO_offset != -1)
    {
        // This compute shader updates the matrix data using the new per_camera data
        GLuint programID = m_main_thread_renderer->GetProgramHandleID(m_main_thread_renderer->m_per_instance_compute_shader.get());
        MathUtil::glFuncs->glUseProgram(programID);
        // m_max_compute_work_groups is guaranteed to be at least 1024
        // so we can do 1024 x the invocation count of the shader, which for this 1D shader is 32, so a min of 32768
        // per compute dispatch. Any extra dispatches would require binding a new section of the per_instance SSBO
        int const work_group_count = std::min(m_max_compute_work_groups, (instance_SSBO_offset / 32) + 1);
        MathUtil::glFuncs->glDispatchCompute(work_group_count, 1, 1); // Offset contains the last written offset, which for a size of 1 is 0, so we add 1 here.
        MathUtil::glFuncs->glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // To make compute shader writes visible to the draw commands
    }
}

void RendererGL44_RenderThread::PrintFPS()
{
    if (m_main_thread_renderer != nullptr && m_main_thread_renderer->m_GPUTimeQueryResults.size() != 0)
    {
        uint64_t sum_frame_time = 0;
        for (uint64_t frame_time_gpu : m_main_thread_renderer->m_GPUTimeQueryResults)
        {
            sum_frame_time += static_cast<uint64_t>(frame_time_gpu);
        }

        uint64_t sum_cpu_time = 0;
        for (uint64_t frame_time_cpu : m_main_thread_renderer->m_CPUTimeQueryResults)
        {
            sum_cpu_time += static_cast<uint64_t>(frame_time_cpu);
        }

        sum_frame_time /= m_main_thread_renderer->m_GPUTimeQueryResults.size();
        sum_cpu_time /= m_main_thread_renderer->m_CPUTimeQueryResults.size();

//        qDebug() << "GPUFPS: " << 1000000000.0 / static_cast<double>(sum_frame_time) << "CPUFPS: " << 1000000000.0 / static_cast<double>(sum_cpu_time);
    }
}

void RendererGL44_RenderThread::InitializeGLContext(QOpenGLContext * p_gl_context)
{
//    qDebug("RendererGL44_RenderThread::InitializeGLContext");

    m_thread = QThread::currentThread();    

    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(DecoupledRender()));
    m_timer.start(0);

    QObject::connect(&m_fps_timer, SIGNAL(timeout()), this, SLOT(PrintFPS()));
    m_fps_timer.start(500);

    m_gl_context = p_gl_context;

    m_gl_surface = new QOffscreenSurface();
    auto format = m_gl_context->format();
    m_gl_surface->setFormat(format);
    m_gl_surface->create();

    m_gl_context->makeCurrent(m_gl_surface);

    m_gl_funcs = m_gl_context->versionFunctions<QOpenGLFunctions_4_4_Core>();

    // Create FBO to use for attaching main-thread FBO textures to for blitting
    m_main_thread_fbo = 0;
    MathUtil::glFuncs->glGenFramebuffers(1, &m_main_thread_fbo);

    AbstractRenderer::InitializeState();

#ifdef WIN32
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#endif
    QThread::currentThread()->setPriority(QThread::TimeCriticalPriority);

    m_gl_context->makeCurrent(m_gl_surface);

#if defined(QT_DEBUG) && !defined(__APPLE__)
    if (m_gl_context->hasExtension(QByteArrayLiteral("GL_ARB_debug_output")))
    {
        PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB = NULL;
        glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glDebugMessageCallbackARB"));

        if (glDebugMessageCallbackARB != NULL)
        {
            qDebug() << "DEBUG OUTPUT SUPPORTED";

            glDebugMessageCallbackARB((GLDEBUGPROCARB)&MathUtil::DebugCallback, NULL);
            m_gl_funcs->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        }
        else
        {
            qDebug() << "DEBUG OUTPUT NOT SUPPORTED!";
        }
    }
#endif

    m_name = QString("OpenGL 4.4");
    //m_perObjectBuffer.Initialize((void*)m_new_per_object_data.data(), m_new_per_object_data.size() * sizeof(AssetShader_Object_Compact), GL_SHADER_STORAGE_BUFFER);
    //m_perMaterialBuffer.Initialize((void*)m_new_per_material_data.data(), m_new_per_material_data.size() * sizeof(AssetShader_Material), GL_SHADER_STORAGE_BUFFER);

    MathUtil::glFuncs->glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &m_max_compute_work_groups);

    m_per_object_data.resize(32768);
    m_per_material_data.resize(32768);
    m_per_instance_data.resize(32768);
    m_per_camera_data.resize(1024);

    m_per_object_buffer.Initialize((void*)m_per_object_data.data(), m_per_object_data.size() * sizeof(PerObjectSSBOData), GL_SHADER_STORAGE_BUFFER);
    m_per_object_data.clear();
    m_per_material_buffer.Initialize((void*)m_per_material_data.data(), m_per_material_data.size() * sizeof(PerMaterialSSBOData), GL_SHADER_STORAGE_BUFFER);
    m_per_material_data.clear();
    m_per_instance_buffer.Initialize((void*)m_per_instance_data.data(), m_per_instance_data.size() * sizeof(PerInstanceSSBOData), GL_SHADER_STORAGE_BUFFER);
    m_per_instance_data.clear();
    m_per_camera_buffer.Initialize((void*)m_per_camera_data.data(), m_per_camera_data.size() * sizeof(PerCameraSSBOData), GL_SHADER_STORAGE_BUFFER);
    m_per_camera_data.clear();

    m_meshes_pending_deletion.reserve(1024);

/*
    // Intialise HBAO+ Library
    GFSDK_SSAO_CustomHeap CustomHeap;
    CustomHeap.new_ = ::operator new;
    CustomHeap.delete_ = ::operator delete;

    GFSDK_SSAO_Status status;
    GFSDK_SSAO_GLFunctions GL;
    GL.glActiveTexture = (PFNGLACTIVETEXTUREPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glActiveTexture"));
    GL.glAttachShader = (PFNGLATTACHSHADERPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glAttachShader"));
    GL.glBindBuffer = (PFNGLBINDBUFFERPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glBindBuffer"));
    GL.glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glBindBufferBase"));
    GL.glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glBindFramebuffer"));
    GL.glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glBindFragDataLocation"));
    GL.glBindTexture = (PFNGLBINDTEXTUREEXTPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glBindTexture"));
    GL.glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glBindVertexArray"));
    GL.glBlendColor = (PFNGLBLENDCOLORPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glBlendColor"));
    GL.glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glBlendEquationSeparate"));
    GL.glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glBlendFuncSeparate"));
    GL.glBufferData = (PFNGLBUFFERDATAPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glBufferData"));
    GL.glBufferSubData = (PFNGLBUFFERSUBDATAPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glBufferSubData"));
    GL.glColorMaski = (PFNGLCOLORMASKIPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glColorMaski"));
    GL.glCompileShader = (PFNGLCOMPILESHADERPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glCompileShader"));
    GL.glCreateShader = (PFNGLCREATESHADERPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glCreateShader"));
    GL.glCreateProgram = (PFNGLCREATEPROGRAMPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glCreateProgram"));
    GL.glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glDeleteBuffers"));
    GL.glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glDeleteFramebuffers"));
    GL.glDeleteProgram = (PFNGLDELETEPROGRAMPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glDeleteProgram"));
    GL.glDeleteShader = (PFNGLDELETESHADERPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glDeleteShader"));
    GL.glDeleteTextures = (PFNGLDELETETEXTURESEXTPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glDeleteTextures"));
    GL.glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glDeleteVertexArrays"));
    typedef void (APIENTRYP PFNGLDISABLEPROC) (GLenum);
    GL.glDisable = (PFNGLDISABLEPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glDisable"));
    GL.glDrawBuffers = (PFNGLDRAWBUFFERSPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glDrawBuffers"));
    typedef void (APIENTRYP PFNGLENABLEPROC) (GLenum);
    GL.glEnable = (PFNGLENABLEPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glEnable"));
    GL.glDrawArrays = (PFNGLDRAWARRAYSEXTPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glDrawArrays"));
    GL.glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glFramebufferTexture"));
    GL.glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glFramebufferTexture2D"));
    GL.glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glFramebufferTextureLayer"));
    GL.glGenBuffers = (PFNGLGENBUFFERSPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGenBuffers"));
    GL.glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGenFramebuffers"));
    GL.glGenTextures = (PFNGLGENTEXTURESEXTPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGenTextures"));
    GL.glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGenVertexArrays"));
    typedef GLenum (APIENTRYP PFNGLGETERRORPROC) ();
    GL.glGetError = (PFNGLGETERRORPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetError"));
    GL.glGetBooleani_v = (PFNGLGETBOOLEANI_VPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetBooleani_v"));
    typedef void (APIENTRYP PFNGLGETFLOAT_VPROC) (GLenum name, GLfloat* params);
    GL.glGetFloatv = (PFNGLGETFLOAT_VPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetFloatv"));
    typedef void (APIENTRYP PFNGLGETINTEGERVPROC) (GLenum name, GLint* params);
    GL.glGetIntegerv = (PFNGLGETINTEGERVPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetIntegerv"));
    GL.glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetIntegeri_v"));
    typedef void (APIENTRYP PFNGLPROGRAMIVPROC) (GLuint program, GLenum name, GLint* params);
    GL.glGetProgramiv = (PFNGLPROGRAMIVPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetProgramiv"));
    GL.glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetProgramInfoLog"));
    typedef void (APIENTRYP PFNGLSHADERIVPROC) (GLuint program, GLenum name, GLint* params);
    GL.glGetShaderiv = (PFNGLSHADERIVPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetShaderiv"));
    GL.glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetShaderInfoLog"));
    typedef const GLubyte* (APIENTRYP PFNGLGETSTRINGPROC) (GLenum name);
    GL.glGetString = (PFNGLGETSTRINGPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetString"));
    GL.glGetUniformBlockIndex =(PFNGLGETUNIFORMBLOCKINDEXPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetUniformBlockIndex"));
    GL.glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glGetUniformLocation"));
    typedef void (APIENTRYP PFNGLGETTEXLEVELPARAMETERIVPROC) (GLenum target, GLint level, GLenum pname, GLint *params);
    GL.glGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC)glGetTexLevelParameteriv;
    typedef GLboolean (APIENTRYP PFNGLISENABLEDPROC) (GLenum cap);
    GL.glIsEnabled = (PFNGLISENABLEDPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glIsEnabled"));
    GL.glIsEnabledi = (PFNGLISENABLEDIPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glIsEnabledi"));
    GL.glLinkProgram = (PFNGLLINKPROGRAMPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glLinkProgram"));
    GL.glPolygonOffset = (PFNGLPOLYGONOFFSETEXTPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glPolygonOffset"));
    GL.glShaderSource = (GFSDK_SSAO_GLFunctions::glShaderSourceGLESType)(PFNGLSHADERSOURCEARBPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glShaderSource"));
    typedef void (APIENTRYP PFNGLTEXIMAGE2DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    GL.glTexImage2D = (PFNGLTEXIMAGE2DPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glTexImage2D"));
    typedef void (APIENTRYP PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    GL.glTexImage3D = (PFNGLTEXIMAGE3DPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glTexImage3D"));
    typedef void (APIENTRYP PFNGLTEXPARAMETERIPROC) (GLenum target, GLenum pname, GLint param);
    GL.glTexParameteri = (PFNGLTEXPARAMETERIPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glTexParameteri"));
    typedef void (APIENTRYP PFNGLTEXPARAMETERFVPROC) (GLenum target, GLenum pname, const GLfloat* params);
    GL.glTexParameterfv = (PFNGLTEXPARAMETERFVPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glTexParameterfv"));
    typedef void (APIENTRYP PFNGLUNIFORMLIPROC) (GLint loc, GLint v0);
    GL.glUniform1i = (PFNGLUNIFORMLIPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glUniform1i"));
    GL.glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glUniformBlockBinding"));
    GL.glUseProgram = (PFNGLUSEPROGRAMPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glUseProgram"));
    typedef void (APIENTRYP PFNGLVIEWPORTPROC) (GLint x, GLint y, GLsizei width, GLsizei height);
    GL.glViewport = (PFNGLVIEWPORTPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glViewport"));

    GFSDK_SSAO_Version headerVersion;
    status = GFSDK_SSAO_CreateContext_GL(&pAOContext, &GL, &CustomHeap, headerVersion);
    assert(status == GFSDK_SSAO_OK); // HBAO+ requires feature level 11_0 or above

    Params.Radius = 1.f;
    Params.Bias = 0.3f;
    Params.PowerExponent = 1.f;
    Params.DepthStorage = GFSDK_SSAO_FP16_VIEW_DEPTHS;
    Params.Blur.Enable = true;
    Params.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;
    Params.Blur.Sharpness = 8.f;
*/
    m_is_initialized = true;
}

void RendererGL44_RenderThread::Process(AbstractRenderer * p_main_thread_renderer,
                                        QHash<size_t, QVector<AbstractRenderCommand> > *,
                                        QHash<StencilReferenceValue, LightContainer> *)
{
     // Queued up data to allow Process to not block the main-thread longer than necessary
    m_main_thread_renderer = p_main_thread_renderer;

    if (m_is_rendering == false && m_is_initialized == true)
    {
        m_is_rendering = true;
    }

    const bool do_VR = (m_main_thread_renderer->m_hmd_manager != nullptr && m_main_thread_renderer->m_hmd_manager->GetEnabled() == true);

    if (do_VR == false && MathUtil::m_frame_limiter_render_thread != 0)
    {
        m_frame_rate_limiter.release(1);
    }
}

/*void RendererGL44_RenderThread::FreeMeshHandles()
{
    m_mesh_deletion_guard.lock();
    size_t const mesh_count = m_meshes_pending_deletion.size();
    for (size_t i = 0; i < mesh_count; ++i)
    {
        size_t const last_index = m_meshes_pending_deletion[i]->m_last_known_index;
        QPair<MeshHandle*, GLuint>& mesh_pair = m_mesh_handle_to_GL_ID[last_index];

        // Null m_mesh_handle_to_GL_ID ptr
        mesh_pair.first = nullptr;

        // Delete GL resource
        MathUtil::glFuncs->glDeleteVertexArrays(1, &mesh_pair.second);
        mesh_pair.second = 0;

        // Deref VBOs assosiated with VAO
        m_mesh_handle_to_buffers[last_index].first = nullptr;
        m_mesh_handle_to_buffers[last_index].second.clear();
        m_mesh_handle_to_buffers[last_index].second.shrink_to_fit();

        // Free memory used by MeshHandle
        delete m_meshes_pending_deletion[i];
    }
    m_meshes_pending_deletion.clear();
    m_mesh_deletion_guard.unlock();
}*/

void RendererGL44_RenderThread::DecoupledRender()
{
    if (m_is_rendering == true && m_is_initialized == true && m_shutting_down == false)
    {
        if (m_main_thread_renderer->m_hmd_manager != nullptr
            && m_hmd_initialized == false)
        {
            m_main_thread_renderer->m_hmd_manager->InitializeGL();
            m_main_thread_renderer->m_hmd_manager->ReCentre();
            m_hmd_initialized = true;
        }

        // Prevents main-thread from reallocating vectors read by render-thread during use
        m_reallocation_guard.lock();

        // Update rendering_index if needed
        if (m_main_thread_renderer->m_current_frame_id != m_main_thread_renderer->m_submitted_frame_id)
        {
            m_main_thread_renderer->m_rendering_index = m_main_thread_renderer->m_completed_submission_index.exchange(m_main_thread_renderer->m_rendering_index);
            m_main_thread_renderer->m_current_frame_id = m_main_thread_renderer->m_submitted_frame_id;
            m_object_buffer_dirty = true;

            // Clean up mesh handles that are pending deletion, we wait until the next unique frame
            // so that we don't delete objects that are in use for the previous one
            m_main_thread_renderer->FreeMeshHandles();
            m_main_thread_renderer->FreeProgramHandles();
            m_main_thread_renderer->FreeTextureHandles();
        }

        const bool do_VR = (m_main_thread_renderer->m_hmd_manager != nullptr && m_main_thread_renderer->m_hmd_manager->GetEnabled() == true);

        bool render_limit = m_frame_rate_limiter.tryAcquire((do_VR == false && MathUtil::m_frame_limiter_render_thread != 0) ? 1 : 0);

        if (render_limit == true)
        {
            m_main_thread_renderer->StartFrame();
            auto texture_size = (m_main_thread_renderer->m_hmd_manager != nullptr)
                    ? m_main_thread_renderer->m_hmd_manager->GetTextureSize()
                    : QSize(m_main_thread_renderer->m_window_width / 2, m_main_thread_renderer->m_window_height);

            // Override texture_size if we are a screenshot frame
            texture_size = (m_main_thread_renderer->m_screenshot_requested) ? QSize(m_main_thread_renderer->m_screenshot_width / 2, m_main_thread_renderer->m_screenshot_height) : texture_size;
            auto msaa_count = m_main_thread_renderer->GetMSAACount();
            msaa_count = (m_main_thread_renderer->m_screenshot_requested) ? m_main_thread_renderer->m_screenshot_sample_count : msaa_count;

            SetIsUsingEnhancedDepthPrecision(m_main_thread_renderer->GetIsUsingEnhancedDepthPrecision());
            ConfigureFramebuffer(texture_size.width()*2, texture_size.height(), msaa_count);
            BindFBOToDraw(FBO_TEXTURE_BITFIELD::COLOR | FBO_TEXTURE_BITFIELD::DEPTH_STENCIL, true);
            UpdateFramebuffer();
            m_main_thread_renderer->WaitforFrameSyncObject();

            // This call allows the PreRender function to get recent pose data for the HMD for this frames render
            // this is important for reducing motion-to-photon latency in VR rendering and to ensure that the timers
            // from the various HMD perf overlays show correct latency values and return us accurate predicted pose values
            if (do_VR)
            {
                m_main_thread_renderer->m_hmd_manager->Update();
            }

            PreRender(&(m_main_thread_renderer->m_scoped_render_commands_cache[m_main_thread_renderer->m_rendering_index]), &(m_main_thread_renderer->m_scoped_light_containers_cache[m_main_thread_renderer->m_rendering_index]));

            if (do_VR)
            {
                m_main_thread_renderer->m_hmd_manager->BeginRendering();
                m_main_thread_renderer->m_hmd_manager->BeginRenderEye(0);
                m_main_thread_renderer->m_hmd_manager->BeginRenderEye(1);
            }

            // Depth Only pass
           /* m_update_GPU_state = true;
            m_allow_color_mask = false;
            SetColorMask(ColorMask::COLOR_WRITES_DISABLED);
            for (size_t scope = 0; scope < (size_t)RENDERER::RENDER_SCOPE::SCOPE_COUNT; ++scope)
            {
                auto current_scope  = static_cast<RENDERER::RENDER_SCOPE>(scope);
                RenderObjectsStereoViewportInstanced(m_main_thread_renderer,
                                            current_scope,
                                            m_main_thread_renderer->m_scoped_render_commands_cache[m_main_thread_renderer->m_rendering_index][scope],
                                            m_main_thread_renderer->m_scoped_light_containers_cache[m_main_thread_renderer->m_rendering_index]);
            }
            SetColorMask(ColorMask::COLOR_WRITES_ENABLED);
            m_allow_color_mask = true;
            m_update_GPU_state = false;

            // HBAO+, written into AO buffer.
            GFSDK_SSAO_InputData_GL Input;
            Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
            Input.DepthData.FullResDepthTexture.Target = GL_TEXTURE_2D_MULTISAMPLE;
            Input.DepthData.FullResDepthTexture.TextureId = GetTextureID(FBO_TEXTURE::DEPTH_STENCIL, true);
            Input.DepthData.MetersToViewSpaceUnits = 1.0f;
            QMatrix4x4 ProjMatrix = m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][static_cast<size_t>(RENDERER::RENDER_SCOPE::CURRENT_ROOM_OBJECTS_OPAQUE)][0].getProjectionMatrix();
            Input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4(ProjMatrix.constData());
            Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;

            GFSDK_SSAO_Output_GL Output;
            Output.OutputFBO = m_FBOs[0];
            Output.Blend.Mode = GFSDK_SSAO_OVERWRITE_RGB;

            BindFBOToDraw(FBO_TEXTURE_BITFIELD::COLOR | FBO_TEXTURE_BITFIELD::AO | FBO_TEXTURE_BITFIELD::DEPTH_STENCIL , true); // We don't bind depth as we are using it for the SSBO+ input
            BindFBOToDraw(FBO_TEXTURE_BITFIELD::AO, false); // We don't bind depth as we are using it for the SSBO+ input

            GFSDK_SSAO_Status status = pAOContext->PreCreateFBOs(Params, m_window_width, m_window_height);
            assert(status == GFSDK_SSAO_OK);
            status = pAOContext->RenderAO(Input, Params, Output, GFSDK_SSAO_RENDER_AO);
            assert(status == GFSDK_SSAO_OK);

            BindFBOToDraw(FBO_TEXTURE_BITFIELD::COLOR | FBO_TEXTURE_BITFIELD::DEPTH_STENCIL, true); // We don't bind depth as we are using it for the SSBO+ input

            GLuint ao_tex_id = GetTextureID(FBO_TEXTURE::AO, false);
            MathUtil::glFuncs->glActiveTexture(GL_TEXTURE14);
            m_active_texture_slot_render = 14;
            MathUtil::glFuncs->glBindTexture(GL_TEXTURE_2D, ao_tex_id);*/

            // Deferred Full-pass(this really needs to be smart about depth to be efficient)
            for (size_t scope = 0; scope < (size_t)RENDERER::RENDER_SCOPE::SCOPE_COUNT; ++scope)
            {
                auto current_scope  = static_cast<RENDERER::RENDER_SCOPE>(scope);
                RenderObjectsStereoViewportInstanced(m_main_thread_renderer,
                                            current_scope,
                                            m_main_thread_renderer->m_scoped_render_commands_cache[m_main_thread_renderer->m_rendering_index][scope],
                                            m_main_thread_renderer->m_scoped_light_containers_cache[m_main_thread_renderer->m_rendering_index]);
            }

            BlitMultisampledFramebuffer(FBO_TEXTURE_BITFIELD::COLOR);
            MathUtil::glFuncs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // TODO:: Make this function create a mip-chain for the attached fbo color texture
            // this combined with a layer for the OVRSDK allows for better filtering
            // it is also useful if we want to create a mip-chain for the depth to use for frame n+1 reprojection or object culling techniques.
            PostRender(&(m_main_thread_renderer->m_scoped_render_commands_cache[m_main_thread_renderer->m_rendering_index]), &(m_main_thread_renderer->m_scoped_light_containers_cache[m_main_thread_renderer->m_rendering_index]));

            // Bind our current FBO as read, and the main-thread textures as our draw-FBO
            // This may have issues if those same main-thread textures are bound to a FBO on the main-thread context.
            if (MathUtil::m_do_equi
                || ((m_main_thread_renderer->m_screenshot_requested == true)
                    && m_main_thread_renderer->m_screenshot_is_equi == true
                    && m_main_thread_renderer->m_screenshot_frame_index == m_main_thread_renderer->m_current_frame_id))
            {
                RenderEqui();
            }


            BindFBOToRead(FBO_TEXTURE_BITFIELD::COLOR, false);
            QVector<uint32_t> draw_buffers;
            draw_buffers.reserve(FBO_TEXTURE::COUNT);
            m_main_thread_renderer->BindFBOAndTextures(draw_buffers, GL_TEXTURE_2D, GL_DRAW_FRAMEBUFFER, m_main_thread_fbo, 0, FBO_TEXTURE_BITFIELD::COLOR);

            if ((m_main_thread_renderer->m_screenshot_requested == false)
                || (m_main_thread_renderer->m_screenshot_is_equi == false)
                || (m_main_thread_renderer->m_screenshot_frame_index != m_main_thread_renderer->m_current_frame_id))
            {
                // Bind our current FBO as read, and the main-thread textures as our draw-FBO
                // This may have issues if those same main-thread textures are bound to a FBO on the main-thread context.
                MathUtil::glFuncs->glBlitFramebuffer(0,0, m_window_width, m_window_height,
                                                     0, 0, m_main_thread_renderer->m_window_width, m_main_thread_renderer->m_window_height,
                                                     GL_COLOR_BUFFER_BIT, GL_LINEAR);
            }

            if (m_main_thread_renderer->m_screenshot_requested == true
                && (m_main_thread_renderer->m_current_frame_id >= m_main_thread_renderer->m_screenshot_frame_index))
            {
                SaveScreenshot();
            }

            if (do_VR)
            {
                // OpenVR binds textures inside of its submission step.
                // I change the active slot here to one we do not use for normal
                // rendering to avoid the call invalidating our GL state copy.
                //
                // OpenVR needs the OpenGL handle for the texture, so we update it here.
                if (m_main_thread_renderer->m_hmd_manager->m_using_openVR)
                {
                    MathUtil::glFuncs->glActiveTexture(GL_TEXTURE15);
                    m_active_texture_slot_render = 15;
                    m_main_thread_renderer->m_hmd_manager->m_color_texture_id =  GetTextureID(FBO_TEXTURE::COLOR, false);
                }

                m_main_thread_renderer->m_hmd_manager->EndRenderEye(0);
                m_main_thread_renderer->m_hmd_manager->EndRenderEye(1);
                m_main_thread_renderer->m_hmd_manager->EndRendering();
                MathUtil::glFuncs->glFlush();
            }

            m_main_thread_renderer->LockFrameSyncObject();
            m_main_thread_renderer->EndFrame();   
        }

        // Prevents main-thread from reallocating vectors read by render-thread during use
        m_reallocation_guard.unlock();
    }
}

void RendererGL44_RenderThread::SaveScreenshot()
{
    if (m_screenshot_pbo_pending == false)
    {
        GLsizei const pbo_size = m_main_thread_renderer->m_screenshot_width * m_main_thread_renderer->m_screenshot_height * sizeof(GL_UNSIGNED_BYTE) * 4; // RGBA8
        MathUtil::glFuncs->glGenBuffers(1, &m_screenshot_pbo);
        MathUtil::glFuncs->glBindBuffer(GL_PIXEL_PACK_BUFFER, m_screenshot_pbo);
        MathUtil::glFuncs->glBufferData(GL_PIXEL_PACK_BUFFER, pbo_size, 0, GL_STREAM_READ);
        MathUtil::glFuncs->glReadPixels(0, 0, m_main_thread_renderer->m_screenshot_width, m_main_thread_renderer->m_screenshot_height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        m_screenshot_pbo_pending = true;
    }
    else
    {
        MathUtil::glFuncs->glBindBuffer(GL_PIXEL_PACK_BUFFER, m_screenshot_pbo);
        unsigned char* ptr = nullptr;
        ptr = (unsigned char*)MathUtil::glFuncs->glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if (ptr != nullptr)
        {
            QString const out_filename = MathUtil::GetLastScreenshotPath();
            QImage img(ptr, m_main_thread_renderer->m_screenshot_width, m_main_thread_renderer->m_screenshot_height, QImage::Format_RGBX8888);
            img = img.mirrored();
            img.save(out_filename, "jpg", 95);
            MathUtil::glFuncs->glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }
        MathUtil::glFuncs->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        MathUtil::glFuncs->glDeleteBuffers(1, &m_screenshot_pbo);
        m_screenshot_pbo_pending = false;
        m_main_thread_renderer->m_screenshot_requested = false;
    }
}

void RendererGL44_RenderThread::RenderEqui()
{
    uint32_t const cube_cross_width = m_window_width;
    uint32_t const cube_cross_height = m_window_height;
    uint32_t const cube_face_dim = qMin(cube_cross_width / 3, cube_cross_height / 2);
    QVector<QVector4D> viewports;
    viewports.reserve(6);
    // This is a 3x2 grid layout to use all of the available framebuffer space
    viewports.push_back(QVector4D(cube_face_dim * 0.0f, cube_face_dim * 0.0f, cube_face_dim, cube_face_dim)); // X+
    viewports.push_back(QVector4D(cube_face_dim * 1.0f, cube_face_dim * 0.0f, cube_face_dim, cube_face_dim)); // X-
    viewports.push_back(QVector4D(cube_face_dim * 2.0f, cube_face_dim * 0.0f, cube_face_dim, cube_face_dim)); // Y+
    viewports.push_back(QVector4D(cube_face_dim * 0.0f, cube_face_dim * 1.0f, cube_face_dim, cube_face_dim)); // Y-
    viewports.push_back(QVector4D(cube_face_dim * 1.0f, cube_face_dim * 1.0f, cube_face_dim, cube_face_dim)); // Z+
    viewports.push_back(QVector4D(cube_face_dim * 2.0f, cube_face_dim * 1.0f, cube_face_dim, cube_face_dim)); // Z-

    // Create a new TextureHandle if our current is nullptr, this is either because it's the first
    // frame, or because the window changed size which nulls the existing TextureHandle.
    if (!m_equi_cubemap_handle || m_equi_cubemap_face_size != cube_face_dim)
    {
        m_equi_cubemap_handle = m_main_thread_renderer->CreateCubemapTextureHandle(cube_face_dim, cube_face_dim, TextureHandle::COLOR_SPACE::SRGB, GL_RGB, false, true, true, TextureHandle::ALPHA_TYPE::NONE, TextureHandle::COLOR_SPACE::SRGB);
        m_equi_cubemap_face_size = cube_face_dim;
    }

    BindFBOToRead(FBO_TEXTURE_BITFIELD::COLOR, false);
    BindFBOToDraw(FBO_TEXTURE_BITFIELD::NONE, false);
    m_main_thread_renderer->BindTextureHandle(&(m_main_thread_renderer->m_texture_handle_to_GL_ID), 13, m_equi_cubemap_handle.get(), true);
    for (uint32_t face_index = 0; face_index < 6; ++face_index)
    {

        uint32_t target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_index;
        CopyReadBufferToTextureHandle(&(m_main_thread_renderer->m_texture_handle_to_GL_ID), m_equi_cubemap_handle.get(), target, 0, 0, 0,
                                      viewports[face_index].x(), viewports[face_index].y(),
                                      cube_face_dim, cube_face_dim);
    }


    // Use forward menu camera as we are drawing a full-screen quad
    QVector<VirtualCamera> overlay_camera;
    overlay_camera.reserve(1);
    overlay_camera.push_back(VirtualCamera(QVector3D(0.0f, 0.0f, 0.0f), QQuaternion(), QVector3D(1.0f, 1.0f, 1.0f),
                                    QVector4D(0, 0, cube_cross_width, cube_cross_height),
                                    float(cube_cross_width)/float(cube_cross_height), -1.0f, 0.1f, 10.0f));
    overlay_camera[0].SetScopeMask(RENDERER::RENDER_SCOPE::ALL, false);
    overlay_camera[0].SetScopeMask(RENDERER::RENDER_SCOPE::POST_PROCESS, true);

    // Cache existing cameras then set the cameras to our one camera needed for cubemap to equi rendering
    auto camera_cache = m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index];
    for (size_t scope_enum = 0; scope_enum < static_cast<size_t>(RENDERER::RENDER_SCOPE::SCOPE_COUNT); ++scope_enum)
    {
        m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][scope_enum].clear();
        for (VirtualCamera& camera : overlay_camera)
        {
            if (camera.GetScopeMask(static_cast<RENDERER::RENDER_SCOPE>(scope_enum)) == true)
            {
                m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index][scope_enum].push_back(camera);
            }
        }
    }

    // Cache then erase any existing commands in the overlay scope
    QHash<size_t, QVector<AbstractRenderCommand>> post_process_commands;

    // Push the AbstractRenderCommand needed to convert the cubemap into an equi to the OVERLAYS scope
    AbstractRenderComandShaderData shader_data(m_main_thread_renderer->m_default_equi_shader.get(),
            AssetShader_Frame(),
            AssetShader_Room(),
            AssetShader_Object(),
            AssetShader_Material());

    shader_data.m_frame.iResolution = QVector4D(0, 0, cube_cross_width, cube_cross_height);
    QMatrix4x4 ident;
    ident.setToIdentity();
    memcpy(shader_data.m_object.iModelMatrix, ident.constData(), 16 * sizeof(float));

    post_process_commands[(size_t)RENDERER::RENDER_SCOPE::POST_PROCESS].push_back(
                AbstractRenderCommand(PrimitiveType::TRIANGLES,
                                       6,
                                       1,
                                       0,
                                       0,
                                       0,
                                       m_main_thread_renderer->m_plane_vao.get(),
                                       shader_data.m_program,
                                       shader_data.m_frame,
                                       shader_data.m_room,
                                       shader_data.m_object,
                                       shader_data.m_material,
                                       m_main_thread_renderer->GetCurrentlyBoundTextures(),
                                       FaceCullMode::DISABLED,
                                       DepthFunc::ALWAYS,
                                       DepthMask::DEPTH_WRITES_DISABLED,
                                       StencilFunc(StencilTestFuncion::ALWAYS, StencilReferenceValue(0), StencilMask(0xffffffff)),
                                       StencilOp(StencilOpAction::KEEP, StencilOpAction::KEEP, StencilOpAction::KEEP),
                                       PolyMode::FILL,
                                       ColorMask::COLOR_WRITES_ENABLED));

    // Do the second pass of rendering to convert cubemap to equi
    PreRender(&post_process_commands, &(m_main_thread_renderer->m_scoped_light_containers_cache[m_main_thread_renderer->m_rendering_index]));
    // This is just to trigger the clearing of the FBO
    //RenderObjectsNaiveDecoupled(m_main_thread_renderer, RENDERER::RENDER_SCOPE::CURRENT_ROOM_PORTAL_STENCILS, post_process_commands[(size_t)RENDERER::RENDER_SCOPE::POST_PROCESS], (m_scoped_light_containers));
    // This draws our full-screen quad with the cubemap-to-equi fragment shader
    BindFBOToRead(FBO_TEXTURE_BITFIELD::NONE, false);
    BindFBOToDraw(FBO_TEXTURE_BITFIELD::COLOR, false);
    RenderObjectsStereoViewportInstanced(m_main_thread_renderer, RENDERER::RENDER_SCOPE::POST_PROCESS, post_process_commands[(size_t)RENDERER::RENDER_SCOPE::POST_PROCESS], m_main_thread_renderer->m_scoped_light_containers_cache[m_main_thread_renderer->m_rendering_index]);

    // Restore the cameras
    m_main_thread_renderer->m_scoped_cameras_cache[m_main_thread_renderer->m_rendering_index] = camera_cache;
}

void RendererGL44_RenderThread::FinishThread()
{
    m_shutting_down = true;
    m_timer.stop();
    emit Finished();
}

void RendererGL44_RenderThread::InitializeGLObjectsMIRROR(AbstractRenderer * p_renderer)
{
    p_renderer->InitializeGLObjects2();
}

void RendererGL44_RenderThread::UpdatePerObjectSSBO(QHash<size_t, QVector<AbstractRenderCommand>> * , bool const  /* = false */)
{
    /*float base_SSBO_offset = 0.0f;
    float previous_base_SSBO_offset = FLT_MAX;
    QVector<float> misc_object_data;
    misc_object_data.resize(16);
    AbstractRenderCommand * previous_unique_command = nullptr;
    size_t previous_unique_command_index = 0;
    uint32_t camera_index = 0;

    for (const RENDERER::RENDER_SCOPE scope : m_scopes)
    {
        QVector<AbstractRenderCommand>& render_command_vector = (*p_scoped_render_commands)[(size_t)scope];

        uint32_t const camera_count = GetCamerasPerScope(scope);
        size_t const command_count = render_command_vector.size();

        previous_unique_command = nullptr;

        for (size_t command_index = 0; command_index < command_count; command_index += camera_count)
        {
            camera_index = 0;
            AbstractRenderCommand & base_render_command = render_command_vector[command_index];
            AssetShader_Material const * new_material_uniforms = base_render_command.GetMaterialUniformsPointer();

            // Update misc data, currently using this for objectID instanceID is added to this to fetch right eye matrices
            if (p_using_instancing
                && previous_unique_command != nullptr
                && previous_unique_command->IsInstancableWith(base_render_command)
               )
            {
                // We found a batchable command so use the same draw ID as the previous
                // and increment the instance count of the first member of the batch
                misc_object_data[0] = previous_base_SSBO_offset;

                for (uint32_t camera_index = 0; camera_index < camera_count; camera_index++)
                {
                    render_command_vector[previous_unique_command_index + camera_index].IncrementInstanceCount();
                }
            }
            else
            {
                // We didn't find a batchable command or we're the first command so store this command as the unique_command
                misc_object_data[0] = base_SSBO_offset;
                previous_base_SSBO_offset = base_SSBO_offset;
                previous_unique_command = &base_render_command;
                previous_unique_command_index = command_index;
            }

            auto cache = misc_object_data[0];
            // Recompute matrices for each camera
            for (const VirtualCamera& camera : m_scoped_cameras_cache[(size_t)scope])
            {
                // If this camera is active for the current scope compute the new matrices for this frame
                if (camera.getScopeMask(scope) == true)
                {
                    AbstractRenderCommand & render_command = render_command_vector[command_index + camera_index];

                    AssetShader_Object & new_object_uniforms = render_command.GetObjectUniformsReference();

                    // This contains the base offset which the instanceID is added to to get this instance's
                    // TODO: Ideally iMiscObjectData could also store the offset into the material SSBO to avoid needing to
                    // give each object it's own copy to match the object uniform offset.
                    memcpy(new_object_uniforms.iMiscObjectData, (char*)misc_object_data.constData(), 16 * sizeof(float));
                    misc_object_data[0] += 1.0;

                    m_new_per_object_data[base_SSBO_offset] = AssetShader_Object_Compact(new_object_uniforms);
                    m_new_per_material_data[base_SSBO_offset] = *new_material_uniforms;
                    ++base_SSBO_offset;
                    ++camera_index;
                }
            }
            misc_object_data[0] = cache;
        }
    }

    // Push the vectors with both eye's data into the SSBOs for use this frame
    m_perObjectBuffer.updateData((float*)m_new_per_object_data.data(), base_SSBO_offset * sizeof(AssetShader_Object_Compact));
    m_perMaterialBuffer.updateData((float*)m_new_per_material_data.data(), base_SSBO_offset * sizeof(AssetShader_Material));*/
}
#endif

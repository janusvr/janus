#include "renderergl.h"

RendererGL::RendererGL() :
      m_gl_surface(nullptr),
      m_gl_context(nullptr),
      m_gl_funcs(nullptr),
      m_main_fbo(0),
      m_is_initialized(false),
      m_hmd_initialized(false),          
      m_screenshot_pbo_pending(false),
      m_screenshot_pbo(0)
{
    qDebug() << "RendererGL::RendererGL()";
}

RendererGL::~RendererGL()
{

}

void RendererGL::Initialize()
{
    PostConstructorInitialize();    

    InitializeGLContext(QOpenGLContext::currentContext());

    /*
    The GL_VERSION and GL_SHADING_LANGUAGE_VERSION strings begin with a version number. The version number uses one of these forms:
    major_number.minor_number major_number.minor_number.release_number
    Vendor-specific information may follow the version number. Its format depends on the implementation, but a space always separates the version number and the vendor-specific information.
    All strings are null-terminated.
    */
    m_name = QString((char *)(m_gl_funcs->glGetString(GL_VERSION)));

    // Store minor/major GL version used
    const QStringList s = m_name.left(m_name.indexOf(" ")).split(".");
    if (s.size() >= 2) {
        m_major_version = s[0].toInt();
        m_minor_version = s[1].toInt();
    }

    qDebug() << "RendererGL::Initialize() - GL_VERSION" << m_name << "major" << m_major_version << "minor" << m_minor_version;

    // Object Shader
    QString default_object_vertex_shader_path("assets/shaders/vertex.txt");
    QByteArray default_object_vertex_shader_bytes = MathUtil::LoadAssetFile(default_object_vertex_shader_path);

    QString default_no_alpha_fragment_shader_path("assets/shaders/default_no_alpha.txt");
    QByteArray default_no_alpha_fragment_shader_bytes = MathUtil::LoadAssetFile(default_no_alpha_fragment_shader_path);

    m_default_object_shader = CompileAndLinkShaderProgram(default_object_vertex_shader_bytes, default_object_vertex_shader_path,
                                                          default_no_alpha_fragment_shader_bytes, default_no_alpha_fragment_shader_path);

    QString default_binary_alpha_fragment_shader_path("assets/shaders/default_binary_alpha.txt");
    QByteArray default_binary_alpha_fragment_shader_bytes = MathUtil::LoadAssetFile(default_binary_alpha_fragment_shader_path);

    m_default_object_shader_binary_alpha = CompileAndLinkShaderProgram(default_object_vertex_shader_bytes, default_object_vertex_shader_path,
                                                                     default_binary_alpha_fragment_shader_bytes, default_binary_alpha_fragment_shader_path);

    QString default_linear_alpha_fragment_shader_path("assets/shaders/default_linear_alpha.txt");
    QByteArray ddefault_linear_alpha_fragment_shader_bytes = MathUtil::LoadAssetFile(default_linear_alpha_fragment_shader_path);

    m_default_object_shader_linear_alpha = CompileAndLinkShaderProgram(default_object_vertex_shader_bytes, default_object_vertex_shader_path,
                                                                     ddefault_linear_alpha_fragment_shader_bytes, default_linear_alpha_fragment_shader_path);

    // Skybox Shader
    QString default_cubemap_vertex_shader_path("assets/shaders/cubemap_vert.txt");
    QByteArray default_cubemap_vertex_shader_bytes = MathUtil::LoadAssetFile(default_cubemap_vertex_shader_path);

    QString default_cubemap_fragment_shader_path("assets/shaders/cubemap_frag.txt");
    QByteArray default_cubemap_fragment_shader_bytes = MathUtil::LoadAssetFile(default_cubemap_fragment_shader_path);

    m_default_skybox_shader = CompileAndLinkShaderProgram(default_cubemap_vertex_shader_bytes, default_cubemap_vertex_shader_path,
                                                          default_cubemap_fragment_shader_bytes, default_cubemap_fragment_shader_path);
    // Portal Shader
    QString default_portal_vertex_shader_path("assets/shaders/vertex.txt");
    QByteArray default_portal_vertex_shader_bytes = MathUtil::LoadAssetFile(default_portal_vertex_shader_path);

    QString default_portal_fragment_shader_path("assets/shaders/portal_frag.txt");
    QByteArray default_portal_fragment_shader_bytes = MathUtil::LoadAssetFile(default_portal_fragment_shader_path);

    m_default_portal_shader = CompileAndLinkShaderProgram(default_portal_vertex_shader_bytes, default_portal_vertex_shader_path,
                                                          default_portal_fragment_shader_bytes, default_portal_fragment_shader_path);

    // Cubemap to equi shader
    QString default_equi_vertex_shader_path("assets/shaders/vertex.txt");
    QByteArray default_equi_vertex_shader_bytes = MathUtil::LoadAssetFile(default_equi_vertex_shader_path);

    QString default_equi_fragment_shader_path("assets/shaders/cubemap_to_equi_frag2.txt");
    QByteArray default_equi_fragment_shader_bytes = MathUtil::LoadAssetFile(default_equi_fragment_shader_path);

    m_default_equi_shader = CompileAndLinkShaderProgram(default_equi_vertex_shader_bytes, default_equi_vertex_shader_path,
                                                          default_equi_fragment_shader_bytes, default_equi_fragment_shader_path);
}

void RendererGL::PreRender(QHash<int, QVector<AbstractRenderCommand> > * p_scoped_render_commands, QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers)
{
    Q_UNUSED(p_scoped_light_containers)
    UpdatePerObjectData(p_scoped_render_commands);
}

void RendererGL::PostRender(QHash<int, QVector<AbstractRenderCommand> > * p_scoped_render_commands, QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers)
{
    Q_UNUSED(p_scoped_render_commands)
    Q_UNUSED(p_scoped_light_containers)
}

QPointer<ProgramHandle> RendererGL::CompileAndLinkShaderProgram(QByteArray & p_vertex_shader, QString p_vertex_shader_path, QByteArray & p_fragment_shader, QString p_fragment_shader_path)
{
//    qDebug() << "RendererGL_LoadingThread::CompileAndLinkShaderProgram" << this;
    QPointer<ProgramHandle> handle_id = nullptr;
    CompileAndLinkShaderProgram2(handle_id, p_vertex_shader, p_vertex_shader_path, p_fragment_shader, p_fragment_shader_path, m_uniform_locs);
    return handle_id;
}

void RendererGL::CompileAndLinkShaderProgram2(QPointer<ProgramHandle> & p_abstract_program, QByteArray & p_vertex_shader,
                                                            QString p_vertex_shader_path, QByteArray & p_fragment_shader, QString p_fragment_shader_path,
                                                            QVector<QVector<GLint>> & p_map)
{
//    qDebug() << "RendererGL_LoadingThread::CompileAndLinkShaderProgram2";
    GLuint program_id;
    p_abstract_program = CreateProgramHandle(program_id);

    GLuint vertex_shader_id = MathUtil::glFuncs->glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = MathUtil::glFuncs->glCreateShader(GL_FRAGMENT_SHADER);
    GLint vertex_compile_result = GL_FALSE;
    GLint fragment_compile_result = GL_FALSE;
    GLint program_link_result = GL_FALSE;

    bool shader_failed = false;
    bool vertex_empty = false;
    bool fragment_empty = false;

    if (p_vertex_shader.contains("#version 330 core") || p_vertex_shader.contains("#version 310 es")) {
        UpgradeShaderSource(p_vertex_shader, true);
        const char * shader_data = p_vertex_shader.data();
        GLint shader_data_size = p_vertex_shader.size();
        MathUtil::glFuncs->glShaderSource(vertex_shader_id, 1, &shader_data, &shader_data_size);
        MathUtil::glFuncs->glCompileShader(vertex_shader_id);
        MathUtil::glFuncs->glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &vertex_compile_result);

        if (vertex_compile_result == GL_FALSE)
        {
            shader_failed = true;
            int log_length;
            MathUtil::glFuncs->glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &log_length);
            QVector<char> vertex_shader_log((log_length > 1) ? log_length : 1);
            MathUtil::glFuncs->glGetShaderInfoLog(vertex_shader_id, log_length, NULL, &vertex_shader_log[0]);
            MathUtil::ErrorLog(QString("Compilation of vertex shader ") + p_vertex_shader_path + QString("failed:")+vertex_shader_log.data());
        }
    }
    else
    {
        vertex_empty = true;
        QString default_object_vertex_shader_path(MathUtil::GetApplicationPath() + "assets/shaders/vertex.txt");
        QFile default_object_vertex_shader_file(default_object_vertex_shader_path);
        default_object_vertex_shader_file.open(QIODevice::ReadOnly | QIODevice::Text);
        QByteArray default_object_vertex_shader_bytes = default_object_vertex_shader_file.readAll();
        default_object_vertex_shader_file.close();

        UpgradeShaderSource(default_object_vertex_shader_bytes, true);
        const char * shader_data = default_object_vertex_shader_bytes.data();
        GLint shader_data_size = default_object_vertex_shader_bytes.size();
        MathUtil::glFuncs->glShaderSource(vertex_shader_id, 1, &shader_data, &shader_data_size);
        MathUtil::glFuncs->glCompileShader(vertex_shader_id);
        MathUtil::glFuncs->glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &vertex_compile_result);

        if (vertex_compile_result == GL_FALSE)
        {
            shader_failed = true;
            int log_length;
            MathUtil::glFuncs->glGetShaderiv(vertex_shader_id, GL_INFO_LOG_LENGTH, &log_length);
            QVector<char> vertex_shader_log((log_length > 1) ? log_length : 1);
            MathUtil::glFuncs->glGetShaderInfoLog(vertex_shader_id, log_length, NULL, &vertex_shader_log[0]);
            MathUtil::ErrorLog(QString("Compilation of vertex shader ") + default_object_vertex_shader_path + QString(" failed:")+vertex_shader_log.data());
        }
    }

    if (!shader_failed && (p_fragment_shader.contains("#version 330 core") || p_fragment_shader.contains("#version 310 es")))
    {
        UpgradeShaderSource(p_fragment_shader, false);
        const char * shader_data = p_fragment_shader.data();
        GLint shader_data_size = p_fragment_shader.size();
        MathUtil::glFuncs->glShaderSource(fragment_shader_id, 1, &shader_data, &shader_data_size);
        MathUtil::glFuncs->glCompileShader(fragment_shader_id);
        MathUtil::glFuncs->glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &fragment_compile_result);

        if (fragment_compile_result == GL_FALSE)
        {
            shader_failed = true;
            int log_length;
            MathUtil::glFuncs->glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &log_length);
            QVector<char> fragment_shader_lod((log_length > 1) ? log_length : 1);
            MathUtil::glFuncs->glGetShaderInfoLog(fragment_shader_id, log_length, NULL, &fragment_shader_lod[0]);
            MathUtil::ErrorLog(QString("Compilation of fragment shader ") + p_fragment_shader_path + QString(" failed:")+fragment_shader_lod.data());
        }
    }
    else
    {
        fragment_empty = true;
        QString default_object_fragment_shader_path("assets/shaders/default_linear_alpha.txt");
        QByteArray default_object_fragment_shader_bytes = MathUtil::LoadAssetFile(default_object_fragment_shader_path);

        UpgradeShaderSource(default_object_fragment_shader_bytes, false);
        const char * shader_data = default_object_fragment_shader_bytes.data();
        GLint shader_data_size = default_object_fragment_shader_bytes.size();
        MathUtil::glFuncs->glShaderSource(fragment_shader_id, 1, &shader_data, &shader_data_size);
        MathUtil::glFuncs->glCompileShader(fragment_shader_id);
        MathUtil::glFuncs->glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &fragment_compile_result);

        if (fragment_compile_result == GL_FALSE)
        {
            shader_failed = true;
            int log_length;
            MathUtil::glFuncs->glGetShaderiv(fragment_shader_id, GL_INFO_LOG_LENGTH, &log_length);
            QVector<char> fragment_shader_lod((log_length > 1) ? log_length : 1);
            MathUtil::glFuncs->glGetShaderInfoLog(fragment_shader_id, log_length, NULL, &fragment_shader_lod[0]);
            MathUtil::ErrorLog(QString("Compilation of fragment shader ") + default_object_fragment_shader_path + QString(" failed:") + fragment_shader_lod.data());
        }
    }

//    qDebug() << "RendererGL_LoadingThread::CompileAndLinkShaderProgram2" << shader_failed << vertex_empty << fragment_empty;

    if (!shader_failed && (!vertex_empty || !fragment_empty))
    {
        MathUtil::glFuncs->glAttachShader(program_id, vertex_shader_id);
        MathUtil::glFuncs->glAttachShader(program_id, fragment_shader_id);
        MathUtil::glFuncs->glLinkProgram(program_id);
        MathUtil::glFuncs->glGetProgramiv(program_id, GL_LINK_STATUS, &program_link_result);

        if (program_link_result == GL_FALSE)
        {
            int log_length;
            MathUtil::glFuncs->glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
            QVector<char> program_log( (log_length > 1) ? log_length : 1 );
            MathUtil::glFuncs->glGetProgramInfoLog(program_id, log_length, NULL, &program_log[0]);

            shader_failed = true;
            MathUtil::ErrorLog(QString("Linking of shaders ") + p_vertex_shader_path + QString(" & ") + p_fragment_shader_path + QString(" failed:") + program_log.data());
        }

//        qDebug() << "Vertex glDetachShader:" << program_id << "," << vertex_shader_id;
        MathUtil::glFuncs->glDetachShader(program_id, vertex_shader_id);

//        qDebug() << "Vertex glDeleteShader:" << vertex_shader_id;
        MathUtil::glFuncs->glDeleteShader(vertex_shader_id);

//        qDebug() << "Fragment glDetachShader:" << program_id << "," << fragment_shader_id;
        MathUtil::glFuncs->glDetachShader(program_id, fragment_shader_id);

//        qDebug() << "Fragment glDeleteShader:" << fragment_shader_id;
        MathUtil::glFuncs->glDeleteShader(fragment_shader_id);
    }

    // If we failed just return the default object shader
    if (shader_failed || (vertex_empty && fragment_empty)) {
        p_abstract_program = m_default_object_shader;
    }
    else {
        int log_length;
        MathUtil::glFuncs->glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
        QVector<char> program_log( (log_length > 1) ? log_length : 1 );
        MathUtil::glFuncs->glGetProgramInfoLog(program_id, log_length, NULL, &program_log[0]);
//        MathUtil::ErrorLog(QString("Linking of shaders ") + p_vertex_shader_path + QString(" & ") + p_fragment_shader_path + QString(" successful:"));
//        MathUtil::ErrorLog(program_log.data());

        MathUtil::glFuncs->glUseProgram(program_id);
        CacheUniformLocations(program_id, m_uniform_locs);
        CacheUniformLocations(program_id, p_map);
    }
}

void RendererGL::CreateMeshHandleForGeomVBOData(GeomVBOData & p_VBO_data)
{
    int32_t float_type = GL_FLOAT;
    int32_t float_size = sizeof(float);

    VertexAttributeLayout layout;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].element_count = 3;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].element_type = float_type;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].stride_in_bytes = 4 * float_size;
    layout.attributes[(uint32_t)VAO_ATTRIB::POSITION].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::NORMAL].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::NORMAL].element_count = 3;
    layout.attributes[(uint32_t)VAO_ATTRIB::NORMAL].element_type = float_type;
    layout.attributes[(uint32_t)VAO_ATTRIB::NORMAL].stride_in_bytes = 4 * float_size;
    layout.attributes[(uint32_t)VAO_ATTRIB::NORMAL].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].element_count = 2;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].element_type = float_type;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].stride_in_bytes = 4 * float_size;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD0].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].buffer_id = VAO_ATTRIB::TEXCOORD0;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].element_count = 2;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].element_type = float_type;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].stride_in_bytes = 4 * float_size;
    layout.attributes[(uint32_t)VAO_ATTRIB::TEXCOORD1].offset_in_bytes = 2 * float_size;

    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].element_count = 4;
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].element_type = float_type;
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].stride_in_bytes = 4 * float_size;
    layout.attributes[(uint32_t)VAO_ATTRIB::COLOR].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].in_use = p_VBO_data.use_skelanim ? true : false;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].element_count = 4;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].element_type = GL_UNSIGNED_BYTE;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].is_normalized = false;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].is_float_attrib = false; // We want a uvec4 not a vec4
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].stride_in_bytes = 4 * sizeof(uint8_t);
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMINDICES].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS].in_use = p_VBO_data.use_skelanim ? true : false;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS].element_count = 4;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS].element_type = float_type;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS].stride_in_bytes = 4 * float_size;
    layout.attributes[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS].offset_in_bytes = 0;

    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].in_use = true;
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].element_count = 1;
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].element_type = GL_UNSIGNED_INT;
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].stride_in_bytes = 1 * sizeof(uint32_t);
    layout.attributes[(uint32_t)VAO_ATTRIB::INDICES].offset_in_bytes = 0;

    CreateMeshHandle(p_VBO_data.m_mesh_handle, layout);
    BindMeshHandle(p_VBO_data.m_mesh_handle);

    auto buffer_handles = GetBufferHandlesForMeshHandle(p_VBO_data.m_mesh_handle);

    BindBufferHandle(buffer_handles[(uint32_t)VAO_ATTRIB::INDICES]);
    MathUtil::glFuncs->glBufferData(GL_ELEMENT_ARRAY_BUFFER, p_VBO_data.m_indices.size() * sizeof(uint32_t), p_VBO_data.m_indices.data(), GL_STATIC_DRAW);

    BindBufferHandle(buffer_handles[(uint32_t)VAO_ATTRIB::POSITION]);
    MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data.m_positions.size() * float_size, p_VBO_data.m_positions.data(), GL_STATIC_DRAW);

    BindBufferHandle(buffer_handles[(uint32_t)VAO_ATTRIB::NORMAL]);
    MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data.m_normals.size() * float_size, p_VBO_data.m_normals.data(), GL_STATIC_DRAW);

    BindBufferHandle(buffer_handles[(uint32_t)VAO_ATTRIB::TEXCOORD0]);
    MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data.m_tex_coords.size() * float_size, p_VBO_data.m_tex_coords.data(), GL_STATIC_DRAW);

    BindBufferHandle(buffer_handles[(uint32_t)VAO_ATTRIB::COLOR]);
    MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data.m_colors.size() * float_size, p_VBO_data.m_colors.data(), GL_STATIC_DRAW);

    if (p_VBO_data.use_skelanim) {
        BindBufferHandle(buffer_handles[(uint32_t)VAO_ATTRIB::SKELANIMINDICES]);
        MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data.m_skel_anim_indices.size() * sizeof(uint8_t), p_VBO_data.m_skel_anim_indices.data(), GL_STATIC_DRAW);

        BindBufferHandle(buffer_handles[(uint32_t)VAO_ATTRIB::SKELANIMWEIGHTS]);
        MathUtil::glFuncs->glBufferData(GL_ARRAY_BUFFER, p_VBO_data.m_skel_anim_weights.size() * float_size, p_VBO_data.m_skel_anim_weights.data(), GL_STATIC_DRAW);
    }
}

QPointer<MeshHandle> RendererGL::CreateMeshHandle(VertexAttributeLayout p_layout)
{
    QPointer<MeshHandle> handle_id = nullptr;
    CreateMeshHandle(handle_id, p_layout);
    return handle_id;
}

void RendererGL::InitializeGLObjects()
{
    AbstractRenderer::InitializeGLObjects();
}

void RendererGL::Render(QHash<int, QVector<AbstractRenderCommand>> * p_scoped_render_commands,
                          QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers)
{
    Q_UNUSED(p_scoped_render_commands);
    Q_UNUSED(p_scoped_light_containers);

    //62.0 - draw
    DecoupledRender();
}

void RendererGL::CreateMeshHandle(QPointer<MeshHandle> & p_handle, VertexAttributeLayout p_layout)
{
    // Calling CreateMeshHandle here will cause the VAO to be created on the render-thread's GL Context, any attempt to bind a
    // VAO obtained from the MeshHandles stored in GeomVBOData on the main-thread will cause wierd behaviour
    // as those VAO ID are not guaranteed to even exist in the main-thread GL context
    uint32_t VAO_id = 0;
    MathUtil::glFuncs->glGenVertexArrays(1, &VAO_id);
    p_handle = AbstractRenderer::CreateMeshHandle(p_layout, VAO_id);
}

void RendererGL::UpgradeShaderSource(QByteArray & p_shader_source, bool p_is_vertex_shader)
{
//    p_shader_source.replace("uniform vec4 iUseSkelAnim;",  "uniform vec4 iUseFlags;");
//    p_shader_source.replace("uniform vec4 iUseLighting;",  "");
    p_shader_source.replace("#version 310 es","#version 330 core");
    p_shader_source.replace("#ifdef GL_FRAGMENT_PRECISION_HIGH\r\n      precision highp float;\r\n#else\r\n      precision mediump float;\r\n#endif\r\n","");
    p_shader_source.replace("uniform lowp","uniform");

    //qDebug() << p_shader_source;
    p_shader_source.replace("uniform vec4 iUseSkelAnim;",  "uniform vec4 iUseFlags;");
    p_shader_source.replace("uniform vec4 iUseLighting;",  "");

    p_shader_source.replace("iUseSkelAnim.x",                      "iUseFlags.x");
    p_shader_source.replace("iUseSkelAnim[0]",                     "iUseFlags.x");
    p_shader_source.replace("iUseLighting.x",                      "iUseFlags.y");
    p_shader_source.replace("iUseLighting[0]",                     "iUseFlags.y");

    if (!p_is_vertex_shader)
    {
        // Add gamma correction step as we output to a linear texture
        if (MathUtil::m_linear_framebuffer == false)
        {
            prependDataInShaderMainFunction(p_shader_source, g_gamma_correction_GLSL);
        }
    }  
}

void RendererGL::UpdatePerObjectData(QHash<int, QVector<AbstractRenderCommand>> * p_scoped_render_commands)
{
    QMatrix4x4 temp_matrix;
    QMatrix4x4 model_matrix;
    QMatrix4x4 model_view_matrix;

    QVector<float> misc_object_data;
    misc_object_data.resize(16);

    // Resize to this frame's camera vectors to fit the current frame's cameras for each scope
    for (const RENDERER::RENDER_SCOPE scope : m_scopes)
    {
        int const camera_count_this_scope = m_scoped_cameras_cache[m_rendering_index][static_cast<int>(scope)].size();
        m_per_frame_scoped_cameras_view_matrix[static_cast<int>(scope)].resize(camera_count_this_scope);
        m_per_frame_scoped_cameras_is_left_eye[static_cast<int>(scope)].resize(camera_count_this_scope);
    }

    // Generate the view matrices and is_left_eye data for this frame's cameras
    if (m_hmd_manager != nullptr && m_hmd_manager->GetEnabled() == true)
    {
        QMatrix4x4 const eye_view_matrix_L = m_hmd_manager->GetEyeViewMatrix(0);
        QMatrix4x4 const eye_view_matrix_R = m_hmd_manager->GetEyeViewMatrix(1);

        for (const RENDERER::RENDER_SCOPE scope : m_scopes)
        {
            const int camera_count_this_scope = m_scoped_cameras_cache[m_rendering_index][static_cast<int>(scope)].size();
            for (int camera_index = 0; camera_index < camera_count_this_scope; ++camera_index)
            {
                QMatrix4x4 composited_view_matrix = m_scoped_cameras_cache[m_rendering_index][static_cast<int>(scope)][camera_index].GetViewMatrix();
                m_per_frame_scoped_cameras_is_left_eye[static_cast<int>(scope)][camera_index] = m_scoped_cameras_cache[m_rendering_index][static_cast<int>(scope)][camera_index].GetLeftEye();
                composited_view_matrix = ((m_per_frame_scoped_cameras_is_left_eye[static_cast<int>(scope)][camera_index]) ? eye_view_matrix_L : eye_view_matrix_R) * composited_view_matrix;
                m_per_frame_scoped_cameras_view_matrix[static_cast<int>(scope)][camera_index] = composited_view_matrix;

                // Update camera viewports, this takes into account things like dynamic resolution scaling
                m_scoped_cameras_cache[m_rendering_index][static_cast<int>(scope)][camera_index].SetViewport(
                            (m_per_frame_scoped_cameras_is_left_eye[static_cast<int>(scope)][camera_index] == true)
                        ? m_hmd_manager->m_eye_viewports[0]
                        : m_hmd_manager->m_eye_viewports[1]);
            }
        }
    }
    else
    {
        for (const RENDERER::RENDER_SCOPE scope : m_scopes)
        {
            const int camera_count_this_scope = m_scoped_cameras_cache[m_rendering_index][static_cast<int>(scope)].size();
            for (int camera_index = 0; camera_index < camera_count_this_scope; ++camera_index)
            {
                m_per_frame_scoped_cameras_view_matrix[static_cast<int>(scope)][camera_index] = m_scoped_cameras_cache[m_rendering_index][static_cast<int>(scope)][camera_index].GetViewMatrix();
                m_per_frame_scoped_cameras_is_left_eye[static_cast<int>(scope)][camera_index] = m_scoped_cameras_cache[m_rendering_index][static_cast<int>(scope)][camera_index].GetLeftEye();
            }
        }
    }

    for (const RENDERER::RENDER_SCOPE scope : m_scopes)
    {
        QVector<AbstractRenderCommand> & render_command_vector = (*p_scoped_render_commands)[static_cast<int>(scope)];

        const int command_count(render_command_vector.size());
        const int camera_count_this_scope(m_per_frame_scoped_cameras_view_matrix[static_cast<int>(scope)].size());

        // For each command
        for (int command_index = 0; command_index < command_count; command_index += camera_count_this_scope)
        {
            // Recompute matrices for each camera affecting each command in this scope
            for (int camera_index = 0; camera_index < camera_count_this_scope; ++camera_index)
            {
                const VirtualCamera& camera = m_scoped_cameras_cache[m_rendering_index][static_cast<int>(scope)][camera_index];

                AbstractRenderCommand & render_command = render_command_vector[command_index + camera_index];
                AssetShader_Object & new_object_uniforms = render_command.GetObjectUniformsReference();

                memcpy((char*)model_matrix.constData(), new_object_uniforms.iModelMatrix, 16 * sizeof(float));
                model_matrix.optimize(); //56.0 - call optimize so internal type is not identity and inverse does nothing

                const QMatrix4x4 & view_matrix = m_per_frame_scoped_cameras_view_matrix[static_cast<int>(scope)][camera_index];
                memcpy(new_object_uniforms.iViewMatrix, view_matrix.constData(), 16 * sizeof(float));

                memcpy(new_object_uniforms.iProjectionMatrix, camera.GetProjectionMatrix().constData(), 16 * sizeof(float));

                temp_matrix = view_matrix.inverted();
                memcpy(new_object_uniforms.iInverseViewMatrix, temp_matrix.constData(), 16 * sizeof(float));

                model_view_matrix = view_matrix * model_matrix;
                memcpy(new_object_uniforms.iModelViewMatrix, model_view_matrix.constData(), 16 * sizeof(float));

                temp_matrix = camera.GetProjectionMatrix() * model_view_matrix;
                memcpy(new_object_uniforms.iModelViewProjectionMatrix, temp_matrix.constData(), 16 * sizeof(float));

                temp_matrix = model_matrix.inverted().transposed();
                memcpy(new_object_uniforms.iTransposeInverseModelMatrix, temp_matrix.constData(), 16 * sizeof(float));

                temp_matrix = model_view_matrix.inverted().transposed();
                memcpy(new_object_uniforms.iTransposeInverseModelViewMatrix, temp_matrix.constData(), 16 * sizeof(float));
            }
        }
    }
}

void RendererGL::InitializeGLContext(QOpenGLContext * p_gl_context)
{
    qDebug("RendererGL::InitializeGLContext");
    m_gl_context = p_gl_context;

    m_gl_surface = new QOffscreenSurface();
    auto format = m_gl_context->format();
    m_gl_surface->setFormat(format);
    m_gl_surface->create();

    m_gl_context->makeCurrent(m_gl_surface);
    m_gl_funcs = m_gl_context->extraFunctions();

    // Create FBO to use for attaching main-thread FBO textures to for blitting
    m_main_fbo = 0;
    MathUtil::glFuncs->glGenFramebuffers(1, &m_main_fbo);

    m_gl_context->makeCurrent(m_gl_surface);

    if (m_gl_context->hasExtension(QByteArrayLiteral("GL_ARB_debug_output")))
    {
        PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB = NULL;
        glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)m_gl_context->getProcAddress(QByteArrayLiteral("glDebugMessageCallbackARB"));

        if (glDebugMessageCallbackARB != NULL)
        {
            qDebug() << "RendererGL::InitializeGLContext - DEBUG OUTPUT SUPPORTED";

            glDebugMessageCallbackARB((GLDEBUGPROCARB)&MathUtil::DebugCallback, NULL);
            m_gl_funcs->glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        }
        else
        {
            qDebug() << "RendererGL::InitializeGLContext - DEBUG OUTPUT NOT SUPPORTED!";
        }
    }

    m_is_initialized = true;
}

void RendererGL::DecoupledRender()
{    
    if (m_is_initialized)
    {        
        if (m_hmd_manager != nullptr
                && m_hmd_manager->GetEnabled() == true
                && m_hmd_initialized == false)
        {
            m_hmd_manager->InitializeGL();
            m_hmd_manager->ReCentre();
            m_hmd_initialized = true;
        }

        // Update rendering_index if needed
        if (m_current_frame_id != m_submitted_frame_id)
        {
            m_rendering_index = m_completed_submission_index.exchange(m_rendering_index);
            m_current_frame_id = m_submitted_frame_id;           
        }

        const bool do_VR = (m_hmd_manager != nullptr && m_hmd_manager->GetEnabled() == true);
//        qDebug() << "RendererGL::DecoupledRender()" << m_is_initialized << m_shutting_down;

        StartFrame();
        auto texture_size = (m_hmd_manager != nullptr && m_hmd_manager->GetEnabled() == true)
                ? m_hmd_manager->GetTextureSize()
                : QSize(m_window_width / 2, m_window_height);

        // Override texture_size if we are a screenshot frame
        texture_size = (m_screenshot_requested) ? QSize(m_screenshot_width / 2, m_screenshot_height) : texture_size;
        auto msaa_count = GetMSAACount();
        msaa_count = (m_screenshot_requested) ? m_screenshot_sample_count : msaa_count;

        SetIsUsingEnhancedDepthPrecision(GetIsUsingEnhancedDepthPrecision());
        ConfigureFramebuffer(texture_size.width()*2, texture_size.height(), msaa_count);
        BindFBOToDraw(FBO_TEXTURE_BITFIELD::COLOR | FBO_TEXTURE_BITFIELD::DEPTH_STENCIL, true);
        UpdateFramebuffer();
        WaitforFrameSyncObject();

        // This call allows the PreRender function to get recent pose data for the HMD for this frames render
        // this is important for reducing motion-to-photon latency in VR rendering and to ensure that the timers
        // from the various HMD perf overlays show correct latency values and return us accurate predicted pose values
        if (do_VR)
        {
            m_hmd_manager->Update();
        }

        PreRender(&(m_scoped_render_commands_cache[m_rendering_index]),
                  &(m_scoped_light_containers_cache[m_rendering_index]));

        if (do_VR)
        {
            m_hmd_manager->BeginRendering();
            m_hmd_manager->BeginRenderEye(0);
            m_hmd_manager->BeginRenderEye(1);
        }

        for (int scope = 0; scope < int(RENDERER::RENDER_SCOPE::SCOPE_COUNT); ++scope)
        {
            auto current_scope  = static_cast<RENDERER::RENDER_SCOPE>(scope);
            RenderObjects(current_scope,
                                        m_scoped_render_commands_cache[m_rendering_index][scope],
                                        m_scoped_light_containers_cache[m_rendering_index]);
        }

        BlitMultisampledFramebuffer(FBO_TEXTURE_BITFIELD::COLOR);
        MathUtil::glFuncs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // TODO:: Make this function create a mip-chain for the attached fbo color texture
        // this combined with a layer for the OVRSDK allows for better filtering
        // it is also useful if we want to create a mip-chain for the depth to use for frame n+1 reprojection or object culling techniques.
        PostRender(&(m_scoped_render_commands_cache[m_rendering_index]), &(m_scoped_light_containers_cache[m_rendering_index]));

        if (MathUtil::m_do_equi
                || ((m_screenshot_requested == true)
                    && m_screenshot_is_equi == true
                    && m_screenshot_frame_index == m_current_frame_id))
        {
            RenderEqui();
        }

        BindFBOToRead(FBO_TEXTURE_BITFIELD::COLOR, false);
        QVector<uint32_t> draw_buffers;
        draw_buffers.reserve(FBO_TEXTURE::COUNT);
        BindFBOAndTextures(draw_buffers, GL_TEXTURE_2D, GL_DRAW_FRAMEBUFFER, m_main_fbo, 0, FBO_TEXTURE_BITFIELD::COLOR);

        // We don't copy the equi screenshot to the main-thread FBO as we wan't to be an
        // offscreen render that doesn't cause a 1 frame flicker
        if ((m_screenshot_requested == false)
                || (m_screenshot_is_equi == false)
                || (m_screenshot_frame_index != m_current_frame_id))
        {
            // Bind our current FBO as read, and the main-thread textures as our draw-FBO
            // This may have issues if those same main-thread textures are bound to a FBO on the main-thread context.
            MathUtil::glFuncs->glBlitFramebuffer(0, 0, m_window_width, m_window_height,
                                                 0, 0, m_window_width, m_window_height,
                                                 GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }

        if (m_screenshot_requested == true
                && (m_current_frame_id >= m_screenshot_frame_index))
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
            if (m_hmd_manager->m_using_openVR)
            {
                MathUtil::glFuncs->glActiveTexture(GL_TEXTURE15);
                m_active_texture_slot_render = 15;
                m_hmd_manager->m_color_texture_id =  GetTextureID(FBO_TEXTURE::COLOR, false);
            }

            m_hmd_manager->EndRenderEye(0);
            m_hmd_manager->EndRenderEye(1);
            m_hmd_manager->EndRendering();
            MathUtil::glFuncs->glFlush();
        }

        LockFrameSyncObject();
        EndFrame();
    }
}

void RendererGL::SaveScreenshot()
{
    if (m_screenshot_pbo_pending == false)
    {
        GLsizei const pbo_size = m_screenshot_width * m_screenshot_height * sizeof(GL_UNSIGNED_BYTE) * 4; // RGBA8
        MathUtil::glFuncs->glGenBuffers(1, &m_screenshot_pbo);
        MathUtil::glFuncs->glBindBuffer(GL_PIXEL_PACK_BUFFER, m_screenshot_pbo);
        MathUtil::glFuncs->glBufferData(GL_PIXEL_PACK_BUFFER, pbo_size, 0, GL_STREAM_READ);
        MathUtil::glFuncs->glReadPixels(0, 0, m_screenshot_width, m_screenshot_height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        m_screenshot_pbo_pending = true;
    }
    else
    {
        GLsizei const pbo_size = m_screenshot_width * m_screenshot_height * sizeof(GL_UNSIGNED_BYTE) * 4; // RGBA8
        MathUtil::glFuncs->glBindBuffer(GL_PIXEL_PACK_BUFFER, m_screenshot_pbo);
        unsigned char* ptr = nullptr;
        ptr = (unsigned char*)MathUtil::glFuncs->glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, pbo_size, GL_MAP_READ_BIT);
        if (ptr != nullptr)
        {
            QString const out_filename = MathUtil::GetLastScreenshotPath();
            QImage img(ptr, m_screenshot_width, m_screenshot_height, QImage::Format_RGBX8888);
            img = img.mirrored();
            img.save(out_filename, "jpg", 95);
            //qDebug() << "GLWidget::SaveScreenShot() - image" << out_filename << "saved";
            MathUtil::glFuncs->glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        }
        MathUtil::glFuncs->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        MathUtil::glFuncs->glDeleteBuffers(1, &m_screenshot_pbo);
        m_screenshot_pbo_pending = false;
        m_screenshot_requested = false;
    }
}

void RendererGL::RenderEqui()
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
        m_equi_cubemap_handle = CreateCubemapTextureHandle(cube_face_dim, cube_face_dim, TextureHandle::COLOR_SPACE::SRGB, GL_RGB, false, true, true, TextureHandle::ALPHA_TYPE::NONE, TextureHandle::COLOR_SPACE::SRGB);
        m_equi_cubemap_face_size = cube_face_dim;
    }

    BindFBOToRead(FBO_TEXTURE_BITFIELD::COLOR, false);
    BindFBOToDraw(FBO_TEXTURE_BITFIELD::NONE, false);
    BindTextureHandle(m_texture_handle_to_GL_ID, 13, m_equi_cubemap_handle, true);
    for (uint32_t face_index = 0; face_index < 6; ++face_index)
    {

        uint32_t target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_index;
        CopyReadBufferToTextureHandle(m_texture_handle_to_GL_ID, m_equi_cubemap_handle, target, 0, 0, 0,
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
    auto camera_cache = m_scoped_cameras_cache[m_rendering_index];
    for (int scope_enum = 0; scope_enum < static_cast<int>(RENDERER::RENDER_SCOPE::SCOPE_COUNT); ++scope_enum)
    {
        m_scoped_cameras_cache[m_rendering_index][scope_enum].clear();
        for (VirtualCamera& camera : overlay_camera)
        {
            if (camera.GetScopeMask(static_cast<RENDERER::RENDER_SCOPE>(scope_enum)) == true)
            {
                m_scoped_cameras_cache[m_rendering_index][scope_enum].push_back(camera);
            }
        }
    }

    // Cache then erase any existing commands in the overlay scope
    QHash<int, QVector<AbstractRenderCommand>> post_process_commands;

    // Push the AbstractRenderCommand needed to convert the cubemap into an equi to the OVERLAYS scope
    AbstractRenderComandShaderData shader_data(m_default_equi_shader,
            AssetShader_Frame(),
            AssetShader_Room(),
            AssetShader_Object(),
            AssetShader_Material());

    shader_data.m_frame.iResolution = QVector4D(0, 0, cube_cross_width, cube_cross_height);
    QMatrix4x4 ident;
    ident.setToIdentity();
    memcpy(shader_data.m_object.iModelMatrix, ident.constData(), 16 * sizeof(float));

    post_process_commands[(int)RENDERER::RENDER_SCOPE::POST_PROCESS].push_back(
                AbstractRenderCommand(PrimitiveType::TRIANGLES,
                                       6,
                                       0,
                                       0,
                                       0,
                                       m_plane_vao,
                                       shader_data.m_program,
                                       shader_data.m_frame,
                                       shader_data.m_room,
                                       shader_data.m_object,
                                       shader_data.m_material,
                                       GetCurrentlyBoundTextures(),
                                       FaceCullMode::DISABLED,
                                       DepthFunc::ALWAYS,
                                       DepthMask::DEPTH_WRITES_DISABLED,
                                       StencilFunc(StencilTestFuncion::ALWAYS, StencilReferenceValue(0), StencilMask(0xffffffff)),
                                       StencilOp(StencilOpAction::KEEP, StencilOpAction::KEEP, StencilOpAction::KEEP),                                       
                                       ColorMask::COLOR_WRITES_ENABLED));

    // Do the second pass of rendering to convert cubemap to equi
    PreRender(&post_process_commands, &(m_scoped_light_containers_cache[m_rendering_index]));
    // This is just to trigger the clearing of the FBO
    //RenderObjectsNaiveDecoupled(m_main_thread_renderer, RENDERER::RENDER_SCOPE::CURRENT_ROOM_PORTAL_STENCILS, post_process_commands[(int)RENDERER::RENDER_SCOPE::POST_PROCESS], (m_scoped_light_containers));
    // This draws our full-screen quad with the cubemap-to-equi fragment shader
    BindFBOToRead(FBO_TEXTURE_BITFIELD::NONE, false);
    BindFBOToDraw(FBO_TEXTURE_BITFIELD::COLOR, false);
    RenderObjects(RENDERER::RENDER_SCOPE::POST_PROCESS, post_process_commands[(int)RENDERER::RENDER_SCOPE::POST_PROCESS], m_scoped_light_containers_cache[m_rendering_index]);

    // Restore the cameras
    m_scoped_cameras_cache[m_rendering_index] = camera_cache;
}

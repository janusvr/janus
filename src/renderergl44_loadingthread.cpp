#include "renderergl44_loadingthread.h"

#if !defined(__APPLE__)
RendererGL44_LoadingThread::RendererGL44_LoadingThread()
{

}

RendererGL44_LoadingThread::~RendererGL44_LoadingThread()
{
    emit EndChildThread();
    m_loading_thread->quit();
    while (m_loading_thread->isFinished() == false)
    {
       QThread::sleep(0.1);
    }
    // end decoupled-thread here so that it doesn't try to render
    // with an invalid main-thread parter renderer.
}

void RendererGL44_LoadingThread::InitializeRenderingThread()
{
//    qDebug() << "RendererGL44_LoadingThread::InitializeRenderingThread()";
    m_loading_thread = new QThread();
    m_loading_thread->setObjectName("RendererGL44_LoadingThreadRenderThread");
    m_loading_worker = new RendererGL44_RenderThread();
    m_loading_worker->moveToThread(m_loading_thread);

    // These signals are asynconous
    QObject::connect(m_loading_worker, SIGNAL(finished()),
                     m_loading_thread, SLOT(quit()));
    QObject::connect(m_loading_worker, SIGNAL(finished()),
                     m_loading_worker, SLOT(deleteLater()));

    // This will make the worker emit it's finished signal, which
    // will queue the worker for deletion on the child-thread's next event loop cycle.
    // This will also make the thread quit() which will emit it's finished event
    // triggering it's own deletion on the main-thread's next event loop cycle.
    // This is a blocking event
    QObject::connect(this, SIGNAL(EndChildThread()),
                     m_loading_worker, SLOT(FinishThread()),
                     Qt::ConnectionType::BlockingQueuedConnection);

    // These signals are syncronous will block until all SLOT callbacks are complete
    QObject::connect(this,             SIGNAL(InitializeGLContext(QOpenGLContext*)),
                     m_loading_worker, SLOT(InitializeGLContext(QOpenGLContext*)),
                     Qt::ConnectionType::BlockingQueuedConnection);

    QObject::connect(this,             SIGNAL(CreateMeshHandleForGeomVBODataMIRRORCOPY(AbstractRenderer *, GeomVBOData *)),
                     m_loading_worker, SLOT(CreateMeshHandleForGeomVBODataMIRRORCOPY(AbstractRenderer *, GeomVBOData *)),
                     Qt::ConnectionType::BlockingQueuedConnection);

    QObject::connect(this,             SIGNAL(CreateMeshHandle(AbstractRenderer *, std::shared_ptr<MeshHandle> *,
                                                               VertexAttributeLayout)),
                     m_loading_worker, SLOT(CreateMeshHandle(AbstractRenderer *, std::shared_ptr<MeshHandle> *,
                                                             VertexAttributeLayout)),
                     Qt::ConnectionType::BlockingQueuedConnection);

    QObject::connect(this,             SIGNAL(InitializeGLObjectsMIRROR(AbstractRenderer *)),
                     m_loading_worker, SLOT(InitializeGLObjectsMIRROR(AbstractRenderer *)),
                     Qt::ConnectionType::BlockingQueuedConnection);

    QObject::connect(this,             SIGNAL(Process(AbstractRenderer *, QHash<size_t, QVector<AbstractRenderCommand>> *, QHash<StencilReferenceValue, LightContainer> *)),
                     m_loading_worker, SLOT(Process(AbstractRenderer *, QHash<size_t, QVector<AbstractRenderCommand>> *, QHash<StencilReferenceValue, LightContainer> *)),
                     Qt::ConnectionType::BlockingQueuedConnection);


    QOpenGLContext * current_context = QOpenGLContext::currentContext();
    auto current_surface = current_context->surface();
    current_context->doneCurrent();

    auto main_format = current_context->format();
    QOpenGLContext * new_context = new QOpenGLContext();
    new_context->setFormat(main_format);
    new_context->setShareContext(current_context);
    new_context->create();
    new_context->doneCurrent();
    current_context->setShareContext(new_context);
    new_context->moveToThread(m_loading_thread);
    current_context->makeCurrent(current_surface);

    m_loading_thread->start();

    emit InitializeGLContext(new_context);
}

void RendererGL44_LoadingThread::Initialize()
{
//    qDebug() << "RendererGL44_LoadingThread::Initialize()";
    m_name = QString("OpenGL 4.4");

    InitializeRenderingThread();

    // Object Shader
    QString default_object_vertex_shader_path("assets/shaders/vertex.txt");
    QByteArray default_object_vertex_shader_bytes = MathUtil::LoadAssetFile(default_object_vertex_shader_path);
    QString default_no_alpha_fragment_shader_path("assets/shaders/default_no_alpha.txt");
    QByteArray default_no_alpha_fragment_shader_bytes = MathUtil::LoadAssetFile(default_no_alpha_fragment_shader_path);

    m_default_object_shader = CompileAndLinkShaderProgram(&default_object_vertex_shader_bytes, default_object_vertex_shader_path,
                                                          &default_no_alpha_fragment_shader_bytes, default_no_alpha_fragment_shader_path);

    QString default_binary_alpha_fragment_shader_path("assets/shaders/default_binary_alpha.txt");
    QByteArray default_binary_alpha_fragment_shader_bytes = MathUtil::LoadAssetFile(default_binary_alpha_fragment_shader_path);

    m_default_object_shader_binary_alpha = CompileAndLinkShaderProgram(&default_object_vertex_shader_bytes, default_object_vertex_shader_path,
                                                                     &default_binary_alpha_fragment_shader_bytes, default_binary_alpha_fragment_shader_path);

    QString default_linear_alpha_fragment_shader_path(MathUtil::GetApplicationPath() + "assets/shaders/default_linear_alpha.txt");
    QByteArray default_linear_alpha_fragment_shader_bytes = MathUtil::LoadAssetFile(default_linear_alpha_fragment_shader_path);
    m_default_object_shader_linear_alpha = CompileAndLinkShaderProgram(&default_object_vertex_shader_bytes, default_object_vertex_shader_path,
                                                                     &default_linear_alpha_fragment_shader_bytes, default_linear_alpha_fragment_shader_path);

    // Skybox Shader
    QString default_cubemap_vertex_shader_path("assets/shaders/cubemap_vert.txt");
    QByteArray default_cubemap_vertex_shader_bytes = MathUtil::LoadAssetFile(default_cubemap_vertex_shader_path);

    QString default_cubemap_fragment_shader_path("assets/shaders/cubemap_frag.txt");
    QByteArray default_cubemap_fragment_shader_bytes = MathUtil::LoadAssetFile(default_cubemap_fragment_shader_path);

    m_default_skybox_shader = CompileAndLinkShaderProgram(&default_cubemap_vertex_shader_bytes, default_cubemap_vertex_shader_path,
                                                          &default_cubemap_fragment_shader_bytes, default_cubemap_fragment_shader_path);
    // Portal Shader
    QString default_portal_vertex_shader_path("assets/shaders/vertex.txt");
    QByteArray default_portal_vertex_shader_bytes = MathUtil::LoadAssetFile(default_portal_vertex_shader_path);

    QString default_portal_fragment_shader_path(MathUtil::GetApplicationPath() + "assets/shaders/portal_frag.txt");
    QByteArray default_portal_fragment_shader_bytes = MathUtil::LoadAssetFile(default_portal_fragment_shader_path);

    m_default_portal_shader = CompileAndLinkShaderProgram(&default_portal_vertex_shader_bytes, default_portal_vertex_shader_path,
                                                          &default_portal_fragment_shader_bytes, default_portal_fragment_shader_path);

    // Cubemap to equi shader
    QString default_equi_vertex_shader_path("assets/shaders/vertex.txt");
    QByteArray default_equi_vertex_shader_bytes = MathUtil::LoadAssetFile(default_equi_vertex_shader_path);

    QString default_equi_fragment_shader_path("assets/shaders/cubemap_to_equi_frag2.txt");
    QByteArray default_equi_fragment_shader_bytes = MathUtil::LoadAssetFile(default_equi_vertex_shader_path);

    m_default_equi_shader = CompileAndLinkShaderProgram(&default_equi_vertex_shader_bytes, default_equi_vertex_shader_path,
                                                          &default_equi_fragment_shader_bytes, default_equi_fragment_shader_path);

    // PerInstance Compute Shader
    QString compute_shader_path("assets/shaders/Compute-PerInstanceData.txt");
    QByteArray compute_shader_bytes = MathUtil::LoadAssetFile(compute_shader_path);

    m_per_instance_compute_shader = CompileAndLinkShaderProgram(&compute_shader_bytes, compute_shader_path);
}

void RendererGL44_LoadingThread::PreRender(QHash<size_t, QVector<AbstractRenderCommand> > * p_scoped_render_commands, QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers)
{
    Q_UNUSED(p_scoped_render_commands)
    Q_UNUSED(p_scoped_light_containers)
}

void RendererGL44_LoadingThread::PostRender(QHash<size_t, QVector<AbstractRenderCommand> > * p_scoped_render_commands, QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers)
{
    Q_UNUSED(p_scoped_render_commands)
    Q_UNUSED(p_scoped_light_containers)
}

void RendererGL44_LoadingThread::UpgradeShaderSource(QByteArray & p_shader_source, bool p_is_vertex_shader)
{
#ifndef __ANDROID__
    p_shader_source.replace("#version 310 es","#version 330 core");
    p_shader_source.replace("#ifdef GL_FRAGMENT_PRECISION_HIGH\r\n      precision highp float;\r\n#else\r\n      precision mediump float;\r\n#endif\r\n","");
    p_shader_source.replace("uniform lowp","uniform");

    //qDebug() << p_shader_source;
#endif

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

std::shared_ptr<ProgramHandle> RendererGL44_LoadingThread::CompileAndLinkShaderProgram(QByteArray * p_compute_shader, QString p_compute_shader_path)
{
    std::shared_ptr<ProgramHandle> handle_id = nullptr;

    GLuint program_id;
    handle_id = CreateProgramHandle(&program_id);

    GLuint compute_shader_id = MathUtil::glFuncs->glCreateShader(GL_COMPUTE_SHADER);
    GLint compute_compile_result = GL_FALSE;

    GLchar * shader_data = p_compute_shader->data();
    GLint shader_data_size = p_compute_shader->size();
    MathUtil::glFuncs->glShaderSource(compute_shader_id, 1, &shader_data, &shader_data_size);
    MathUtil::glFuncs->glCompileShader(compute_shader_id);
    MathUtil::glFuncs->glGetShaderiv(compute_shader_id, GL_COMPILE_STATUS, &compute_compile_result);

    if (compute_compile_result == GL_FALSE)
    {
        int log_length;
        MathUtil::glFuncs->glGetShaderiv(compute_shader_id, GL_INFO_LOG_LENGTH, &log_length);
        QVector<char> vertex_shader_log((log_length > 1) ? log_length : 1);
        MathUtil::glFuncs->glGetShaderInfoLog(compute_shader_id, log_length, NULL, &vertex_shader_log[0]);
        MathUtil::ErrorLog(QString("Compilation of compute shader ") + p_compute_shader_path + QString("failed:")+vertex_shader_log.data());
        handle_id = nullptr;
    }
    else
    {
        MathUtil::glFuncs->glAttachShader(program_id, compute_shader_id);
        MathUtil::glFuncs->glLinkProgram(program_id);
    }

    MathUtil::glFuncs->glDetachShader(program_id, compute_shader_id);
    MathUtil::glFuncs->glDeleteShader(compute_shader_id);

    return handle_id;
}

std::shared_ptr<ProgramHandle> RendererGL44_LoadingThread::CompileAndLinkShaderProgram(QByteArray * p_vertex_shader, QString p_vertex_shader_path, QByteArray * p_fragment_shader, QString p_fragment_shader_path)
{
    std::shared_ptr<ProgramHandle> handle_id = nullptr;
    CompileAndLinkShaderProgram2(&handle_id, p_vertex_shader, p_vertex_shader_path, p_fragment_shader, p_fragment_shader_path, &m_uniform_locs);
    return handle_id;
}

void RendererGL44_LoadingThread::CompileAndLinkShaderProgram2(std::shared_ptr<ProgramHandle> * p_abstract_program, QByteArray * p_vertex_shader,
                                                            QString p_vertex_shader_path, QByteArray * p_fragment_shader, QString p_fragment_shader_path,
                                                            QVector<QVector<GLint>> *p_map)
{
    GLuint program_id;
    *p_abstract_program = CreateProgramHandle(&program_id);

    GLuint vertex_shader_id = MathUtil::glFuncs->glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_id = MathUtil::glFuncs->glCreateShader(GL_FRAGMENT_SHADER);
    GLint vertex_compile_result = GL_FALSE;
    GLint fragment_compile_result = GL_FALSE;
    GLint program_link_result = GL_FALSE;

    bool shader_failed = false;
    bool vertex_empty = false;
    bool fragment_empty = false;
    if (p_vertex_shader->contains("#version 330 core") || p_vertex_shader->contains("#version 310 es"))
    {
        UpgradeShaderSource(*p_vertex_shader, true);
        GLchar * shader_data = p_vertex_shader->data();
        GLint shader_data_size = p_vertex_shader->size();
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
        QString default_object_vertex_shader_path("assets/shaders/vertex.txt");
        QByteArray default_object_vertex_shader_bytes = MathUtil::LoadAssetFile(default_object_vertex_shader_path);

        UpgradeShaderSource(default_object_vertex_shader_bytes, true);
        GLchar * shader_data = default_object_vertex_shader_bytes.data();
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

    if (!shader_failed && (p_fragment_shader->contains("#version 330 core") || p_fragment_shader->contains("#version 310 es")))
    {
        UpgradeShaderSource(*p_fragment_shader, false);
        GLchar * shader_data = p_fragment_shader->data();
        GLint shader_data_size = p_fragment_shader->size();
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
        QString default_object_fragment_shader_path("assets/shaders/default_no_alpha.txt");
        QByteArray default_object_fragment_shader_bytes = MathUtil::LoadAssetFile(default_object_fragment_shader_path);

        UpgradeShaderSource(default_object_fragment_shader_bytes, false);
        GLchar * shader_data = default_object_fragment_shader_bytes.data();
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
            MathUtil::ErrorLog(QString("Compilation of fragment shader ") + default_object_fragment_shader_path + QString(" failed:")+fragment_shader_lod.data());
        }
    }

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
            MathUtil::ErrorLog(QString("Linking of shaders ") + p_vertex_shader_path + QString(" & ") + p_fragment_shader_path + QString(" failed:")+program_log.data());
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
    if (shader_failed || (vertex_empty && fragment_empty))
    {
        (*p_abstract_program) = m_default_object_shader;
    }
    else
    {
        int log_length;
        MathUtil::glFuncs->glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);
        QVector<char> program_log( (log_length > 1) ? log_length : 1 );
        MathUtil::glFuncs->glGetProgramInfoLog(program_id, log_length, NULL, &program_log[0]);;
//        MathUtil::ErrorLog(QString("Linking of shaders ") + p_vertex_shader_path + QString(" & ") + p_fragment_shader_path + QString(" successful:"));
//        MathUtil::ErrorLog(program_log.data());

        MathUtil::glFuncs->glUseProgram(program_id);
        CacheUniformLocations(program_id, &m_uniform_locs);
        CacheUniformLocations(program_id, p_map);
    }
}

void RendererGL44_LoadingThread::CreateMeshHandleForGeomVBOData(GeomVBOData * p_VBO_data)
{
    emit CreateMeshHandleForGeomVBODataMIRRORCOPY((AbstractRenderer*)this, p_VBO_data);
}

std::shared_ptr<MeshHandle> RendererGL44_LoadingThread::CreateMeshHandle(VertexAttributeLayout p_layout)
{
    std::shared_ptr<MeshHandle> handle_id = nullptr;
    emit CreateMeshHandle((AbstractRenderer*)this, &handle_id, p_layout);
    return handle_id;
}

/*void RendererGL44_LoadingThread::RemoveMeshHandleFromMap(MeshHandle* p_handle)
{
    // We keep handles in the vector but clear their contents and add the address to a free list so that it can be reused by a later creation call;
    auto const mesh_count = m_mesh_handle_to_GL_ID.size();
    for (auto itr = 0; itr < mesh_count; ++itr)
    {
        if (m_mesh_handle_to_GL_ID[itr].first == p_handle && m_mesh_handle_to_GL_ID[itr].first->m_UUID.m_UUID == p_handle->m_UUID.m_UUID)
        {
            QPair<MeshHandle*, GLuint>& mesh_pair = m_mesh_handle_to_GL_ID[itr];
            mesh_pair.first->m_last_known_index = itr;

            m_loading_worker->m_mesh_deletion_guard.lock();
            m_loading_worker->m_meshes_pending_deletion.emplace_back(mesh_pair.first);
            m_loading_worker->m_mesh_deletion_guard.unlock();
            return;
        }
    }
}*/

/*void RendererGL44_LoadingThread::RemoveProgramHandleFromMap(ProgramHandle* p_handle)
{
    // We keep handles in the vector but clear their contents and add the address to a free list so that it can be reused by a later creation call;
    auto const program_count = m_program_handle_to_GL_ID.size();
    for (auto itr = 0; itr < program_count; ++itr)
    {
        if (m_program_handle_to_GL_ID[itr].first == p_handle && m_program_handle_to_GL_ID[itr].first->m_UUID.m_UUID == p_handle->m_UUID.m_UUID)
        {
            QPair<ProgramHandle*, GLuint>& program_pair = m_program_handle_to_GL_ID[itr];
            program_pair.first->m_last_known_index = itr;

            m_loading_worker->m_program_deletion_guard.lock();
            m_loading_worker->m_programs_pending_deletion.emplace_back(program_pair.first);
            m_loading_worker->m_program_deletion_guard.unlock();
            return;
        }
    }
}*/

void RendererGL44_LoadingThread::InitializeGLObjects()
{
    AbstractRenderer::InitializeGLObjects();
}

void RendererGL44_LoadingThread::Render(QHash<size_t, QVector<AbstractRenderCommand>> * p_scoped_render_commands,
                          QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers)
{
    m_loading_worker->Process((AbstractRenderer*)this, p_scoped_render_commands, p_scoped_light_containers);
}
#endif

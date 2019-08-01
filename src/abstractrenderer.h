#ifndef ABSTRACTRENDERER_H
#define ABSTRACTRENDERER_H

#include <qopengl.h>
#include <qopenglext.h>
#include <QDebug>
#include <memory>
#include <QtAlgorithms>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMap>
#include <QVector>
#include <vector>
#include <atomic>
#include <unordered_map>
#include <memory>
#include <future>
#include "mathutil.h"
#include "lightmanager.h"
#include "assetimagedata.h"

#include "cmft/image.h"
#include "cmft/cubemapfilter.h"
#include "cmft/clcontext.h"
#include "cmft/print.h"

#include "rendererinterface.h"
#include "abstracthmdmanager.h"
#define gli glm
#include "gli/gli.hpp"

enum ShaderUniformEnum : uint8_t
{
    iLeftEye = 0,
	iResolution = 1,
	iGlobalTime = 2,
	iViewportCount = 3,

	iMiscRoomData = 4,
	iPlayerPosition = 5,
	iUseClipPlane = 6,
	iClipPlane = 7,
	iFogEnabled = 8,
	iFogMode = 9,
	iFogDensity = 10,
	iFogStart = 11,
	iFogEnd = 12,
	iFogCol = 13,
	iRoomMatrix = 14,

	iAmbient = 15,
	iDiffuse = 16,
	iSpecular = 17,
	iShininess = 18,
	iEmission = 19,
	iLightmapScale = 20,
	iTiling = 21,
	iUseTexture = 22,

	iMiscObjectData = 23,
	iConstColour = 24,
	iChromaKeyColour = 25,
    iUseFlags = 26,
	iModelMatrix = 27,
	iViewMatrix = 28,
	iProjectionMatrix = 29,
	iInverseViewMatrix = 30,
	iModelViewMatrix = 31,
	iModelViewProjectionMatrix = 32,
	iTransposeInverseModelMatrix = 33,
    iTransposeInverseModelViewMatrix = 34,
    iSkelAnimJoints = 35,

    iTexture0 = 36,
    iTexture1 = 37,
    iTexture2 = 38,
    iTexture3 = 39,
    iTexture4 = 40,
    iTexture5 = 41,
    iTexture6 = 42,
    iTexture7 = 43,
    iTexture8 = 44,
    iTexture9 = 45,

    iCubeTexture0 = 46,
    iCubeTexture1 = 47,
    iCubeTexture2 = 48,
    iCubeTexture3 = 49,
    count = 50,
};


class PerObjectSSBOData
{
public:
    PerObjectSSBOData()
    {

    }
    ~PerObjectSSBOData()
    {

    }

    bool operator==(PerObjectSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerObjectSSBOData)) == 0);
    }

    bool operator!=(PerObjectSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerObjectSSBOData)) != 0);
    }

    float m_constColour[4];
    float m_chromaKeyColour[4];
    float m_useFlags[4];
};

class PerMaterialSSBOData
{
public:
    PerMaterialSSBOData()
    {

    }
    ~PerMaterialSSBOData()
    {

    }

    bool operator==(PerMaterialSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerMaterialSSBOData)) == 0);
    }

    bool operator!=(PerMaterialSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerMaterialSSBOData)) != 0);
    }

    float m_ambient[4];
    float m_diffuse[4];
    float m_specular[4];
    float m_shininess[4];
    float m_emission[4];
    float m_tiling[4];
    float m_lightmapScale[4];
    float m_useTexture[16];
};

class PerInstanceSSBOData
{
public:
    PerInstanceSSBOData()
    {

    }
    ~PerInstanceSSBOData()
    {

    }

    int32_t m_obj_cam_mat_indices[4];
    float m_modelMatrix[16];
    float m_transposeInverseModelMatrix[16];
    float m_modelViewMatrix[16];
    float m_modelViewprojectionMatrix[16];
    float m_transposeInverseModelViewMatrix[16];
};

class PerCameraSSBOData
{
public:
    PerCameraSSBOData()
    {

    }
    ~PerCameraSSBOData()
    {

    }

    bool operator==(PerCameraSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerCameraSSBOData)) == 0);
    }

    bool operator!=(PerCameraSSBOData const & rhs) const
    {
        return (memcmp(this, &rhs, sizeof(PerCameraSSBOData)) != 0);
    }

    float m_viewmatrix[16];
    float m_projectionMatrix[16];
    float m_inverseViewMatrix[16];
};

class AbstractRenderer
{
public:
    AbstractRenderer();
    virtual ~AbstractRenderer();

// Interface
    virtual void Initialize() = 0;
    void InitializeHMDManager(QPointer<AbstractHMDManager> p_hmd_manager);
    virtual QString GetRendererName() const;
    virtual int GetRendererMajorVersion() const;
    virtual int GetRendererMinorVersion() const;
    virtual void Render(QHash<int, QVector<AbstractRenderCommand>> * p_scoped_render_commands,
                        QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers) = 0;
    virtual void PreRender(QHash<int, QVector<AbstractRenderCommand> > * p_scoped_render_commands, QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers) = 0;
    virtual void PostRender(QHash<int, QVector<AbstractRenderCommand> > * p_scoped_render_commands, QHash<StencilReferenceValue, LightContainer> * p_scoped_light_containers) = 0;
    virtual void UpgradeShaderSource(QByteArray& p_shader_source, bool p_is_vertex_shader) = 0;
    void RequestScreenShot(uint32_t const p_width, uint32_t const p_height, uint32_t const p_sample_count, bool const p_is_equi, uint64_t p_frame_index);
   
	// Shader Handle
    virtual QPointer<ProgramHandle> CompileAndLinkShaderProgram(QByteArray * p_vertex_shader, QString p_vertex_shader_path,
                                                                              QByteArray * p_fragment_shader, QString p_fragment_shader_path);
	
	// Texture Handle
    virtual QPointer<TextureHandle> CreateTextureHandle(TextureHandle::TEXTURE_TYPE p_texture_type,
        TextureHandle::COLOR_SPACE p_color_space,
        TextureHandle::ALPHA_TYPE p_alpha_type, uint32_t p_width, uint32_t p_height,
        GLuint p_GL_texture_ID);
    virtual void RemoveTextureHandleFromMap(QPointer<TextureHandle> p_handle);
    QVector<QPointer<BufferHandle>> GetBufferHandlesForMeshHandle(QPointer<MeshHandle> p_mesh_handle);
    int GetTextureWidth(TextureHandle* p_handle);
    int GetTextureHeight(TextureHandle* p_handle);
    void externalFormatAndTypeFromSize(GLenum* p_pixel_format, GLenum* p_pixel_type, uint const p_pixel_size);
    QPointer<TextureHandle> CreateCubemapTextureHandleFromAssetImages(QVector<QPointer<AssetImageData>>& p_skybox_image_data, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateTextureFromAssetImageData(QPointer<AssetImageData> data, bool is_left, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateTextureFromGLIData(const QByteArray & ba, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateTextureQImage(const QImage & img, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateCubemapTextureHandle(const uint32_t p_width, const uint32_t p_height, const TextureHandle::COLOR_SPACE p_color_space, const int32_t p_internal_texture_format, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateCubemapTextureHandleFromTextureHandles(QVector<QPointer<AssetImageData> > &p_skybox_image_data, QVector<QPointer<TextureHandle> > &p_skybox_image_handles, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    void GenerateEnvMapsFromCubemapTextureHandle(Cubemaps &p_cubemaps);
#ifdef WIN32
    QVector<QPointer<TextureHandle> > CreateSlugTextureHandles(uint32_t const p_curve_texture_width, uint32_t const p_curve_texture_height, void const * p_curve_texture,
                                                                          uint32_t const p_band_texture_width, uint32_t const p_band_texture_height, void const * p_band_texture);
#endif
    void UpdateTextureHandleData(TextureHandle* p_handle, uint const p_level, uint const p_x_offset, uint const p_y_offset, uint const p_width, uint const p_height, uint const p_pixel_size, void* const p_pixel_data);
    void UpdateTextureHandleData(TextureHandle* p_handle, uint const p_level, uint const p_x_offset, uint const p_y_offset, uint const p_width, uint const p_height, int const p_pixel_format, int const p_pixel_type, void* const p_pixel_data, uint32_t const p_data_size);
    void GenerateTextureHandleMipMap(QPointer <TextureHandle> p_handle);

    void CopyReadBufferToTextureHandle(QMap<QPointer<TextureHandle>, GLuint> &p_map, QPointer<TextureHandle> p_handle,
                                               uint32_t p_target,
                                               int32_t p_level,
                                               int32_t p_dst_x,
                                               int32_t p_dst_y,
                                               int32_t p_src_x,
                                               int32_t p_src_y,
                                               int32_t p_src_width,
                                               int32_t p_src_height);

	// Mesh Handle (VAO)
    virtual void CreateMeshHandleForGeomVBOData(GeomVBOData & p_VBO_data);
    virtual QPointer<MeshHandle> CreateMeshHandle(VertexAttributeLayout p_layout);
    virtual QPointer<MeshHandle> CreateMeshHandle(VertexAttributeLayout p_layout, GLuint p_GL_VAO_ID);
    virtual void RemoveMeshHandleFromMap(QPointer <MeshHandle> p_handle);

	// Buffer Handle (VBO, etc)
    QPointer<BufferHandle> CreateBufferHandle(BufferHandle::BUFFER_TYPE const p_buffer_type, BufferHandle::BUFFER_USAGE const p_buffer_usage);
    virtual void RemoveBufferHandleFromMap(QPointer<BufferHandle> p_handle);
    void ConfigureBufferHandleData(QPointer<BufferHandle> p_buffer_handle, uint32_t const p_data_size, void* const p_data, BufferHandle::BUFFER_USAGE const p_buffer_usage);
    void UpdateBufferHandleData(QPointer<BufferHandle> p_buffer_handle, uint32_t const p_offset, uint32_t const p_data_size, void* const p_data);

// Helper Functions
    void InitializeState();

    TextureSet GetCurrentlyBoundTextures();

    inline void BindTextureHandle(QMap<QPointer <TextureHandle>, GLuint> & p_map,
                                  uint32_t p_slot_index, QPointer <TextureHandle> p_texture_handle, bool p_force_rebind = false)
    {
        if (p_texture_handle.isNull() || p_texture_handle->m_UUID.m_in_use_flag == 0)
		{
            qDebug() << QString("ERROR: AbstractRenderer::BindTextureHandle p_texture_handle was nullptr");
            return;
		}

        if (m_active_texture_slot != p_slot_index)
        {
            MathUtil::glFuncs->glActiveTexture(GL_TEXTURE0 + p_slot_index);
            m_active_texture_slot = p_slot_index;
        }
		
        if (m_bound_texture_handles[p_slot_index].isNull() || m_bound_texture_handles[p_slot_index]->m_UUID.m_UUID != p_texture_handle->m_UUID.m_UUID || p_force_rebind == true)
        {            
            GLuint texture_id = 0;
            if (p_map.contains(p_texture_handle)) {
                texture_id = p_map[p_texture_handle];
            }

            auto texture_type = p_texture_handle->GetTextureType();
			GLenum texture_target = (texture_type == TextureHandle::TEXTURE_2D) ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;

            MathUtil::glFuncs->glBindTexture(texture_target, texture_id);
			m_bound_texture_handles[p_slot_index] = p_texture_handle;
        }
    }   

    virtual GLuint GetCurrentlyBoundMeshHandle() { return m_bound_VAO; }
    virtual void BindMeshHandle(QPointer <MeshHandle> p_mesh_handle)
	{
		GLuint VAO_id = 0;

        if (p_mesh_handle.isNull()) {
            qDebug() << QString("ERROR: AbstractRenderer::BindMeshHandle p_mesh_handle was nullptr");
			return;
		}

        if (m_mesh_handle_to_GL_ID.contains(p_mesh_handle)) {
            VAO_id = m_mesh_handle_to_GL_ID[p_mesh_handle];
        }        

        if (m_bound_VAO != VAO_id) {
			MathUtil::glFuncs->glBindVertexArray(VAO_id);
			m_bound_VAO = VAO_id;
		}
	}

	inline GLuint GetCurrentlyBoundBufferHandle(BufferHandle::BUFFER_TYPE p_buffer_type) 
	{
		uint16_t buffer_index = p_buffer_type - 1;
		
		return m_bound_buffers[buffer_index];
	}

    inline void BindBufferHandle(QPointer <BufferHandle> p_buffer_handle, BufferHandle::BUFFER_TYPE p_buffer_type)
	{
		GLuint VBO_id = 0;

        if (p_buffer_handle.isNull()) {
            qDebug() << QString("ERROR: AbstractRenderer::BindBufferHandle p_buffer_handle was nullptr");
			return;
		}

        if (m_buffer_handle_to_GL_ID.contains(p_buffer_handle)) {
            VBO_id = m_buffer_handle_to_GL_ID[p_buffer_handle];
        }

		GLenum buffer_type = (GLenum)BufferHandle::GetBufferTypeEnum(p_buffer_type);
		uint16_t buffer_index = p_buffer_type - 1;

        MathUtil::glFuncs->glBindBuffer(buffer_type, VBO_id);
        m_bound_buffers[buffer_index] = VBO_id;
	}

    inline void BindBufferHandle(QPointer <BufferHandle> p_buffer_handle)
	{
        if (p_buffer_handle) {
            BindBufferHandle(p_buffer_handle, (BufferHandle::BUFFER_TYPE)p_buffer_handle->m_UUID.m_buffer_type);
        }        
	}

    bool GetIsEnhancedDepthPrecisionSupported() const;
    bool GetIsUsingEnhancedDepthPrecision() const;
    void SetIsUsingEnhancedDepthPrecision(bool const p_is_using);

    void SetFaceCullMode(FaceCullMode p_face_cull_mode);
    FaceCullMode GetFaceCullMode() const;

    void SetDefaultFaceCullMode(FaceCullMode p_face_cull_mode);
    FaceCullMode GetDefaultFaceCullMode() const;

    void SetMirrorMode(bool p_mirror_mode);
    bool GetMirrorMode() const;

    void SetDepthFunc(DepthFunc p_depth_func);
    DepthFunc GetDepthFunc() const;

    void SetDepthMask(DepthMask p_depth_mask);
    DepthMask GetDepthMask() const;

    void SetStencilFunc(StencilFunc p_stencil_func);
    StencilFunc GetStencilFunc() const;

    void SetStencilOp(StencilOp p_stencil_op);
    StencilOp GetStencilOp() const;

    void SetColorMask(ColorMask p_color_mask);
    ColorMask GetColorMask() const;

    void RenderObjects(RENDERER::RENDER_SCOPE const p_scope, QVector<AbstractRenderCommand> const & p_object_render_commands, QHash<StencilReferenceValue, LightContainer> const & p_scoped_light_containers);

    void InitializeLightUBOs();
    void PushNewLightData(LightContainer const * p_lightContainer);
    virtual void InitializeGLObjects();

    virtual GLuint GetProgramHandleID(QPointer <ProgramHandle> p_handle);

    GLuint InitGLBuffer(GLsizeiptr p_dataSizeInBytes, void* p_data, GLenum p_bufferType, GLenum p_bufferUse);
    GLuint InitGLVertexAttribBuffer(GLenum p_dataType, GLboolean p_normalised, GLint p_dataTypeCount, GLsizeiptr p_dataSizeInBytes, GLuint p_attribID, GLuint p_attribDivisor, GLsizei p_stride, GLenum p_bufferUse, void* p_data);
    void CopyDataBetweenBuffers(GLuint p_src, GLuint p_dst, GLsizeiptr p_size, GLintptr p_srcOffset, GLintptr p_dstOffset);
    void CreateShadowVAO(GLuint p_VAO, QVector<GLuint> p_VBOs);
    void CreateShadowFBO(GLuint p_FBO, QVector<GLuint> p_texture_ids);

    void WaitforFrameSyncObject();
    void LockFrameSyncObject();
    void StartFrame();
    void EndFrame();
    void SetCameras(QVector<VirtualCamera> * p_cameras);
    void SetDefaultFontGlyphAtlas(QPointer<TextureHandle> p_handle);
    QPointer <TextureHandle> GetDefaultFontGlyphAtlas();
    QVector<VirtualCamera> const & GetCameras() const;
    uint32_t GetCamerasPerScope(RENDERER::RENDER_SCOPE const p_scope) const;

    QVector<uint64_t> & GetGPUTimeQueryResults();
    QVector<uint64_t> & GetCPUTimeQueryResults();
    int64_t GetFrameCounter();
    int GetNumTextures() const;
    void prependDataInShaderMainFunction(QByteArray &p_shader_source, const char *p_insertion_string);

    static char const * g_gamma_correction_GLSL;
    QPointer<ProgramHandle> CreateProgramHandle(uint32_t & p_GPU_ID);
    void RemoveProgramHandleFromMap(QPointer <ProgramHandle> p_handle);

    // FramebufferManager relocation
    bool IsFramebufferConfigurationValid() const;
    void checkFrameBufferCompleteness(uint32_t const p_target) const;
    void UpdateFramebuffer();
    void ConfigureFramebuffer(uint32_t const p_window_width, uint32_t const p_window_height, uint32_t const p_msaa_count);
    void ConfigureWindowSize(uint32_t const p_window_width, uint32_t const p_window_height);
    void ConfigureSamples(uint32_t const p_msaa_count);
    uint32_t GetTextureID(FBO_TEXTURE_ENUM const p_texture_index, bool const p_multisampled) const;
    QVector<uint32_t> BindFBOToRead(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask, bool const p_bind_multisampled = true);
    QVector<uint32_t> BindFBOToDraw(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask, bool const p_bind_multisampled = true);
    void BindFBOAndTextures(QVector<uint32_t>& p_bound_buffers, uint32_t const p_texture_type,
                            uint32_t const p_framebuffer_target, uint32_t const p_fbo,
                            int const p_texture_offset, FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask) const;
    void BlitMultisampledFramebuffer(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask,
                                     int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1,
                                     int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1);
    void BlitMultisampledFramebuffer(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask);
    uint32_t GetWindowWidth() const {return m_window_width;}
    uint32_t GetWindowHeight()const {return m_window_height;}
    uint32_t GetMSAACount() const {return m_msaa_count;}

    bool m_update_GPU_state;
    bool m_allow_color_mask;
    GLuint m_active_texture_slot;
    GLuint m_active_texture_slot_render;

    QMap<QPointer <TextureHandle>, GLuint> m_texture_handle_to_GL_ID;

    QPointer<AbstractHMDManager> m_hmd_manager;
    QVector<uint64_t> m_GPUTimeQueryResults; // GPU data from frame 0 is read on frame 2 because of readback delays to avoid stalling
    QVector<uint64_t> m_CPUTimeQueryResults;
    QElapsedTimer m_frame_time_timer;
    QVector<QMatrix4x4> m_per_frame_cameras;
    QVector<bool> m_per_frame_cameras_is_left_eye;
    QVector<RENDERER::RENDER_SCOPE> m_scopes;
    QPointer<ProgramHandle> m_default_object_shader;
    QPointer<ProgramHandle> m_default_object_shader_binary_alpha;
    QPointer<ProgramHandle> m_default_object_shader_linear_alpha;
    QPointer<ProgramHandle> m_default_skybox_shader;
    QPointer<ProgramHandle> m_default_portal_shader;
    QPointer<ProgramHandle> m_default_equi_shader;
    QPointer<ProgramHandle> m_per_instance_compute_shader;

    QPointer<ProgramHandle> GetDefaultObjectShaderProgram();
    QPointer<ProgramHandle> GetDefaultSkyboxShaderProgram();
    QPointer<ProgramHandle> GetDefaultPortalShaderProgram();

    QPointer<MeshHandle> m_default_vao;

    QPointer<BufferHandle> m_slab_vbo;
    QPointer<MeshHandle> m_slab_vao;

    QPointer<BufferHandle> m_cube_vbo;
    QPointer<MeshHandle> m_cube_vao;

    QPointer<BufferHandle> m_skycube_vbo;
    QPointer<MeshHandle> m_skycube_vao;

    QPointer<BufferHandle> m_cube3_vbo;
    QPointer<MeshHandle> m_cube3_vao;

    QPointer<BufferHandle> m_plane_vbo;
    QPointer<MeshHandle> m_plane_vao;

    QPointer<BufferHandle> m_disc_vbo;
    QPointer<MeshHandle> m_disc_vao;

    QPointer<BufferHandle> m_cone_vbo;
    QPointer<MeshHandle> m_cone_vao;

    QPointer<BufferHandle> m_cone2_vbo;
    QPointer<MeshHandle> m_cone2_vao;

    QPointer<BufferHandle> m_pyramid_vbo;
    QPointer<MeshHandle> m_pyramid_vao;

    QPointer<BufferHandle> m_portal_stencil_cube_vbo;
    QPointer<MeshHandle> m_portal_stencil_cube_vao;

    QPointer<BufferHandle> m_portal_stencil_cylinder_vbo;
    QPointer<MeshHandle> m_portal_stencil_cylinder_vao;

    GLuint m_fullScreenQuadShaderProgram;
    QPointer<MeshHandle> m_fullScreenQuadVAO;
    QVector<QPointer<BufferHandle>> m_fullScreenQuadVBOs;

    uint32_t m_window_width;
    uint32_t m_window_height;
    uint32_t m_msaa_count;
    bool m_framebuffer_requires_initialization;
    bool m_framebuffer_initialized;
    QVector<GLuint> m_FBOs;
    QVector<GLuint> m_textures;
    QVector<QPointer<TextureHandle>> m_texture_handles;
    QPointer<TextureHandle> m_default_font_glyph_atlas;
    uint8_t m_current_submission_index;
    std::atomic<uint8_t> m_completed_submission_index;
    uint8_t m_rendering_index;
    uint64_t m_submitted_frame_id;
    uint64_t m_current_frame_id;
    QVector<QHash<int, QVector<AbstractRenderCommand>>> m_scoped_render_commands_cache;
    QVector<QHash<StencilReferenceValue, LightContainer>> m_scoped_light_containers_cache;
    //QVector<QVector<VirtualCamera>> m_cameras_cache;
    QVector<QHash<int, QVector<VirtualCamera>>> m_scoped_cameras_cache;
    QHash<int, QVector<QMatrix4x4>> m_per_frame_scoped_cameras_view_matrix;
    QHash<int, QVector<bool>> m_per_frame_scoped_cameras_is_left_eye;
    uint32_t m_draw_id;

    // Screenshot info
    bool m_screenshot_requested;
    uint32_t m_screenshot_width;
    uint32_t m_screenshot_height;
    uint32_t m_screenshot_sample_count;
    bool m_screenshot_is_equi;
    uint64_t m_screenshot_frame_index;
    bool m_enhanced_depth_precision_used;
    bool m_enhanced_depth_precision_supported;

    void PostConstructorInitialize();

protected:

    void CacheUniformLocations(GLuint p_program, QVector<QVector<GLint>> & p_map);

    static inline void UpdateUniform4fv(GLint const p_loc,  GLuint const p_vector_count, float* p_values)
    {
        if (p_loc != -1)
        {
            MathUtil::glFuncs->glUniform4fv(p_loc, p_vector_count, p_values);
        }
    }

    static inline void UpdateUniform4iv(GLint const p_loc,  GLuint const p_vector_count, int32_t* p_values)
    {
        if (p_loc != -1)
        {
            MathUtil::glFuncs->glUniform4iv(p_loc, p_vector_count, p_values);
        }
    }

    static inline void UpdateUniformMatrix4fv(GLint const p_loc,  GLuint const p_matrix_count, float* p_values)
    {
        if (p_loc != -1)
        {
            MathUtil::glFuncs->glUniformMatrix4fv(p_loc, p_matrix_count, GL_FALSE, p_values);
        }
    }

    inline void UpdateFrameUniforms(GLuint const p_program_id, AssetShader_Frame const * const p_new_uniforms) const
    {
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iLeftEye)], 1, (float *)&(p_new_uniforms->iLeftEye));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iResolution)], 1, (float *)&(p_new_uniforms->iResolution));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iGlobalTime)], 1, (float *)&(p_new_uniforms->iGlobalTime));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iViewportCount)], 1, (float *)&(p_new_uniforms->iViewportCount));
    }

    inline void UpdateRoomUniforms(GLuint const p_program_id, AssetShader_Room const * const p_new_uniforms) const
    {
        UpdateUniformMatrix4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iMiscRoomData)], 1, (float *)&(p_new_uniforms->iMiscRoomData[0]));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iPlayerPosition)], 1, (float *)&(p_new_uniforms->iPlayerPosition));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iUseClipPlane)], 1, (float *)&(p_new_uniforms->iUseClipPlane));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iClipPlane)], 1, (float *)&(p_new_uniforms->iClipPlane));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iFogEnabled)], 1, (float *)&(p_new_uniforms->iFogEnabled));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iFogMode)], 1, (float *)&(p_new_uniforms->iFogMode));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iFogDensity)], 1, (float *)&(p_new_uniforms->iFogDensity));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iFogStart)], 1, (float *)&(p_new_uniforms->iFogStart));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iFogEnd)], 1, (float *)&(p_new_uniforms->iFogEnd));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iFogCol)], 1, (float *)&(p_new_uniforms->iFogCol));
        UpdateUniformMatrix4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iRoomMatrix)], 1, (float *)&(p_new_uniforms->iRoomMatrix[0]));
    }

    inline void UpdateObjectUniforms(GLuint const p_program_id, AssetShader_Object const * const p_new_uniforms) const
    {
        UpdateUniform4iv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iMiscObjectData)], 1, (int32_t *)&(p_new_uniforms->iMiscObjectData[0]));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iConstColour)], 1, (float *)&(p_new_uniforms->iConstColour[0]));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iChromaKeyColour)], 1, (float *)&(p_new_uniforms->iChromaKeyColour[0]));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iUseFlags)], 1, (float *)&(p_new_uniforms->iUseFlags[0]));
        UpdateUniformMatrix4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iModelMatrix)], 1, (float *)&(p_new_uniforms->iModelMatrix[0]));
        UpdateUniformMatrix4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iViewMatrix)], 1, (float *)&(p_new_uniforms->iViewMatrix[0]));
        UpdateUniformMatrix4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iProjectionMatrix)], 1, (float *)&(p_new_uniforms->iProjectionMatrix[0]));
        UpdateUniformMatrix4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iInverseViewMatrix)], 1, (float *)&(p_new_uniforms->iInverseViewMatrix[0]));
        UpdateUniformMatrix4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iModelViewMatrix)], 1, (float *)&(p_new_uniforms->iModelViewMatrix[0]));
        UpdateUniformMatrix4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iModelViewProjectionMatrix)], 1, (float *)&(p_new_uniforms->iModelViewProjectionMatrix[0]));
        UpdateUniformMatrix4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iTransposeInverseModelMatrix)], 1, (float *)&(p_new_uniforms->iTransposeInverseModelMatrix[0]));
        UpdateUniformMatrix4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iTransposeInverseModelViewMatrix)], 1, (float *)&(p_new_uniforms->iTransposeInverseModelViewMatrix[0]));

        if (p_new_uniforms->iUseFlags[0] == 1.0f)
        {
            UpdateUniformMatrix4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iSkelAnimJoints)], static_cast<GLuint>(ASSETSHADER_MAX_JOINTS), (float *)&(p_new_uniforms->iSkelAnimJoints[0]));
        }
    }

    inline void UpdateMaterialUniforms(GLuint const p_program_id, AssetShader_Material const * const p_new_uniforms) const
    {
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iAmbient)], 1, (float *)&(p_new_uniforms->iAmbient));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iDiffuse)], 1, (float *)&(p_new_uniforms->iDiffuse));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iSpecular)], 1, (float *)&(p_new_uniforms->iSpecular));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iShininess)], 1, (float *)&(p_new_uniforms->iShininess));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iEmission)], 1, (float *)&(p_new_uniforms->iEmission));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iLightmapScale)], 1, (float *)&(p_new_uniforms->iLightmapScale));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iTiling)], 1, (float *)&(p_new_uniforms->iTiling));
        UpdateUniform4fv(m_uniform_locs[p_program_id][(ShaderUniformEnum::iUseTexture)], 4, (float *)&(p_new_uniforms->iUseTexture[0]));
    }

    void InitScreenAlignedQuad();
    void InitScreenQuadShaderProgram();

    PFNGLCLIPCONTROLPROC m_glClipControl;
    QVector<GLsync> m_syncObjects; // These are used to prevent stopming over data in-use by the GPU
    QVector<GLuint> m_GPUTimeQuery; // Used for profiling GPU frame times
    GLuint64 m_GPUTimeMin;
    GLuint64 m_GPUTimeMax;
    GLuint64 m_GPUTimeAvg;
    int64_t m_frame_counter;
    QString m_name;
    int m_major_version;
    int m_minor_version;

    friend class Renderer;
    QVector<AssetShader_Object> m_sequntialTestData;
    QVector<QVector<GLint>> m_uniform_locs;

    QMap<TextureHandle*, int> m_texture_handle_to_width;
    QMap<TextureHandle*, int> m_texture_handle_to_height;

    QMap <MeshHandle*, GLuint> m_mesh_handle_to_GL_ID;
    QMap <MeshHandle*, QVector<QPointer<BufferHandle>>> m_mesh_handle_to_buffers;
    QMap <BufferHandle*, GLuint> m_buffer_handle_to_GL_ID;
    QMap <ProgramHandle*, GLuint> m_program_handle_to_GL_ID;
    uint32_t m_texture_UUID;
    uint32_t m_mesh_UUID;
    uint32_t m_buffer_UUID;
    uint32_t m_shader_UUID;

private:

    void GetViewportsAndCameraCount(QVector<float>& viewports, RENDERER::RENDER_SCOPE const p_scope, int &camera_count);

    void InternalFormatFromSize(GLenum *p_pixel_format, const uint p_pixel_size);

    LightContainer m_empty_light_container;
    GLfloat m_max_anisotropy;
    QVector <QPointer <TextureHandle> > m_bound_texture_handles;
    QVector <QPointer <TextureHandle> > m_bound_texture_handles_render;
	GLuint m_bound_VAO;
	GLuint m_bound_buffers[BUFFER_TYPE_COUNT];
    FaceCullMode m_face_cull_mode;
    FaceCullMode m_current_face_cull_mode;
    FaceCullMode m_default_face_cull_mode;
    bool m_mirror_mode;
    DepthFunc m_depth_func;
    DepthFunc m_current_depth_func;
    DepthMask m_depth_mask;
    DepthMask m_current_depth_mask;
    StencilFunc m_stencil_func;
    StencilFunc m_current_stencil_func;
    StencilOp m_stencil_op;
    StencilOp m_current_stencil_op;    
    ColorMask m_color_mask;
    ColorMask m_current_color_mask;
    BlendFunction m_blend_src;
    BlendFunction m_current_blend_src;
    BlendFunction m_blend_dest;
    BlendFunction m_current_blend_dest;
    GLuint m_light_UBOs[static_cast<int>(BUFFER_CHUNK_COUNT)];
    LightContainer m_dummyLights;
    GLuint m_active_light_UBO_index;
};

#endif // ABSTRACTRENDERER_H


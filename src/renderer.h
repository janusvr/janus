#ifndef RENDERER_H
#define RENDERER_H

#include <qopengl.h>
#include <qopenglext.h>
#include <QtCore>
#include <QtAlgorithms>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include <algorithm>
#include <vector>
#include <cfloat>
#include <stdint.h>
#include <cmath>

#include "half.hpp"

#include "cmft/image.h"
#include "cmft/cubemapfilter.h"
#include "cmft/clcontext.h"
#include "cmft/print.h"

#include "assetimagedata.h"
#include "mathutil.h"
#include "lightmanager.h"
#include "abstracthmdmanager.h"

#define gli glm
#include "gli/gli.hpp"

class AbstractHMDManager; // Forward Declare for interface

class Renderer : public QObject
{
    Q_OBJECT

public:
	Renderer();
    ~Renderer();

    void Initialize();    
    void InitializeHMDManager(QPointer <AbstractHMDManager> p_hmd_manager);    

    QPointer<ProgramHandle> GetDefaultObjectShaderProgram();
    QPointer<ProgramHandle> GetDefaultSkyboxShaderProgram();
    QPointer<ProgramHandle> GetDefaultPortalShaderProgram();

    void SetCameras(QVector<VirtualCamera> * p_cameras);
	void SetDefaultFontGlyphAtlas(QPointer<TextureHandle> p_handle);
    QPointer <TextureHandle> GetDefaultFontGlyphAtlas();

    QPointer<TextureHandle> CreateCubemapTextureHandleFromAssetImages(QVector<QPointer<AssetImageData>>& p_skybox_image_data, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateTextureFromAssetImageData(QPointer<AssetImageData> data, bool is_left, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateTextureFromGLIData(const QByteArray & ba, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateTextureQImage(const QImage & img, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateCubemapTextureHandle(const uint32_t p_width, const uint32_t p_height, const TextureHandle::COLOR_SPACE p_color_space, const int32_t p_internal_texture_format, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateCubemapTextureHandleFromTextureHandles(QVector<QPointer<AssetImageData> > &p_skybox_image_data, QVector<QPointer<TextureHandle> > &p_skybox_image_handles, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);

    void UpdateTextureHandleData(TextureHandle* p_handle, uint const p_level, uint const p_x_offset, uint const p_y_offset, uint const p_width, uint const p_height, uint const p_pixel_size, void* const p_pixel_data);
    void UpdateTextureHandleData(TextureHandle* p_handle, uint const p_level, uint const p_x_offset, uint const p_y_offset, uint const p_width, uint const p_height, int const p_pixel_format, int const p_pixel_type, void* const p_pixel_data, uint32_t const p_data_size);
    void GenerateTextureHandleMipMap(TextureHandle* p_handle);

    void CreateMeshHandleForGeomVBOData(GeomVBOData &p_VBO_data);
    QPointer<MeshHandle> CreateMeshHandle(VertexAttributeLayout p_layout);
    void BindMeshHandle(QPointer <MeshHandle> p_mesh_handle);
    QVector<QPointer<BufferHandle>> GetBufferHandlesForMeshHandle(QPointer <MeshHandle> p_mesh_handle);
    void RemoveMeshHandleFromMap(QPointer <MeshHandle> p_handle);
	
	QPointer<BufferHandle> CreateBufferHandle(BufferHandle::BUFFER_TYPE const p_buffer_type, BufferHandle::BUFFER_USAGE const p_buffer_usage);
    void BindBufferHandle(QPointer <BufferHandle> p_buffer_handle, BufferHandle::BUFFER_TYPE const p_buffer_type);
    void BindBufferHandle(QPointer <BufferHandle> p_buffer_handle);
	void ConfigureBufferHandleData(QPointer<BufferHandle> p_buffer_handle, uint32_t const p_data_size, void* const p_data, BufferHandle::BUFFER_USAGE const p_buffer_usage);
	void UpdateBufferHandleData(QPointer<BufferHandle> p_buffer_handle, uint32_t const p_offset, uint32_t const p_data_size, void* const p_data);
    void RemoveBufferHandleFromMap(QPointer <BufferHandle> p_handle);

    bool GetIsEnhancedDepthPrecisionSupported() const;
    bool GetIsUsingEnhancedDepthPrecision() const;
    void SetIsUsingEnhancedDepthPrecision(bool const p_is_using);

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

    TextureSet GetCurrentlyBoundTextures();

    int GetTextureWidth(TextureHandle* p_handle);
    int GetTextureHeight(TextureHandle* p_handle);

    void Render();
    void PushAbstractRenderCommand(AbstractRenderCommand& p_object_render_command);    

    void PushLightContainer(LightContainer const * p_lightContainer, StencilReferenceValue p_room_stencil_ref);

    void BeginScope(RENDERER::RENDER_SCOPE p_scope);
    void EndCurrentScope();

    QVector<uint64_t> & GetGPUTimeQueryResults();
    QVector<uint64_t> & GetCPUTimeQueryResults();

    int64_t GetFrameCounter();
    int GetNumTextures() const;

    QString GetRendererName() const;
    int GetRendererMajorVersion() const;
    int GetRendererMinorVersion() const;

    QPointer<MeshHandle> GetSkyboxCubeVAO();
    GLuint GetSkyboxCubePrimCount() const;
    QPointer<MeshHandle> GetTexturedCubeVAO();
    GLuint GetTexturedCubePrimCount() const;
    QPointer<MeshHandle> GetTexturedCube2VAO();
    GLuint GetTexturedCube2PrimCount() const;
    QPointer<MeshHandle> GetTexturedCube3VAO();
    GLuint GetTexturedCube3PrimCount() const;
    QPointer<MeshHandle> GetPortalStencilCylinderVAO();
    GLuint GetPortalStencilCylinderPrimCount() const;
    QPointer<MeshHandle> GetPortalStencilCubeVAO();
    GLuint GetPortalStencilCubePrimCount() const;
    QPointer<MeshHandle> GetPlaneVAO();
    GLuint GetPlanePrimCount() const;
    QPointer<MeshHandle> GetDiscVAO();
    GLuint GetDiscPrimCount() const;
    QPointer<MeshHandle> GetConeVAO();
    GLuint GetConePrimCount() const;
    QPointer<MeshHandle> GetCone2VAO();
    GLuint GetCone2PrimCount() const;
    QPointer<MeshHandle> GetPyramidVAO();
    GLuint GetPyramidPrimCount() const;

    void ConfigureFramebuffer(uint32_t const p_window_width, uint32_t const p_window_height, uint32_t const p_msaa_count);    
    void ConfigureSamples(uint32_t const p_msaa_count);

    QVector<uint32_t> BindFBOToRead(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask, bool const p_bind_multisampled = true) const;
    QVector<uint32_t> BindFBOToDraw(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask, bool const p_bind_multisampled = true) const;
    void BlitMultisampledFramebuffer(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask,
                                     int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1,
                                     int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1) const;
    void BlitMultisampledFramebuffer(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask) const;
    uint32_t GetWindowWidth() const;
    uint32_t GetWindowHeight() const;
    uint32_t GetMSAACount() const;
    void SubmitFrame();
    void RequestScreenShot(uint32_t const p_width, uint32_t const p_height, uint32_t const p_sample_count, bool const p_is_equi, uint64_t p_frame_index);
    uint64_t GetLastSubmittedFrameID();

    //    Interface        
    void UpgradeShaderSource(QByteArray& p_shader_source, bool p_is_vertex_shader);

    //	 Texture Handle
    QPointer<TextureHandle> CreateTextureHandle(TextureHandle::TEXTURE_TYPE p_texture_type,
                                                TextureHandle::COLOR_SPACE p_color_space,
                                                TextureHandle::ALPHA_TYPE p_alpha_type, uint32_t p_width, uint32_t p_height,
                                                GLuint p_GL_texture_ID);
    void RemoveTextureHandleFromMap(QPointer<TextureHandle> p_handle);
    void externalFormatAndTypeFromSize(GLenum* p_pixel_format, GLenum* p_pixel_type, uint const p_pixel_size);

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

    //	 Mesh Handle (VAO)
    QPointer<MeshHandle> CreateMeshHandle(VertexAttributeLayout p_layout, GLuint p_GL_VAO_ID);

    // Helper Functions
    void BindTextureHandle(uint32_t p_slot_index, QPointer <TextureHandle> p_texture_handle, bool p_force_rebind = false);

    void SetFaceCullMode(FaceCullMode p_face_cull_mode);
    FaceCullMode GetFaceCullMode() const;

    void RenderObjects(RENDERER::RENDER_SCOPE const p_scope, QVector<AbstractRenderCommand> const & p_object_render_commands, QHash<StencilReferenceValue, LightContainer> const & p_scoped_light_containers);

    void PushNewLightData(LightContainer const * p_lightContainer);
    virtual void InitializeGLObjects();

    GLuint GetProgramHandleID(QPointer <ProgramHandle> p_handle);

    GLuint InitGLBuffer(GLsizeiptr p_dataSizeInBytes, void* p_data, GLenum p_bufferType, GLenum p_bufferUse);
    GLuint InitGLVertexAttribBuffer(GLenum p_dataType, GLboolean p_normalised, GLint p_dataTypeCount, GLsizeiptr p_dataSizeInBytes, GLuint p_attribID, GLuint p_attribDivisor, GLsizei p_stride, GLenum p_bufferUse, void* p_data);
    void CopyDataBetweenBuffers(GLuint p_src, GLuint p_dst, GLsizeiptr p_size, GLintptr p_srcOffset, GLintptr p_dstOffset);       

    void prependDataInShaderMainFunction(QByteArray &p_shader_source, const char *p_insertion_string);

    static char const * g_gamma_correction_GLSL;
    QPointer<ProgramHandle> CreateProgramHandle(uint32_t & p_GPU_ID);
    void RemoveProgramHandleFromMap(QPointer <ProgramHandle> p_handle);

    // FramebufferManager relocation
    bool IsFramebufferConfigurationValid() const;
    void checkFrameBufferCompleteness(uint32_t const p_target) const;
    void UpdateFramebuffer();
    QVector<uint32_t> BindFBOToRead(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask, bool const p_bind_multisampled = true);
    QVector<uint32_t> BindFBOToDraw(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask, bool const p_bind_multisampled = true);
    void BindFBOAndTextures(QVector<uint32_t>& p_bound_buffers, uint32_t const p_texture_type,
                            uint32_t const p_framebuffer_target, uint32_t const p_fbo,
                            int const p_texture_offset, FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask) const;
    void BlitMultisampledFramebuffer(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask,
                                     int32_t srcX0, int32_t srcY0, int32_t srcX1, int32_t srcY1,
                                     int32_t dstX0, int32_t dstY0, int32_t dstX1, int32_t dstY1);
    void BlitMultisampledFramebuffer(FBO_TEXTURE_BITFIELD_ENUM const p_textures_bitmask);   

    QPointer <ProgramHandle> CompileAndLinkShaderProgram(QByteArray & p_vertex_shader, QString p_vertex_shader_path, QByteArray & p_fragment_shader, QString p_fragment_shader_path);

    void SaveScreenshot();

    void CreateMeshHandle(QPointer<MeshHandle> &p_handle, VertexAttributeLayout p_layout);    

    static QPointer <Renderer> m_pimpl;

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
    uint8_t m_completed_submission_index;
    uint8_t m_rendering_index;
    uint64_t m_submitted_frame_id;
    uint64_t m_current_frame_id;
    QVector<QHash<int, QVector<AbstractRenderCommand>>> m_scoped_render_commands_cache;
    QVector<QHash<StencilReferenceValue, LightContainer>> m_scoped_light_containers_cache;
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

protected:

    void CacheUniformLocations(GLuint p_program);

    static void UpdateUniform4fv(GLint const p_loc,  GLuint const p_vector_count, float* p_values);
    static void UpdateUniform4iv(GLint const p_loc,  GLuint const p_vector_count, int32_t* p_values);
    static void UpdateUniformMatrix4fv(GLint const p_loc,  GLuint const p_matrix_count, float* p_values);

    void UpdateFrameUniforms(GLuint const p_program_id, AssetShader_Frame const * const p_new_uniforms) const;
    void UpdateRoomUniforms(GLuint const p_program_id, AssetShader_Room const * const p_new_uniforms) const;
    void UpdateObjectUniforms(GLuint const p_program_id, AssetShader_Object const * const p_new_uniforms) const;
    void UpdateMaterialUniforms(GLuint const p_program_id, AssetShader_Material const * const p_new_uniforms) const;

    PFNGLCLIPCONTROLPROC m_glClipControl;   
    QVector<GLuint> m_GPUTimeQuery; // Used for profiling GPU frame times
    GLuint64 m_GPUTimeMin;
    GLuint64 m_GPUTimeMax;
    GLuint64 m_GPUTimeAvg;
    int64_t m_frame_counter;
    QString m_name;
    int m_major_version;
    int m_minor_version;

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

    void InitializeScopes();
    void InitializeState();
    void InitializeLightUBOs();
    void InitializeGLContext(QOpenGLContext* p_gl_context);

    void SortRenderCommandsByDistance(QVector<AbstractRenderCommand>& render_command_vector, const bool p_is_transparent);    

    void InternalFormatFromSize(GLenum *p_pixel_format, const uint p_pixel_size);
    void UpdatePerObjectData(QHash<int, QVector<AbstractRenderCommand> > &p_scoped_render_commands);
    void RenderEqui();

    RENDERER::RENDER_SCOPE m_current_scope;
    QVector<AbstractRenderCommand_sort> m_sorted_command_indices;

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

    QOffscreenSurface *  m_gl_surface;
    QOpenGLContext * m_gl_context;    

    GLuint m_main_fbo;
    QPointer<TextureHandle> m_equi_cubemap_handle;
    uint32_t m_equi_cubemap_face_size;
    bool m_is_initialized;
    bool m_hmd_initialized;

    bool m_screenshot_pbo_pending;
    GLuint m_screenshot_pbo;

};

#endif // RENDERER_H

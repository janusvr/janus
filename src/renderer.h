#ifndef RENDERER_H
#define RENDERER_H

#include <QDebug>
#include <vector>
#include <QVector>
#include <QString>
#include <QImage>
#include <algorithm>
#include "rendererinterface.h"
#include "assetimagedata.h"
#include "renderergl.h"

class Renderer : public RendererInterface
{
public:
	Renderer();
    ~Renderer();

	static Renderer * GetSingleton()
	{
		static Renderer * const singleton = new Renderer();
		return singleton;
	}

    uint32_t CreateVAO();
    uint32_t DeleteVAO();

    void Initialize();
    void InitializeScopes();
    void InitializeState();
    void InitializeLightUBOs();
    void InitializeHMDManager(QPointer <AbstractHMDManager> p_hmd_manager);
    QPointer<ProgramHandle> CompileAndLinkShaderProgram(QByteArray * p_vertex_shader, QString p_vertex_shader_path,
                                                                      QByteArray * p_fragment_shader, QString p_fragment_shader_path);

    QPointer<ProgramHandle> GetDefaultObjectShaderProgram();
    QPointer<ProgramHandle> GetDefaultSkyboxShaderProgram();
    QPointer<ProgramHandle> GetDefaultPortalShaderProgram();

    void SetCameras(QVector<VirtualCamera> * p_cameras);
	void SetDefaultFontGlyphAtlas(QPointer<TextureHandle> p_handle);
	TextureHandle* GetDefaultFontGlyphAtlas();

    QPointer<TextureHandle> CreateCubemapTextureHandleFromAssetImages(QVector<QPointer<AssetImageData>>& p_skybox_image_data, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateTextureFromAssetImageData(QPointer<AssetImageData> data, bool is_left, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateTextureFromGLIData(const QByteArray & ba, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateTextureQImage(const QImage & img, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateCubemapTextureHandle(const uint32_t p_width, const uint32_t p_height, const TextureHandle::COLOR_SPACE p_color_space, const int32_t p_internal_texture_format, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    QPointer<TextureHandle> CreateCubemapTextureHandleFromTextureHandles(QVector<QPointer<AssetImageData> > &p_skybox_image_data, QVector<QPointer<TextureHandle> > &p_skybox_image_handles, const bool tex_mipmap, const bool tex_linear, const bool tex_clamp, const TextureHandle::ALPHA_TYPE tex_alpha, const TextureHandle::COLOR_SPACE tex_colorspace);
    void GenerateEnvMapsFromCubemapTextureHandle(Cubemaps &p_cubemaps);
#ifdef WIN32
    QVector<QPointer<TextureHandle>> CreateSlugTextureHandles(uint32_t const p_curve_texture_width,
                                                                           uint32_t const p_curve_texture_height,
                                                                           void const * p_curve_texture,
                                                                           uint32_t const p_band_texture_width,
                                                                           uint32_t const p_band_texture_height,
                                                                           void const * p_band_texture);
#endif
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

    //AbstractRenderCommand CreateDefaultAbstractRenderCommand();
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

    void BindTextureHandle(uint32_t p_slot_index, TextureHandle* p_id);
    TextureSet GetCurrentlyBoundTextures();

    int GetTextureWidth(TextureHandle* p_handle);
    int GetTextureHeight(TextureHandle* p_handle);

    void Render();
    void PushAbstractRenderCommand(AbstractRenderCommand& p_object_render_command);
    void RenderObjects();	    

    void PushLightContainer(LightContainer const * p_lightContainer, StencilReferenceValue p_room_stencil_ref);

    void BeginScope(RENDERER::RENDER_SCOPE p_scope);
    void EndCurrentScope();
    RENDERER::RENDER_SCOPE GetCurrentScope();

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
    void ConfigureWindowSize(uint32_t const p_window_width, uint32_t const p_window_height);
    void ConfigureSamples(uint32_t const p_msaa_count);
    uint32_t GetTextureID(FBO_TEXTURE_ENUM const p_texture_index, bool const p_multisampled) const;
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

private:

    void PreRender(QHash<int, QVector<AbstractRenderCommand> > & p_scoped_render_commands, QHash<StencilReferenceValue, LightContainer> & p_scoped_light_containers);
    void PostRender(QHash<int, QVector<AbstractRenderCommand> > & p_scoped_render_commands, QHash<StencilReferenceValue, LightContainer> & p_scoped_light_containers);
    void SortRenderCommandsByDistance(QVector<AbstractRenderCommand>& render_command_vector, const bool p_is_transparent);    

    RENDERER::RENDER_SCOPE m_current_scope;
    std::unique_ptr<AbstractRenderer> m_abstractRenderer;    
    QVector<AbstractRenderCommand_sort> m_sorted_command_indices;
};

#endif // RENDERER_H

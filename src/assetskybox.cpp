#include "assetskybox.h"

AssetSkybox::AssetSkybox() :
	m_has_generated_texture(false)
{
    m_texture_handle = AssetImage::null_cubemap_tex_handle;
}

AssetSkybox::~AssetSkybox()
{    
}

void AssetSkybox::SetAssetImages(const QVector<QPointer <AssetImage> > & imgs)
{
    if ((m_asset_images.size() == 0)
            || (m_asset_images.size() != imgs.size())
            || (m_asset_images != imgs))
    {
        m_asset_images = imgs;
        m_has_generated_texture = false;
        m_texture_handle = AssetImage::null_cubemap_tex_handle;
    }
}

QVector<QPointer <AssetImage> > & AssetSkybox::GetAssetImages()
{
    return m_asset_images;
}

void AssetSkybox::UpdateAssets()
{
    for (QPointer <AssetImage> & a : m_asset_images) {
        if (a) {
            a->UpdateGL();
        }
    }
}

QPointer <TextureHandle> AssetSkybox::GetTextureHandle()
{    
    const int imageCount = m_asset_images.size();
    bool imagesReady = true;

	if (m_has_generated_texture == false)
    {        
        for (int imageIndex = 0; imageIndex < imageCount; ++imageIndex)
		{
            if (m_asset_images[imageIndex] == 0)
            {
                imagesReady = false;
            }
            QPointer <AssetImage> a =  m_asset_images[imageIndex];
            if ((!a || !a->GetFinished()))
            {
                imagesReady = false;
            }
		}

        if (imageCount == 1 && imagesReady && !m_has_generated_texture && m_asset_images[0])
		{
            m_texture_handle = nullptr;
            m_has_generated_texture = true;
		}

		if (imageCount > 1 && imagesReady && !m_has_generated_texture)
        {
            QVector <QPointer <AssetImageData> > asset_image_datas;
			asset_image_datas.resize(6);
            QVector <QPointer <TextureHandle> > asset_image_handles;
            asset_image_handles.resize(6);

			bool tex_linear = false;
			bool tex_mipmap = false;
			bool tex_clamp = false;
            TextureHandle::ALPHA_TYPE tex_alpha = TextureHandle::ALPHA_TYPE::UNDEFINED;
            TextureHandle::COLOR_SPACE tex_colorspace = TextureHandle::COLOR_SPACE::SRGB;
			for (int i = 0; i < imageCount; ++i)
            {
                QPointer <AssetImage> asset_image = m_asset_images[i];
                asset_image_handles[i] = asset_image.data()->GetTextureHandle(true);

                if (asset_image
                    && asset_image->GetTextureData().isNull() == false)
                {
                    asset_image_datas[i] = asset_image->GetTextureData();
                    if (i == 0)
                    {
                        tex_linear = asset_image->GetProperties()->GetTexLinear();
                        tex_mipmap = asset_image->GetProperties()->GetTexMipmap();
                        tex_clamp = asset_image->GetProperties()->GetTexClamp();
                        QString const tex_alpha_string = asset_image->GetProperties()->GetTexAlpha();

                        if (tex_alpha_string.contains("none")) {
                            tex_alpha = TextureHandle::ALPHA_TYPE::NONE;
                        }
                        else if (tex_alpha_string.contains("cutout")) {
                            tex_alpha = TextureHandle::ALPHA_TYPE::CUTOUT;
                        }
                        else if (tex_alpha_string.contains("blended")) {
                            tex_alpha = TextureHandle::ALPHA_TYPE::BLENDED;
                        }
                        else {
                            tex_alpha = TextureHandle::ALPHA_TYPE::UNDEFINED;
                        }

                        QString const tex_colorspace_string = asset_image->GetProperties()->GetTexColorspace();

                        if (tex_colorspace_string.contains("sRGB")) {
                            tex_colorspace = TextureHandle::COLOR_SPACE::SRGB;
                        }
                        else {
                            tex_colorspace = TextureHandle::COLOR_SPACE::LINEAR;
                        }
                    }
                }
			}

            m_texture_handle = Renderer::m_pimpl->CreateCubemapTextureHandleFromTextureHandles(asset_image_datas, asset_image_handles, tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);

			m_has_generated_texture = true;
		}
	}

    return (!m_texture_handle.isNull()) ? m_texture_handle : m_asset_images[0]->GetTextureHandle(true);
}

void AssetSkybox::DrawGL(QPointer <AssetShader> shader, const QMatrix4x4 & model_matrix)
{   
    TextureHandle* tex_handle = GetTextureHandle();
    // Tex_handle will be nullptr if we are using a cubemap assetImage
    // if we generated the handle ourselves from faces then we have a valid pointer here.
    Renderer * renderer = Renderer::m_pimpl;
    renderer->BindTextureHandle(10, tex_handle);

    MathUtil::PushModelMatrix();
    MathUtil::LoadModelMatrix(model_matrix);
	shader->SetUseTextureAll(false);
	shader->SetUseCubeTextureAll(false);
	shader->SetUseCubeTexture0(true);
    shader->SetUseLighting(renderer->GetIsUsingEnhancedDepthPrecision() == true
                           && renderer->GetIsEnhancedDepthPrecisionSupported() == true);
	shader->SetConstColour(QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
	shader->SetAmbient(QVector3D(1.0f, 1.0f, 1.0f));
	shader->SetDiffuse(QVector3D(1.0f, 1.0f, 1.0f));
	shader->SetEmission(QVector3D(0, 0, 0));
	shader->SetSpecular(QVector3D(0.04f, 0.04f, 0.04f));
	shader->SetShininess(50.0f);

    shader->UpdateObjectUniforms();

    AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                            renderer->GetSkyboxCubePrimCount(),
                            0,
                            0,
                            0,
                            renderer->GetSkyboxCubeVAO(),
                            shader->GetProgramHandle(),
                            shader->GetFrameUniforms(),
                            shader->GetRoomUniforms(),
                            shader->GetObjectUniforms(),
                            shader->GetMaterialUniforms(),
                            renderer->GetCurrentlyBoundTextures(),
                            renderer->GetDefaultFaceCullMode(),
                            renderer->GetDepthFunc(),
                            renderer->GetDepthMask(),
                            renderer->GetStencilFunc(),
                            renderer->GetStencilOp(),
                            renderer->GetColorMask());
    renderer->PushAbstractRenderCommand(a);

    MathUtil::PopModelMatrix();  
}

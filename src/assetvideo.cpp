#include "assetvideo.h"

AssetVideo::AssetVideo() :
    sound_enabled(true),
    volume(70),
    default_volume(70)
{
    props->SetType(TYPE_ASSETVIDEO);
    props->SetSBS3D(false);
    props->SetOU3D(false);
    props->SetReverse3D(false);
    props->SetLoop(false);
    props->SetAutoPlay(false);
}

AssetVideo::~AssetVideo()
{
}

void AssetVideo::SetupOutput(MediaContext * ctx)
{
    media_player.Set3D(props->GetSBS3D(), props->GetOU3D(), props->GetReverse3D());
    media_player.SetupOutput(ctx, props->GetSrcURL(), props->GetLoop());
}

void AssetVideo::ClearOutput(MediaContext * ctx)
{
    MediaPlayer::ClearOutput(ctx);
}

bool AssetVideo::GetPlaying(MediaContext * ctx)
{
    return media_player.GetPlaying(ctx);
}

void AssetVideo::Play(MediaContext * ctx)
{
    media_player.Play(ctx);
}

void AssetVideo::Seek(MediaContext * ctx, const float pos)
{
    media_player.Seek(ctx, pos);
}

void AssetVideo::Pause(MediaContext * ctx)
{
    media_player.Pause(ctx);
}

void AssetVideo::Stop(MediaContext * ctx)
{
    media_player.Stop(ctx);
}

bool AssetVideo::CanPause(MediaContext * ctx)
{
    return media_player.CanPause(ctx);
}

float AssetVideo::GetCurTime(MediaContext * ctx) const
{
    return media_player.GetCurTime(ctx);
}

float AssetVideo::GetTotalTime(MediaContext * ctx) const
{
    return media_player.GetTotalTime(ctx);
}

void AssetVideo::SetSoundEnabled(MediaContext * ctx, const bool b)
{
    sound_enabled = b;

    if (!sound_enabled) {
        volume = 0;
        media_player.SetVolume(ctx, 0);
    }
    else {
        volume = default_volume;
        media_player.SetVolume(ctx, volume);
    }
}

void AssetVideo::DrawGL(MediaContext * ctx, QPointer <AssetShader> shader, const bool left_eye, const float aspect_ratio)
{
    shader->SetUseTextureAll(false);
    shader->SetUseTexture(0, true, false, false);

    auto tex_id_left = GetTextureHandle(ctx, left_eye);
    auto tex_id_right = GetTextureHandle(ctx, !left_eye);

    Renderer * renderer = Renderer::m_pimpl;
    renderer->BindTextureHandle(0, tex_id_left);

    auto textureSet = renderer->GetCurrentlyBoundTextures();
    if (tex_id_left != tex_id_right) {
        textureSet.SetTextureHandle(0, tex_id_right, 2);
    }

    //qDebug() << "AssetVideo::DrawVideoGL()" << GetTextureGL(left_eye) << aspect;
    MathUtil::ModelMatrix().scale(1, aspect_ratio, 1);

    shader->UpdateObjectUniforms();

    AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                            renderer->GetTexturedCubePrimCount(),
                            0,
                            0,
                            0,
                            renderer->GetTexturedCubeVAO(),
                            shader->GetProgramHandle(),
                            shader->GetFrameUniforms(),
                            shader->GetRoomUniforms(),
                            shader->GetObjectUniforms(),
                            shader->GetMaterialUniforms(),
                            textureSet,
                            renderer->GetDefaultFaceCullMode(),
                            renderer->GetDepthFunc(),
                            renderer->GetDepthMask(),
                            renderer->GetStencilFunc(),
                            renderer->GetStencilOp(),
                            renderer->GetColorMask());
    renderer->PushAbstractRenderCommand(a);
}

void AssetVideo::DrawSelectedGL(QPointer <AssetShader> shader)
{
    shader->SetConstColour(QVector4D(0.5f, 1.0f, 0.5f, 0.25f));
    shader->SetChromaKeyColour(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
    shader->SetUseCubeTextureAll(false);
    shader->SetUseTextureAll(false);
    shader->SetUseLighting(false);
    shader->SetAmbient(QVector3D(1.0f, 1.0f, 1.0f));
    shader->SetDiffuse(QVector3D(1.0f, 1.0f, 1.0f));
    shader->SetSpecular(QVector3D(0.04f, 0.04f, 0.04f));
    shader->SetShininess(20.0f);

    shader->UpdateObjectUniforms();

    Renderer * renderer = Renderer::m_pimpl;
    AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                            renderer->GetTexturedCubePrimCount(),
                            0,
                            0,
                            0,
                            renderer->GetTexturedCubeVAO(),
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

    shader->SetConstColour(QVector4D(1,1,1,1));
}

TextureHandle* AssetVideo::GetTextureHandle(MediaContext * ctx, const bool left_eye)
{
    const bool sbs3d = props->GetSBS3D();
    const bool ou3d = props->GetOU3D();

    if (sbs3d || ou3d) {
        return (left_eye ?
                    media_player.GetLeftTextureHandle(ctx) :
                    media_player.GetRightTextureHandle(ctx));
    }
    else {
        return media_player.GetTextureHandle(ctx);
    }
    return AssetImage::null_image_tex_handle;
}

float AssetVideo::GetAspectRatio(MediaContext * ctx) const
{
    const float v = media_player.GetAspectRatio(ctx);
    //qDebug() << "AssetVideo::GetAspectRatio" << v;
    return v;
}

void AssetVideo::mousePressEvent(MediaContext * ctx, QMouseEvent * e)
{
    media_player.mousePressEvent(ctx, e);
}

void AssetVideo::mouseMoveEvent(MediaContext * ctx, QMouseEvent * e)
{
    media_player.mouseMoveEvent(ctx, e);
}

void AssetVideo::mouseReleaseEvent(MediaContext * ctx, QMouseEvent * e)
{
    media_player.mouseReleaseEvent(ctx, e);
}

void AssetVideo::SetCursorActive(MediaContext * ctx, const bool b)
{
    media_player.SetCursorActive(ctx, b);
}

bool AssetVideo::GetCursorActive(MediaContext * ctx) const
{
    return media_player.GetCursorActive(ctx);
}

int AssetVideo::GetWidth(MediaContext * ctx) const
{
    return media_player.GetWidth(ctx);
}

int AssetVideo::GetHeight(MediaContext * ctx) const
{
    return media_player.GetHeight(ctx);
}

bool AssetVideo::GetReady(MediaContext * ctx) const
{
    return media_player.GetReady(ctx);
}

bool AssetVideo::GetFinished(MediaContext * ctx) const
{
    return media_player.GetReady(ctx);
}


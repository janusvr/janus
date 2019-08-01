#include "assetsound.h"

AssetSound::AssetSound() :
    sound_enabled(true),
    volume(70),
    default_volume(70)
{
    props->SetType(TYPE_ASSETSOUND);
}

AssetSound::~AssetSound()
{
    //media.stop();
}

void AssetSound::SetupOutput(MediaContext * ctx, const bool loop)
{
    media_player.SetupOutput(ctx, props->GetSrcURL(), loop, true);
}

void AssetSound::ClearOutput(MediaContext * ctx)
{
    MediaPlayer::ClearOutput(ctx);
}

void AssetSound::Play(MediaContext * ctx)
{
    media_player.Play(ctx);
}

void AssetSound::Seek(MediaContext * ctx, const float pos)
{
    media_player.Seek(ctx, pos);
}

void AssetSound::Pause(MediaContext * ctx)
{
    media_player.Pause(ctx);
}

void AssetSound::Stop(MediaContext * ctx)
{
    media_player.Stop(ctx);
}

bool AssetSound::CanPause(MediaContext * ctx)
{
    return media_player.CanPause(ctx);
}

float AssetSound::GetCurTime(MediaContext * ctx) const
{
    return media_player.GetCurTime(ctx);
}

float AssetSound::GetTotalTime(MediaContext * ctx) const
{
    return media_player.GetTotalTime(ctx);
}

void AssetSound::SetSoundEnabled(MediaContext * ctx, const bool b)
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

bool AssetSound::GetPlaying(MediaContext * ctx) const
{
    return media_player.GetPlaying(ctx);
}

bool AssetSound::GetReady(MediaContext * ctx)
{
    return media_player.GetReady(ctx);
}

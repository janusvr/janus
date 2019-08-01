#ifndef ASSETSOUND_H
#define ASSETSOUND_H

#include <AL/al.h>
#include <AL/alc.h>

#include <QtCore>
#include <QtConcurrent/QtConcurrent>

#include "asset.h"
#include "mediaplayer.h"

class AssetSound : public Asset
{
    Q_OBJECT

public:

    AssetSound(); //load image through a URL
    ~AssetSound();

    void SetupOutput(MediaContext * ctx, const bool loop);
    static void ClearOutput(MediaContext * ctx);

    void Play(MediaContext * ctx);
    void Seek(MediaContext * ctx, const float pos);
    void Pause(MediaContext * ctx);
    void Stop(MediaContext * ctx);

    bool CanPause(MediaContext * ctx);

    float GetCurTime(MediaContext * ctx) const;
    float GetTotalTime(MediaContext * ctx) const;

    void SetSoundEnabled(MediaContext * ctx, const bool b);

    bool GetPlaying(MediaContext * ctx) const;
    bool GetReady(MediaContext * ctx);

private:

    MediaPlayer media_player;

    bool sound_enabled;

    int volume;
    int default_volume;
};

#endif // ASSETSOUND_H

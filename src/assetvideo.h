#ifndef ASSETVIDEO_H
#define ASSETVIDEO_H

#include "asset.h"
#include "assetshader.h"
#include "mathutil.h"
#include "rendererinterface.h"
#include "mediaplayer.h"

class AssetVideo : public Asset
{

public:

    AssetVideo();
    virtual ~AssetVideo();

    void SetupOutput(MediaContext * ctx);
    static void ClearOutput(MediaContext * ctx);

    void mousePressEvent(MediaContext * ctx, QMouseEvent * e);
    void mouseMoveEvent(MediaContext * ctx, QMouseEvent * e);
    void mouseReleaseEvent(MediaContext * ctx, QMouseEvent * e);   

    void SetCursorActive(MediaContext * ctx, const bool b);
    bool GetCursorActive(MediaContext * ctx) const;

    bool GetPlaying(MediaContext * ctx);

    void Play(MediaContext * ctx);
    void Seek(MediaContext * ctx, const float pos);
    void Pause(MediaContext * ctx);
    void Stop(MediaContext * ctx);

    bool CanPause(MediaContext * ctx);

    float GetCurTime(MediaContext * ctx) const;
    float GetTotalTime(MediaContext * ctx) const;

    void SetSoundEnabled(MediaContext * ctx, const bool b);

    int GetWidth(MediaContext * ctx) const;
    int GetHeight(MediaContext * ctx) const;

    float GetAspectRatio(MediaContext * ctx) const;

    void DrawGL(MediaContext * ctx, QPointer <AssetShader> shader, const bool left_eye, const float aspect_ratio);
    void DrawSelectedGL(QPointer <AssetShader> shader);
    TextureHandle* GetTextureHandle(MediaContext * ctx, const bool left_eye);

    bool GetReady(MediaContext * ctx) const;
    bool GetFinished(MediaContext * ctx) const;

private:              

    MediaPlayer media_player;

    bool sound_enabled;   

    int volume;
    int default_volume;       
};

#endif // ASSETVIDEO_H

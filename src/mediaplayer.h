#ifndef MediaPlayer_H
#define MediaPlayer_H

#include <QtGui>
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>
#include <QGLWidget>

#ifdef WIN32
#define ssize_t SSIZE_T
#endif
#include <vlc/vlc.h>

#include <AL/al.h>
#include <AL/alc.h>

#include "rendererinterface.h"
#include "assetimage.h"

class MediaPlayer;

struct MediaContext
{
    //Texture info
    QPointer<TextureHandle> m_texture_handles[2] = {nullptr,nullptr};

    //Mouse info
    bool cursor_active = false;
    QPoint cursor_pos;

    //Setup flag
    bool setup = false;

    //VLC info
    libvlc_media_player_t * media_player = nullptr;
    libvlc_media_t * media = nullptr;
    QString src;

    //Video info
    QMutex video_lock;
    QImage * img[2] = {nullptr,nullptr};
    unsigned int video_width = 1080;
    unsigned int video_height = 720;

    bool sbs3d = false;
    bool ou3d = false;
    bool reverse3d = false;
    bool update_tex = false;

    //Audio info
    QMutex audio_lock;
    int audio_channels = 1;
    int audio_sample_rate = 44100;
    int audio_sample_size = 16;

    QVector3D pos = QVector3D(0,0,0);
    QVector3D vel = QVector3D(0,0,0);
    float dist = 0.0f;
    float gain = 1.0f;
    float doppler_factor = 1.0f;
    float pitch = 1.0f;
    bool positional_sound = true;

    ALuint openal_source = 0;
    bool playing = false;
    QList <ALuint> * buffer_queue = nullptr;

    bool audio_only = false;
    bool loop = false;

    bool app_paused = false;

    MediaPlayer * player = nullptr;
};

class MediaPlayer : public QObject
{

Q_OBJECT

public:

    MediaPlayer();
    ~MediaPlayer();

    void SetupOutput(MediaContext * ctx, QString url, const bool loop, const bool audio_only = false);
    static void ClearOutput(MediaContext * ctx);
    static void ClearVLC(MediaContext * ctx);

    bool GetPlaying(MediaContext * ctx) const;
    bool GetReady(MediaContext * ctx) const;

    int GetWidth(MediaContext * ctx) const;
    int GetHeight(MediaContext * ctx) const;

    void SetVolume(MediaContext * ctx, const int i);
    int GetVolume(MediaContext * ctx) const;

    void Play(MediaContext * ctx);
    void Restart(MediaContext * ctx);
    void Seek(MediaContext * ctx, const float pos);
    void Pause(MediaContext * ctx);
    void Stop(MediaContext * ctx);

    bool CanPause(MediaContext * ctx);

    static void handleEvent(const libvlc_event_t* pEvt, void* pUserData);

    //VLC video callbacks
    static void *lock(void *data, void **p_pixels);
    static void unlock(void *data, void *id, void *const *p_pixels);
    static void display(void *data, void *id);

    //VLC audio callbacks
    static void play(void *data, const void *samples, unsigned count, int64_t pts);
    static void pause(void *data, int64_t pts);
    static void resume(void *data, int64_t pts);
    static void flush(void *data, int64_t pts);
    static void drain(void *data);

    float GetCurTime(MediaContext * ctx) const;
    float GetTotalTime(MediaContext * ctx) const;

    void mousePressEvent(MediaContext * ctx, QMouseEvent * e);
    void mouseMoveEvent(MediaContext * ctx, QMouseEvent * e);
    void mouseReleaseEvent(MediaContext * ctx, QMouseEvent * e);
    void SetCursorActive(MediaContext * ctx, const bool b);
    bool GetCursorActive(MediaContext * ctx) const;

    QPointer <TextureHandle> GetTextureHandle(MediaContext * ctx);
    QPointer <TextureHandle> GetLeftTextureHandle(MediaContext * ctx);
    QPointer <TextureHandle> GetRightTextureHandle(MediaContext * ctx);

    void SetUpdateTexture(MediaContext * ctx, const bool b);
    float GetAspectRatio(MediaContext * ctx) const;

    void Set3D(const bool sbs3d, const bool ou3d, const bool reverse3d);

signals:

    void signalPlay(MediaContext * ctx);
    void signalRestart(MediaContext * ctx);
    void signalSeek(MediaContext * ctx, float pos);
    void signalPause(MediaContext * ctx);
    void signalStop(MediaContext * ctx);

private slots:

    void slotPlay(MediaContext * ctx);
    void slotRestart(MediaContext * ctx);
    void slotSeek(MediaContext * ctx, float pos);
    void slotPause(MediaContext * ctx);
    void slotStop(MediaContext * ctx);

private:

    void DrawInterfaceOverlay(MediaContext * ctx, QImage & img);
    void UpdateTexture(MediaContext * ctx);
    void UpdateLeftRightTextures(MediaContext * ctx);

    void log_cb(void *logdata, int level, const libvlc_log_t *ctx, const char *fmt, va_list args);

    static int buffer_pool_size;

    bool sbs3d;
    bool ou3d;
    bool reverse3d;

    static libvlc_instance_t * vlcInstance;
    //MediaContext ctx;
};

#endif // MediaPlayer_H

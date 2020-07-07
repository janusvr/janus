#include "mediaplayer.h"

int MediaPlayer::buffer_pool_size = 128;
libvlc_instance_t * MediaPlayer::vlcInstance = nullptr;

MediaPlayer::MediaPlayer() :
    sbs3d(false),
    ou3d(false),
    reverse3d(false)
{
    if (!vlcInstance) {
        // VLC pointers
        const char * const vlc_args[] = {
            "--intf=dummy", // Don't use any interface
            "--quiet",
            /*
            "--ignore-config",        // Don't use VLC's config
            "--extraintf=logger", // Don't use any interface
            "--verbose=0",  // Be verbose
            "--v=0",
            "--file-logging",
            "--logfile=vlc_log.txt",
            "--logmode=text",
            "--log-verbose=1",
            "-q",
            */
            "--no-ts-trust-pcr" // Fixes playback for some .m3u8 files
        };

        // We launch VLC
        vlcInstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    }

    connect(this, SIGNAL(signalPlay(MediaContext*)), this, SLOT(slotPlay(MediaContext*)) );
    connect(this, SIGNAL(signalRestart(MediaContext*)), this, SLOT(slotRestart(MediaContext*)) );
    connect(this, SIGNAL(signalSeek(MediaContext*,float)), this, SLOT(slotSeek(MediaContext*,float)) );
    connect(this, SIGNAL(signalPause(MediaContext*)), this, SLOT(slotPause(MediaContext*)) );
    connect(this, SIGNAL(signalStop(MediaContext*)), this, SLOT(slotStop(MediaContext*)) );
}

MediaPlayer::~MediaPlayer()
{

}

void MediaPlayer::handleEvent(const libvlc_event_t* pEvt, void* pUserData)
{
    struct MediaContext *ctx = (struct MediaContext *) pUserData;

    switch(pEvt->type)
    {
    case libvlc_MediaPlayerEndReached:
        //qDebug() << "MediaPlayerEndReached";
        //Queue object to be setup again, if stopped and not looping or if stopped and gzipped and looping
        if (!ctx->loop) {
            //ctx->setup = false;
            ctx->player->Stop(ctx);
        }
        else {
            ctx->playing = false;
            ctx->player->Restart(ctx);
        }
        break;
    default:
        break;
        //qDebug() << libvlc_event_type_name(pEvt->type);
    }
}

void MediaPlayer::SetupOutput(MediaContext * ctx, QString vid_url, const bool loop, const bool audio_only)
{
    if (!ctx->setup){
        MediaPlayer::ClearOutput(ctx);

        ctx->video_lock.lock();
        ctx->audio_lock.lock();

        ctx->setup = true;
        ctx->audio_only = audio_only;
        ctx->player = this;

        if (!ctx->audio_only){
            ctx->img[0] = new QImage(1080, 720, QImage::Format_ARGB32);
            ctx->img[1] = new QImage(1080, 720, QImage::Format_ARGB32);

            ctx->ou3d = ou3d;
            ctx->sbs3d = sbs3d;
            ctx->reverse3d = reverse3d;

            ctx->m_texture_handles[0] = AssetImage::null_image_tex_handle;
            ctx->m_texture_handles[1] = AssetImage::null_image_tex_handle;
        }

        //Generate OpenAL source for video
        ctx->buffer_queue = new QList<ALuint>();
        ALuint * buffer = (ALuint *) malloc(sizeof(ALuint) * buffer_pool_size);
        if (buffer != nullptr)
        {                    
            alGenSources(1, &(ctx->openal_source));
            alGenBuffers(buffer_pool_size, buffer);

            for (int i=0;i<buffer_pool_size;++i) {
                ctx->buffer_queue->push_back(buffer[i]);
            }

            ctx->src = vid_url;
            ctx->loop = loop;

            if (vlcInstance) {
                ctx->media = libvlc_media_new_location(vlcInstance, ctx->src.simplified().toUtf8().constData());
                ctx->media_player = libvlc_media_player_new_from_media(ctx->media);
                libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(ctx->media_player);                
                libvlc_event_attach(eventManager, libvlc_MediaPlayerEndReached, &MediaPlayer::handleEvent, ctx);
                libvlc_video_set_callbacks(ctx->media_player,
                                           &MediaPlayer::lock,
                                           &MediaPlayer::unlock,
                                           &MediaPlayer::display,
                                           ctx);                
                libvlc_video_set_format(ctx->media_player, "RGBA", 1080, 720, 1080*4);
                libvlc_audio_set_callbacks(ctx->media_player,
                                           &MediaPlayer::play,
                                           &MediaPlayer::pause,
                                           &MediaPlayer::resume,
                                           &MediaPlayer::flush,
                                           &MediaPlayer::drain,
                                           ctx);
                libvlc_audio_set_format(ctx->media_player, "S16N", 44100, 1);
            }
            else {
                qDebug("MediaPlayer::SetupOutput - libvlc_new NULL instance returned");
            }
        }
        else {
            qDebug("MediaPlayer::SetupOutput - Error: Failed to alloc data in VideoSurface::SetupOutput.");
        }

        ctx->audio_lock.unlock();
        ctx->video_lock.unlock();
    }
}

void MediaPlayer::ClearVLC(MediaContext * ctx)
{
    if (libvlc_media_player_is_playing(ctx->media_player)){
        libvlc_media_player_stop(ctx->media_player);
    }

    libvlc_media_release(ctx->media);
    libvlc_media_player_release(ctx->media_player);

    ctx->media_player = nullptr;
    ctx->media = nullptr;
}

void MediaPlayer::ClearOutput(MediaContext * ctx)
{
    ctx->video_lock.lock();
    ctx->audio_lock.lock();

    for (int i=0; i<2; i++)
    {
        if (ctx->img[i]) {
            delete ctx->img[i];
            ctx->img[i] = nullptr;
        }
    }

    if (ctx->media_player){
        QtConcurrent::run(&MediaPlayer::ClearVLC, ctx).waitForFinished();
    }

    if (ctx->openal_source > 0) {
        ALint is_playing;
        alGetSourcei(ctx->openal_source, AL_SOURCE_STATE, &is_playing);
        if (is_playing == AL_PLAYING) {
            alSourceStop(ctx->openal_source);
        }

        //Dequeue buffers
        int buffers_to_dequeue = 0;
        alGetSourcei(ctx->openal_source, AL_BUFFERS_QUEUED, &buffers_to_dequeue);
        if (buffers_to_dequeue > 0)
        {
            QVector<ALuint> buffHolder;
            buffHolder.resize(buffers_to_dequeue);
            alSourceUnqueueBuffers(ctx->openal_source, buffers_to_dequeue, buffHolder.data());
            for (int i=0;i<buffers_to_dequeue;++i) {
                // Push the recovered buffers back on the queue
                alDeleteBuffers(1, &buffHolder[i]);
            }
        }

        if (ctx->buffer_queue) {
            while (!ctx->buffer_queue->isEmpty()) {
                ALuint myBuff = ctx->buffer_queue->front();
                ctx->buffer_queue->pop_front();
                alDeleteBuffers(1, &myBuff);
            }

            ctx->buffer_queue->clear();
            delete ctx->buffer_queue;
        }

        alSourcei(ctx->openal_source, AL_BUFFER, 0);

        alDeleteSources(1, &(ctx->openal_source));
        ctx->openal_source = 0;
    }

    ctx->playing = false;
    ctx->setup = false;
    ctx->loop = false;

    ctx->audio_lock.unlock();
    ctx->video_lock.unlock();
}


void MediaPlayer::Set3D(const bool sbs3d, const bool ou3d, const bool reverse3d)
{
    this->sbs3d = sbs3d;
    this->ou3d = ou3d;
    this->reverse3d = reverse3d;
}

bool MediaPlayer::GetPlaying(MediaContext * ctx) const
{
    return (ctx->media_player && libvlc_media_player_is_playing(ctx->media_player));
}

bool MediaPlayer::CanPause(MediaContext * ctx)
{
    return (ctx->media_player && libvlc_media_player_can_pause(ctx->media_player));
}


bool MediaPlayer::GetReady(MediaContext * ctx) const
{
    return (ctx->setup);
}

void MediaPlayer::SetVolume(MediaContext * ctx, const int i)
{
    if (ctx->media_player) libvlc_audio_set_volume(ctx->media_player, i);
}

int MediaPlayer::GetVolume(MediaContext * ctx) const
{
    return (ctx->media_player) ? (int) libvlc_audio_get_volume(ctx->media_player) : 70;
}

void MediaPlayer::Play(MediaContext * ctx)
{
    emit signalPlay(ctx);
}

void MediaPlayer::slotPlay(MediaContext *ctx)
{
    if (ctx->media_player && !ctx->playing && libvlc_media_player_is_playing(ctx->media_player) == 0){
        libvlc_media_player_play(ctx->media_player);
        ctx->playing = true;
    }
}

void MediaPlayer::Restart(MediaContext * ctx)
{
    emit signalRestart(ctx);
}

void MediaPlayer::slotRestart(MediaContext *ctx)
{
    if (ctx->media_player){
        libvlc_media_player_stop(ctx->media_player);
        libvlc_media_player_play(ctx->media_player);
        ctx->playing = true;
    }
}

void MediaPlayer::Seek(MediaContext * ctx, const float pos)
{
    emit signalSeek(ctx,pos);
}

void MediaPlayer::slotSeek(MediaContext *ctx, float pos)
{
    if (ctx->media_player && libvlc_media_player_is_seekable(ctx->media_player)) {
        libvlc_media_player_set_time(ctx->media_player, pos*1000.0f);
    }
}

void MediaPlayer::Pause(MediaContext * ctx)
{
    emit signalPause(ctx);
}

void MediaPlayer::slotPause(MediaContext *ctx)
{
    if (ctx->media_player && ctx->playing && libvlc_media_player_can_pause(ctx->media_player)){
        libvlc_media_player_pause(ctx->media_player); //Pause if playing
        ctx->playing = false;
    }
}

void MediaPlayer::Stop(MediaContext * ctx)
{
    emit signalStop(ctx);
}

void MediaPlayer::slotStop(MediaContext *ctx)
{
    if (ctx->media_player) {
        libvlc_media_player_stop(ctx->media_player);
        ctx->playing = false;
    }
}

float MediaPlayer::GetCurTime(MediaContext * ctx) const
{
    if (ctx->media_player){
        return float(double(libvlc_media_player_get_time(ctx->media_player)) / 1000.0);
    }
    return 0;
}

float MediaPlayer::GetTotalTime(MediaContext * ctx) const
{
    if (ctx->media_player){
        return float(double(libvlc_media_player_get_length(ctx->media_player)) / 1000.0);
    }
    return 0;
}

int MediaPlayer::GetWidth(MediaContext * ctx) const
{
    return (ctx->sbs3d) ? ctx->video_width / 2 : ctx->video_width;
}

int MediaPlayer::GetHeight(MediaContext * ctx) const
{
    return (ctx->ou3d) ? ctx->video_height / 2 : ctx->video_height;
}

void MediaPlayer::SetUpdateTexture(MediaContext * ctx, const bool b)
{
    ctx->update_tex = b;
}

void MediaPlayer::UpdateTexture(MediaContext * ctx)
{
    if (ctx->audio_only) {
        return;
    }

    if (ctx->update_tex && ctx->video_lock.tryLock()) {
        if (ctx->img[0] && !ctx->img[0]->isNull()) {
            if (ctx->m_texture_handles[0] == nullptr || ctx->m_texture_handles[0] == AssetImage::null_image_tex_handle) {
                ctx->m_texture_handles[0] = Renderer::m_pimpl->CreateTextureQImage(*(ctx->img[0]), true, true, false, TextureHandle::ALPHA_TYPE::BLENDED, TextureHandle::COLOR_SPACE::SRGB);
            }
            else {
                Renderer::m_pimpl->UpdateTextureHandleData(ctx->m_texture_handles[0], 0 ,0, 0,
                        ctx->img[0]->width(), ctx->img[0]->height(), GL_RGBA, GL_UNSIGNED_BYTE, (void *)ctx->img[0]->constBits(), ctx->img[0]->width() * ctx->img[0]->height() * 4);
                Renderer::m_pimpl->GenerateTextureHandleMipMap(ctx->m_texture_handles[0]);
            }
        }
        ctx->update_tex = false;
        ctx->video_lock.unlock();
    }
}

void MediaPlayer::UpdateLeftRightTextures(MediaContext * ctx)
{
    if (ctx->audio_only) {
        return;
    }

    if (ctx->update_tex && ctx->video_lock.tryLock()) {
        for (int i=0; i<2; ++i) {
            if (ctx->img[i] && !ctx->img[i]->isNull()) {
                if (!ctx->m_texture_handles[i] || ctx->m_texture_handles[i] == AssetImage::null_image_tex_handle) {
                    ctx->m_texture_handles[i] = Renderer::m_pimpl->CreateTextureQImage(*(ctx->img[i]), true, true, false, TextureHandle::ALPHA_TYPE::BLENDED, TextureHandle::COLOR_SPACE::SRGB);
                }
                else
                {
                    Renderer::m_pimpl->UpdateTextureHandleData(ctx->m_texture_handles[i], 0, 0, 0,
                            ctx->img[i]->width(), ctx->img[i]->height(), GL_RGBA, GL_UNSIGNED_BYTE, (void *)ctx->img[i]->constBits(), ctx->img[i]->width() * ctx->img[i]->height() * 4);
                    Renderer::m_pimpl->GenerateTextureHandleMipMap(ctx->m_texture_handles[i]);
                }
            }
        }

        ctx->update_tex = false;
        ctx->video_lock.unlock();
    }
}

QPointer <TextureHandle> MediaPlayer::GetTextureHandle(MediaContext * ctx)
{
    UpdateTexture(ctx);
    return ctx->m_texture_handles[0];
}

QPointer <TextureHandle> MediaPlayer::GetLeftTextureHandle(MediaContext * ctx)
{
    UpdateLeftRightTextures(ctx);
    return ctx->m_texture_handles[0];
}

QPointer <TextureHandle> MediaPlayer::GetRightTextureHandle(MediaContext * ctx)
{
    UpdateLeftRightTextures(ctx);
    return ctx->m_texture_handles[1];
}

float MediaPlayer::GetAspectRatio(MediaContext * ctx) const
{
    float aspect = 0.7f;
    if (ctx->audio_only || !ctx->img[0] || ctx->img[0]->isNull() || ctx->video_width == 0 || ctx->video_height == 0) {
        aspect = 0.7f;
    }
    else {
        aspect = (float(ctx->video_height) / float(ctx->video_width));
        if (ctx->sbs3d){
            aspect *= 2.0f;
        }
        else if (ctx->ou3d){
            aspect *= 0.5f;
        }
    }    

    //62.0 - detect rotated videos, and alter aspect ratio accordingly
    libvlc_video_orient_t orient = libvlc_video_orient_top_left;
    libvlc_media_track_t **mediatracks;
    unsigned trackcount = libvlc_media_tracks_get(ctx->media, &mediatracks);
    for (uint t = 0; t < trackcount; ++t) {
        if (mediatracks[t]->i_type == libvlc_track_video) {
            orient = (*mediatracks)->video->i_orientation;
        }
    }
    if (orient == libvlc_video_orient_left_bottom || orient == libvlc_video_orient_right_top) {
        aspect = 1.0f / aspect;
    }

    return aspect;
}


void MediaPlayer::DrawInterfaceOverlay(MediaContext * ctx, QImage & img)
{
    if (ctx->audio_only) return;
    //render interface overlay
    if (ctx->cursor_active) {
        const int height = img.height();

        QPixmap play_pixmap;
        if (ctx->playing) {
            play_pixmap = QApplication::style()->standardIcon(QStyle::SP_MediaPause).pixmap(32,32);
        }
        else {
            play_pixmap = QApplication::style()->standardIcon(QStyle::SP_MediaPlay).pixmap(32,32);
        }
        QPixmap stop_pixmap = QApplication::style()->standardIcon(QStyle::SP_MediaStop).pixmap(32,32);
        QPixmap backward_pixmap = QApplication::style()->standardIcon(QStyle::SP_MediaSeekBackward).pixmap(32,32);
        QPixmap forward_pixmap = QApplication::style()->standardIcon(QStyle::SP_MediaSeekForward).pixmap(32,32);

        QPainter painter(&img);
        painter.drawPixmap(0,height-32,play_pixmap);
        painter.drawPixmap(32,height-32,stop_pixmap);
        painter.drawPixmap(64,height-32,backward_pixmap);
        painter.drawPixmap(196,height-32,forward_pixmap);

        if (ctx->media_player && libvlc_media_player_is_seekable(ctx->media_player)) {
            const double pos = double(libvlc_media_player_get_position(ctx->media_player)) / double(libvlc_media_player_get_length(ctx->media_player)) * 100.0;
            painter.drawRect(96,height-16, 100, 1);
            painter.drawRect(96+pos,height-24, 1, 16);
        }
    }
}

void MediaPlayer::mousePressEvent(MediaContext * ctx, QMouseEvent * e)
{
    ctx->cursor_pos = e->pos();

    /* cruzjo - Interface not working, commented out for now
      if (cursor_pos.y() > img[0].height()-32) {
        if (cursor_pos.x() < 32) {
            if (IsPlaying()) {
                Pause();
            }
            else {
                Play(false);
            }
        }
        else if (cursor_pos.x() < 64) {
            Stop();
        }
        else if (cursor_pos.x() < 96) {
            media.setPosition(media.position()-media.duration()/20);
        }
        else if (cursor_pos.x() < 196) {
            const double pos = double(cursor_pos.x()-96) / 100.0 * double(media.duration());
            media.setPosition(pos);
        }
        else if (cursor_pos.x() < 228 && media.isSeekable()) {
            media.setPosition(media.position()+media.duration()/20);
        }
    }
    else {*/
    //}
}

void MediaPlayer::mouseMoveEvent(MediaContext * ctx, QMouseEvent * e)
{
    ctx->cursor_pos = e->pos();

    //qDebug() << img[0].width() << img[0].height() << cursor_pos;
    const int width = (ctx->sbs3d) ? ctx->video_width / 2 : ctx->video_width;
    const int height = (ctx->ou3d) ? ctx->video_height / 2 : ctx->video_height;

    if (ctx->cursor_pos.x() < 2) {
        ctx->cursor_pos.setX(2);
    }
    if (ctx->cursor_pos.x() > width-2) {
        ctx->cursor_pos.setX(width-2);
    }
    if (ctx->cursor_pos.y() < 2) {
        ctx->cursor_pos.setY(2);
    }
    if (ctx->cursor_pos.y() > height-2) {
        ctx->cursor_pos.setY(height-2);
    }

    if (ctx->cursor_active) {
        ctx->update_tex = true;
    }
}

void MediaPlayer::mouseReleaseEvent(MediaContext * ctx, QMouseEvent * e)
{
    ctx->cursor_pos = e->pos();

    //if not clicking in the menu, clicking just goes between pause and play
    if (GetPlaying(ctx)) {
        Pause(ctx);
    }
    else {
        Play(ctx);
    }
}

void MediaPlayer::SetCursorActive(MediaContext * ctx, const bool b)
{
    ctx->cursor_active = b;
}

bool MediaPlayer::GetCursorActive(MediaContext * ctx) const
{
    return ctx->cursor_active;
}

//Callbacks for video frames

void *MediaPlayer::lock(void *data, void **p_pixels)
{
    MediaContext *ctx = (MediaContext *) data;

    ctx->video_lock.lock();

    if (!ctx->setup || ctx->audio_only || !ctx->media_player){
        ctx->video_lock.unlock();
        return NULL;
    }

    *(ctx->img[0]) = ctx->img[0]->scaled(1080, 720);
    *p_pixels = ctx->img[0]->bits();

    return NULL; /* picture identifier, not needed here */
}

void MediaPlayer::unlock(void *data, void *id, void *const *)
{
    MediaContext *ctx = (MediaContext *) data;

    if (ctx->audio_only || !ctx->media_player){
        ctx->video_lock.unlock();
        return;
    }

    libvlc_media_track_t **tracks;
    unsigned tracksCount;
    bool resized = false;
    tracksCount = libvlc_media_tracks_get(ctx->media, &tracks);
    if( tracksCount > 0 )
    {
        for(unsigned i = 0; i < tracksCount; i++)
        {
            libvlc_media_track_t *track = tracks[i];
            if(track->i_type == libvlc_track_video && track->i_id == 0)
            {
                libvlc_video_track_t *videoTrack = track->video;
                if (ctx->video_width != videoTrack->i_width || ctx->video_height != videoTrack->i_height)
                {
                    ctx->video_width = videoTrack->i_width;
                    ctx->video_height = videoTrack->i_height;

                    if (ctx->video_width > 1080 || ctx->video_height > 720) {
                        QImage * new_0 = new QImage(ctx->video_width, ctx->video_height, QImage::Format_ARGB32);
                        QImage * new_1 = new QImage(ctx->video_width, ctx->video_height, QImage::Format_ARGB32);

                        delete ctx->img[0];
                        delete ctx->img[1];

                        ctx->img[0] = new_0;
                        ctx->img[1] = new_1;

                        resized = true;
                    }
                }
            }
        }
        libvlc_media_tracks_release(tracks, tracksCount);
    }

    //qDebug() << "Video size" << w << h;

    if (!resized) {
        *(ctx->img[0]) = (ctx->img[0]->scaled(QSize(ctx->video_width,ctx->video_height)));
    }

    if (ctx->sbs3d || ctx->ou3d)
    {
        const int w = ctx->img[0]->width();
        const int h = ctx->img[0]->height();

        if (ctx->sbs3d)
        {
            if (ctx->reverse3d)
            {
                *(ctx->img[1]) = (ctx->img[0]->copy(0,0,w/2,h));
                *(ctx->img[0]) = (ctx->img[0]->copy(w/2,0,w/2,h));
            }
            else
            {
                *(ctx->img[1]) = (ctx->img[0]->copy(w/2,0,w/2,h));
                *(ctx->img[0]) = (ctx->img[0]->copy(0,0,w/2, h));
            }
        }
        else if (ctx->ou3d)
        {
            if (ctx->reverse3d)
            {
                *(ctx->img[1]) = (ctx->img[0]->copy(0,h/2,w,h/2));
                *(ctx->img[0]) = (ctx->img[0]->copy(0,0,w,h/2));
            }
            else
            {
                *(ctx->img[1]) = (ctx->img[0]->copy(0,0,w,h/2));
                *(ctx->img[0]) = (ctx->img[0]->copy(0,h/2,w,h/2));
            }
        }
    }
    else
    {
        delete ctx->img[1];
        ctx->img[1] = nullptr;
    }

    //ctx->img[0]->save(MathUtil::GetScreenshotPath() + "out-" + MathUtil::GetCurrentDateTimeAsString() + ".png", "png", 90);
    //qDebug() << "ctx->img" << ctx->img[0]->format();

    ctx->update_tex = true;

    ctx->video_lock.unlock();
    assert(id == NULL); /* picture identifier, not needed here */
}

void MediaPlayer::display(void *data, void *id)
{
    //Don't display
    (void) data;
    assert(id == NULL);
}

//Callbacks for audio frames

void MediaPlayer::play(void *data, const void *samples, unsigned count, int64_t )
{
    MediaContext *ctx = (MediaContext *) data;

    ctx->audio_lock.lock();

    if (!ctx->setup || ctx->openal_source == 0){
        ctx->audio_lock.unlock();
        return;
    }

    //qDebug() << "playing";
    if (ctx->positional_sound){
        alSourcei(ctx->openal_source, AL_SOURCE_RELATIVE, AL_FALSE);
        alSource3f(ctx->openal_source, AL_POSITION, ctx->pos.x(), ctx->pos.y(), ctx->pos.z()); //set source position
        alSource3f(ctx->openal_source, AL_VELOCITY, ctx->vel.x(), ctx->vel.y(), ctx->vel.z()); //set source velocity
        alSourcef(ctx->openal_source, AL_DOPPLER_FACTOR, ctx->doppler_factor);
    }
    else{
        alSourcei(ctx->openal_source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSource3f(ctx->openal_source, AL_POSITION, 0, 0, 0); //set source position
        alSource3f(ctx->openal_source, AL_VELOCITY, 0, 0, 0); //set source velocity
        alSourcef(ctx->openal_source, AL_DOPPLER_FACTOR, ctx->doppler_factor);
    }

    alSourcef(ctx->openal_source, AL_ROLLOFF_FACTOR, 2.0f);
    alSourcef(ctx->openal_source, AL_REFERENCE_DISTANCE, ctx->dist);

    alSourcef(ctx->openal_source, AL_PITCH, ctx->pitch);
    alSourcef(ctx->openal_source, AL_GAIN, ctx->gain);

    ALint availBuffers=0; // Buffers to be recovered
    alGetSourcei(ctx->openal_source, AL_BUFFERS_PROCESSED, &availBuffers);
    //qDebug() << availBuffers;
    if (availBuffers > 0) {
        QVector<ALuint> buffHolder;
        buffHolder.resize(availBuffers); //49.50 crash with audio on Windows
        alSourceUnqueueBuffers(ctx->openal_source, availBuffers, buffHolder.data());
        for (int i=0;i<availBuffers;++i) {
            // Push the recovered buffers back on the queue
            ctx->buffer_queue->push_back(buffHolder[i]);
        }
    }

    //qDebug() << "buffer input queue" << SoundManager::buffer_input_queue.size();
    if (!ctx->buffer_queue->isEmpty()) { // We just drop the data if no buffers are available

        ALuint myBuff = ctx->buffer_queue->front();
        ctx->buffer_queue->pop_front();

        //49.8 bugfix - use the buffer size (which can vary), not a fixed size causing portions of audio not to play
        QByteArray audio = QByteArray((const char *) samples, count*2); //16 BIT = 2 BYTES PER SAMPLE

        alBufferData(myBuff, AL_FORMAT_MONO16, audio.data(), audio.size(), ctx->audio_sample_rate);

        // Queue the buffer
        alSourceQueueBuffers(ctx->openal_source, 1, &myBuff);

        // Restart the source if needed
        ALint sState=0;
        alGetSourcei(ctx->openal_source, AL_SOURCE_STATE, &sState);
        if (sState != AL_PLAYING) {
            alSourcePlay(ctx->openal_source);
            //qDebug() << "playing" << openal_stream_source;
        }
    }
    ctx->audio_lock.unlock();
}

void MediaPlayer::pause(void *data, int64_t )
{
    MediaContext *ctx = (MediaContext *) data;

    ctx->audio_lock.lock();

    if (!ctx->setup || ctx->openal_source == 0){
        ctx->audio_lock.unlock();
        return;
    }

    ALint is_playing;
    alGetSourcei(ctx->openal_source, AL_SOURCE_STATE, &is_playing);
    if (is_playing == AL_PLAYING) {
        alSourcePause(ctx->openal_source);
    }

    ctx->audio_lock.unlock();
}

void MediaPlayer::resume(void *data, int64_t )
{
    (void) data;
}

void MediaPlayer::flush(void *data, int64_t )
{
    (void) data;
}

void MediaPlayer::drain(void *data)
{
    (void) data;
}

 void MediaPlayer::log_cb(void *logdata, int level, const libvlc_log_t *ctx, const char *fmt, va_list args)
 {
    //Do nothing
 }

#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>

#include <string.h>

#include <QVector>

#include "assetsound.h"
#include "player.h"
#include "multiplayermanager.h"
#include "roomobject.h"
#include "audioutil.h"

class MultiPlayerManager;
class RoomObject;

enum SOUND_EFFECT
{
    SOUND_RIFT,
    SOUND_NORIFT,
    SOUND_SPECIFYURL,
    SOUND_NEWENTRANCE,
    SOUND_FLIGHT,
    SOUND_NOFLIGHT,
    SOUND_ROOM,
    SOUND_RESET,
    SOUND_BACK,
    SOUND_CLICK1,
    SOUND_CLICK2,
    SOUND_SAVED,
    SOUND_SELECTOBJECT,
    SOUND_SELECTIMAGE,
    SOUND_NEWTEXT,
    SOUND_COPYING,
    SOUND_PASTING,
    SOUND_DELETING,
    SOUND_POP1,
    SOUND_POP2,
    SOUND_RECORDGHOST,
    SOUND_GHOSTSAVED,
    SOUND_ASSIGNAVATAR,
    SOUND_UPDATE,
    SOUND_AUTHENTICATE,
    SOUND_VIVE,
    SOUND_NOVIVE,
    SOUND_DESKTOP,
    SOUND_WMXR,
    SOUND_NOWMXR,

    SOUND_COUNT
};

class SoundManager
{

public:

    SoundManager();
    ~SoundManager();

    static float GetMicLevel();

    static void Load(QString device_id = NULL, QString capture_device_id_l = NULL);
    static void Unload();

    static QVariantList GetDevices(ALenum specifier);
    static ALCdevice * GetDevice();

    static void Play(const SOUND_EFFECT index, const bool loop, const QVector3D & p, const float dist);
    static void Stop(const SOUND_EFFECT index);
    static void StopAll();

    static void SetEnabled(const bool b);
    static bool GetEnabled();

    static void SetBuiltinSoundsEnabled(const bool b);
    static bool GetBuiltinSoundsEnabled();

    static bool GetCaptureDeviceEnabled();

    static void Update(QPointer <Player> player);

    static QList <QByteArray> GetMicBuffers();
    static void ClearMicBuffers();   

    static int input_frequency;
    static int input_capture_size;
    static int input_buffer_pool_size;

    static QVector <ALuint> buffer_input_pool;
    static QList <ALuint> buffer_input_queue;

    static void SetGainMic(float gain);
    static void SetThresholdVolume(float f);
    static bool GetThresholdPast();

private:

    static bool enabled;
    static bool builtin_sounds_enabled;
    static bool capture_device_enabled;

    static QVector <QPointer <AssetSound> > sounds;
    static QVector<QPointer<RoomObject>> sound_objects_queued_to_play;
    static QVector<QPointer<RoomObject>> sound_objects;

    static ALCcontext * context;
    static ALCdevice * device;        
    static ALuint buffer;
    static ALuint source;

    static QByteArray bufferdata;

    static ALCdevice * device_input;

    static float input_mic_level;
    static QByteArray bufferdata_input;
    static ALuint buffer_input;
    static ALuint source_input;

    //static ALuint equalizer;
    static float gain_mic;

    static QList <QByteArray> input_mic_buffers;

    static bool recording;

    static float threshold_volume;
    static QList <float> packet_max_volumes;
    static bool threshold_past;

    static const int packet_volumes_to_keep;

};

#endif // SOUNDMANAGER_H

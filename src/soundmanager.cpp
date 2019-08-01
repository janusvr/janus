#include "soundmanager.h"

QVector <QPointer <AssetSound> > SoundManager::sounds;
QVector <QPointer<RoomObject>> SoundManager::sound_objects;
QVector <QPointer<RoomObject>> SoundManager::sound_objects_queued_to_play;

bool SoundManager::enabled = true;
bool SoundManager::builtin_sounds_enabled = true;
bool SoundManager::capture_device_enabled = false;
float SoundManager::input_mic_level = 0.0f;
ALCdevice * SoundManager::device = NULL;
ALCdevice * SoundManager::device_input = NULL;
ALCcontext * SoundManager::context = NULL;
ALuint SoundManager::buffer = 0;
ALuint SoundManager::buffer_input = 0;
ALuint SoundManager::source = 0;
ALuint SoundManager::source_input = 0;
QByteArray SoundManager::bufferdata_input;
QByteArray SoundManager::bufferdata;

QVector <ALuint> SoundManager::buffer_input_pool;
QList <ALuint> SoundManager::buffer_input_queue;

int SoundManager::input_frequency = 44100; //44100; //used to be 24000, resulting in bad VOIP quality
int SoundManager::input_capture_size = 2880; //2880; //2646; //2880; //1024
int SoundManager::input_buffer_pool_size = 128; //32

float SoundManager::gain_mic = 1.0f;

QList <QByteArray> SoundManager::input_mic_buffers;

bool SoundManager::recording = false;

float SoundManager::threshold_volume = -20.0f;
QList <float> SoundManager::packet_max_volumes;
bool SoundManager::threshold_past = false;

const int SoundManager::packet_volumes_to_keep = 10;

SoundManager::SoundManager()
{
}

SoundManager::~SoundManager()
{
}

float SoundManager::GetMicLevel()
{
    return input_mic_level;
}

void SoundManager::Load(QString device_id, QString capture_device_id)
{
//    qDebug() << "SoundManager::Load()";

    //qDebug() << "Playback" << GetDevices(ALC_DEVICE_SPECIFIER);
    bool closed_playback = false;
    bool closed_capture = false;
    if (device_id == "No device"){
        if (device) {
            alcMakeContextCurrent(NULL);
            if (context) alcDestroyContext(context);
            alcCloseDevice(device);

            device = NULL;

            qDebug() << "SoundManager::Load() - Playback device closed";
        }
        closed_playback = true;
    }
    if (capture_device_id == "No device"){
        if (device_input) {
            alcCaptureStop(device_input);
            alcCaptureCloseDevice(device_input);

            device_input = NULL;

            qDebug() << "SoundManager::Load() - Capture device closed";

            capture_device_enabled = false;
        }
        closed_capture = true;
    }    

    QString device_to_open;
    if (!closed_playback){
            if (!GetDevices(ALC_DEVICE_SPECIFIER).contains(device_id) || device_id == "Default device" || device_id.isNull()){
                if (!device_id.isNull() && device_id != "Default device"){
                    qDebug() << "SoundManager::Load() - Playback device not found; using default";
                }
                device_id = GetDevices(ALC_DEVICE_SPECIFIER)[1].toString();
            }
            device_to_open = device_id;
        }

    //qDebug() << "Capture" << GetDevices(ALC_CAPTURE_DEVICE_SPECIFIER);
    QString capture_device_to_open;
    if (!closed_capture){
        if (!GetDevices(ALC_CAPTURE_DEVICE_SPECIFIER).contains(capture_device_id) || capture_device_id == "Default device" || capture_device_id.isNull()){
            if (!capture_device_id.isNull() && capture_device_id != "Default device") {
                qDebug() << "SoundManager::Load() - Capture device not found; using default";
            }
            capture_device_id = GetDevices(ALC_CAPTURE_DEVICE_SPECIFIER)[1].toString();
        }
        capture_device_to_open = capture_device_id;
    }  

    // Initialize Open AL (reinitialize if new device is chosen)
    if (((device == NULL && device_input == NULL) || QString::compare((char *) alcGetString(device, ALC_DEVICE_SPECIFIER), device_to_open) != 0
            || QString::compare((char *) alcGetString(device_input, ALC_CAPTURE_DEVICE_SPECIFIER), capture_device_to_open) != 0))
    {
        if(!closed_playback && (device == NULL || QString::compare((char *) alcGetString(device, ALC_DEVICE_SPECIFIER), device_to_open) != 0))
        {
            alcMakeContextCurrent(NULL);
            if (context) alcDestroyContext(context);
            if (device) alcCloseDevice(device);

//            device = alcOpenDevice((ALCchar *) device_to_open); // open default device
            device = alcOpenDevice(NULL); // open default device
            if (device) {
                // Request 1 Auxiliary Send per Source
                //ALint attribs[4] = {0};
                //attribs[0] = ALC_MAX_AUXILIARY_SENDS;
                //attribs[1] = 1;

                //context = alcCreateContext(device, attribs); // create context
                context = alcCreateContext(device, NULL); // create context
                if (context) {
        //            qDebug() << "SoundManager::Load() - Setting active context";
                    alcMakeContextCurrent(context); // set active context
                }
            }

            alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED); //AL_INVERSE_DISTANCE_CLAMPED - the default
            //alDistanceModel(AL_EXPONENT_DISTANCE_CLAMPED);
        }

        if (!closed_capture && (device_input == NULL || QString::compare((char *) alcGetString(device_input, ALC_CAPTURE_DEVICE_SPECIFIER), capture_device_to_open) != 0))
        {
            if (device_input) {
                alcCaptureStop(device_input);
                alcCaptureCloseDevice(device_input);

                device_input = NULL;
            }

            buffer_input_pool = QVector<ALuint>(input_buffer_pool_size, 0);
            bufferdata_input = QByteArray(input_capture_size*sizeof(short), '\0');

            device_input = alcCaptureOpenDevice(NULL, input_frequency, AL_FORMAT_MONO16, input_capture_size*sizeof(short));

            //qDebug() << "DEVICEINPUT" << device_input;
            if (device_input) {

                packet_max_volumes.clear();
                for (int i = 0; i < packet_volumes_to_keep; i++){
                    packet_max_volumes.push_back(-100.0f);
                }
                threshold_past = false;

//                Analytics::PostEvent("voip", "init");
                capture_device_enabled = true;

                alcCaptureStart(device_input);

                alGenBuffers(1, &buffer_input);
                alBufferData(buffer_input, AL_FORMAT_MONO16, bufferdata_input.data(), bufferdata_input.size(), input_frequency);

                alGenSources(1, &source_input);
            }

            // Request the default capture device with a half-second buffer
            alGenBuffers(input_buffer_pool_size, &buffer_input_pool[0]); // Create some buffer-objects

            // Queue our buffers onto an STL list
            for (int i=0;i<buffer_input_pool.size();++i) {
                buffer_input_queue.push_back(buffer_input_pool[i]);
            }
            //qDebug() << "SoundManager::SetupOpenAL()" << device_input;
        }     

        ALenum err = alGetError();
        if (err != AL_NO_ERROR) {
            qDebug() << "SoundManager::Load() - Error occurred: " << err;
        }
//        else {
//            qDebug() << "SoundManager::Load() - No errors";
//        }

        if (sounds.isEmpty() && builtin_sounds_enabled) {
            sounds.resize(SOUND_COUNT);

            for (int i=0; i<SOUND_COUNT; ++i) {
                QPointer <AssetSound> snd = new AssetSound();
                QString filename = "assets/sounds/female/" + QString::number(i) + QString(".wav");
                snd->SetSrc(MathUtil::GetApplicationURL(), filename);
                sounds[i] = snd;
            }
        }
    }
}

void SoundManager::Unload()
{
    if (device_input) {
        alcCaptureStop(device_input);
        alcCaptureCloseDevice(device_input);

        device_input = NULL;
    }  

    // Clean-up
    if (buffer > 0) {
        alDeleteBuffers(1, &buffer);
        buffer = 0;
    }

    alcMakeContextCurrent(NULL);
    if (context) alcDestroyContext(context);
    if (device) alcCloseDevice(device);
    device = NULL;
}

QVariantList SoundManager::GetDevices(ALenum specifier)
{
    QVariantList device_list;

    char *devices;
    devices = (char *) alcGetString(NULL, specifier);

    device_list.push_back("Default device");

    if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE)
    {
        while(devices && *devices !=NULL)
        {
            device_list.push_back(devices);
            devices += strlen(devices) + 1;
        }
    }
    device_list.push_back("No device");

    /*for (int j = 0; j < device_list.length(); j++){
        qDebug() << device_list[j];
    }*/

    return device_list;
}

ALCdevice * SoundManager::GetDevice()
{
    return device;
}

void SoundManager::Play(const SOUND_EFFECT index, const bool loop, const QVector3D & p, const float dist)
{
    if (index < 0 || index >= sounds.size() || !enabled || !builtin_sounds_enabled || index >= SOUND_COUNT) {
        return;
    }

    QPointer <RoomObject> new_sound_object = new RoomObject();
    new_sound_object->SetType(TYPE_SOUND);
    new_sound_object->SetAssetSound(sounds[index]);
    new_sound_object->GetProperties()->SetLoop(loop);
    new_sound_object->GetProperties()->SetPos(p);
    new_sound_object->GetProperties()->SetScale(QVector3D(dist, dist, dist));

    sounds[index]->SetupOutput(new_sound_object->GetMediaContext(), false);

    //        new_sound_object->Play();

    //        qDebug() << "SoundManager::Play3D(): queueing" << set << index;
    sound_objects_queued_to_play.push_back(new_sound_object);
}

void SoundManager::StopAll()
{
    //55.7 Note: below code only stops qmediaplayer backend sounds
    /*for (int i=0; i<sounds.size(); ++i) {
        sounds[i]->Stop(0);
    }*/

    for (int i=0; i<sound_objects.size(); ++i) {
        if (sound_objects[i]) {
            sound_objects[i]->Stop();
        }
    }
    sound_objects.clear();
    sound_objects_queued_to_play.clear();
}

void SoundManager::Stop(const SOUND_EFFECT index)
{
    if (index < 0 || index >= sounds.size()) {
        return;
    }

    if (sound_objects[index]) {
        sound_objects[index]->Stop();
    }
}

void SoundManager::SetEnabled(const bool b)
{
    //qDebug() << "setting enabled " << b;
    enabled = b;

    for (int i=0; i<sound_objects.size(); ++i) {
        if (sound_objects[i]) {
            QPointer <AssetSound> snd = sound_objects[i]->GetAssetSound();
            if (snd && snd->GetReady(sound_objects[i]->GetMediaContext())) {
                snd->SetSoundEnabled(sound_objects[i]->GetMediaContext(), b);
            }
        }
    }
    for (int i=0; i<sound_objects_queued_to_play.size(); ++i) {
        if (sound_objects_queued_to_play[i]) {
            QPointer <AssetSound> snd = sound_objects_queued_to_play[i]->GetAssetSound();
            if (snd && snd->GetReady(sound_objects_queued_to_play[i]->GetMediaContext())) {
                snd->SetSoundEnabled(sound_objects_queued_to_play[i]->GetMediaContext(), b);
            }
        }
    }
}

bool SoundManager::GetEnabled()
{
    return enabled;
}

void SoundManager::SetBuiltinSoundsEnabled(const bool b)
{
    builtin_sounds_enabled = b;
}

bool SoundManager::GetBuiltinSoundsEnabled()
{
    return builtin_sounds_enabled;
}

bool SoundManager::GetCaptureDeviceEnabled()
{
    return capture_device_enabled;
}

void SoundManager::Update(QPointer <Player> player)
{            
    const QVector3D pos = player->GetProperties()->GetPos()->toQVector3D()
            + player->GetProperties()->GetLocalHeadPos()->toQVector3D(); //+ player->GetEyePoint(); //+ player->GetHeadPos();
    alListener3f(AL_POSITION, pos.x(), pos.y(), pos.z());

    const QVector3D vel = player->GetProperties()->GetVel()->toQVector3D();
    alListener3f(AL_VELOCITY, vel.x(), vel.y(), vel.z());

    //0,1,2 - forward
    //3,4,5 - up
    const QVector3D forward = player->GetProperties()->GetViewDir()->toQVector3D();
    const QVector3D up = player->GetProperties()->GetUpDir()->toQVector3D();

    float directionvect[6];
    directionvect[0] = forward.x();
    directionvect[1] = forward.y();
    directionvect[2] = forward.z();
    directionvect[3] = up.x();
    directionvect[4] = up.y();
    directionvect[5] = up.z();
    alListenerfv(AL_ORIENTATION, directionvect);

    if (player->GetRecording()){
        recording = true;
    }
    else if (recording && !player->GetRecording()){
        QFile file(MathUtil::GetRecordingPath() + "recording-" + MathUtil::GetCurrentDateTimeAsString() + ".wav");
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            file.write(WavHeader(input_frequency, 16, 1, bufferdata.size()).getByteArray());
            file.write(bufferdata);
            file.close();
        }
        recording = false;
    }
    else if (!bufferdata.isNull()){
        bufferdata.clear();
    }

    bool capture_samples = false;

    if (device_input) {
        // Poll for captured audio
        ALCint samplesIn=0;  // How many samples are captured
        alcGetIntegerv(device_input, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &samplesIn);

        //qDebug() << buffer_input_queue.size(); //should generally be size 1 or 2
        if (samplesIn > input_capture_size) {
            // Grab the sound
            alcCaptureSamples(device_input, bufferdata_input.data(), input_capture_size);
            capture_samples = true;
        }
    }

    if (capture_samples) {
        int max = 0;

        for (int i=0;i<input_capture_size;i++) //input_capture_size*number_channels
        {
            int sample = ((ALshort*)bufferdata_input.data())[i] * gain_mic;
            if(sample < -32768) sample = -32768;
            else if(sample > 32767) sample = 32767;
            ((ALshort*)bufferdata_input.data())[i] = sample;

            max = std::max(max, abs(sample));

            if (player->GetRecording()) {
                bufferdata.push_back(char(sample));
                bufferdata.push_back(char(sample>>8));
            }
            //qDebug() << sample;
        }

        if (packet_max_volumes.length() == packet_volumes_to_keep){
            packet_max_volumes.pop_front();
            packet_max_volumes.push_back(20*log10(float(max)/float(32767)));
            QList<float>::iterator i;
            bool tp = false;
            for (i = packet_max_volumes.begin(); i != packet_max_volumes.end(); i++){
                //qDebug() << *i << threshold_volume;
                if (*i >= threshold_volume){
                    tp = true;
                    break;
                }
            }
            threshold_past = tp;
        }
        //}
            //hopefully solves the buffer queueing issue where sound falls behind
        if (player->GetSpeaking()) {
            input_mic_level = MathUtil::GetSoundLevel(bufferdata_input);
            input_mic_buffers.push_back(AudioUtil::encode(bufferdata_input).toBase64());
            //                qDebug() << "inputmicbuffers size" << input_mic_buffers.size();
        }
    }

//    qDebug() << "SoundManager::Update()" << sound_objects_queued_to_play.size();
    for (int i=0; i<sound_objects_queued_to_play.size(); ++i) {
        if (sound_objects_queued_to_play[i]) {
            QPointer <AssetSound> snd = sound_objects_queued_to_play[i]->GetAssetSound();
            if (snd && snd->GetReady(sound_objects_queued_to_play[i]->GetMediaContext())) {
                sound_objects.push_back(sound_objects_queued_to_play[i]);
                sound_objects.back()->Play();
                sound_objects_queued_to_play.erase(sound_objects_queued_to_play.begin() + i, sound_objects_queued_to_play.begin() + i + 1);
                --i;
            }
        }
        else {
			sound_objects_queued_to_play.erase(sound_objects_queued_to_play.begin() + i, sound_objects_queued_to_play.begin() + i + 1);
            --i;
        }
    }

    for (int i=0; i<sound_objects.size(); ++i) {
        if (sound_objects[i]) {
            sound_objects[i]->Update(0.0f);
            if (!sound_objects[i]->GetPlaying()) {
				sound_objects.erase(sound_objects.begin() + i, sound_objects.begin() + i + 1);
                --i;
            }
        }
        else {
			sound_objects.erase(sound_objects.begin() + i, sound_objects.begin() + i + 1);
            --i;
        }
    }

//    ALenum err = alGetError();
//    if (err != AL_NO_ERROR) {
//        qDebug() << "SoundManager::Update() - Error occurred: " << err;
//    }
}

QList <QByteArray> SoundManager::GetMicBuffers()
{
    return input_mic_buffers;
}

void SoundManager::ClearMicBuffers()
{
    input_mic_buffers.clear();
}

void SoundManager::SetGainMic(float gain)
{
    gain_mic = gain/100.0;
}

void SoundManager::SetThresholdVolume(float f)
{
    threshold_volume = f;
}

bool SoundManager::GetThresholdPast()
{
    return threshold_past;
}


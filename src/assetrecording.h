#ifndef ASSETRECORDING_H
#define ASSETRECORDING_H

#include <QtConcurrent/QtConcurrent>

#include "asset.h"
#include "webasset.h"

class AssetRecordingPacket
{
public:

    AssetRecordingPacket() :
        pTime(0.0)
    {

    }

    void Write(QTextStream & ofs)
    {
        ofs << pTime << " " << pPacket;
    }

    void SetTimeToEpoch()
    {
        pTime = double(QDateTime::currentMSecsSinceEpoch())/1000.0;
    }

    double pTime;
    QString pPacket;
};

class AssetRecording : public Asset
{
public:
    AssetRecording();
    virtual ~AssetRecording();

    void Load();
    void Unload();

    bool GetLoaded() const;
    bool GetProcessed() const;

    void Update();

    void Play(const bool l);
    void Seek(const float pos);
    void Pause();
    void Stop();

    bool GetPlaying() const;
    QList <AssetRecordingPacket> GetPackets();

    void SetRoomID(const QString s);
    QString GetRoomID() const;

private:

    void LoadDataThread();

    double start_time; //the time shown in the file the recording began (relative to epoch, in sec)

    QList <AssetRecordingPacket> recording_data;

    QTime dt_time;
    double play_time_elapsed;
    int packet_index;
    bool playing;

    QString room_id;
    QString original_room_id;
};

#endif // ASSETRECORDING_H

#include "assetrecording.h"

AssetRecording::AssetRecording() :
    start_time(0.0),
    play_time_elapsed(0.0),
    packet_index(0),
    playing(false)
{
//    qDebug() << "AssetRecording::AssetRecording()";
    props->SetType(TYPE_ASSETRECORDING);
    props->SetSampleRate(44100);
    dt_time.start();
}

AssetRecording::~AssetRecording()
{
}

void AssetRecording::Load()
{
//    qDebug() << "AssetRecording::Load()" << GetS("_src_url");
    WebAsset::Load(QUrl(props->GetSrcURL()));
}

void AssetRecording::Unload()
{
    WebAsset::Unload();
}

bool AssetRecording::GetLoaded() const
{
    return WebAsset::GetLoaded();
}

bool AssetRecording::GetProcessed() const
{
    return WebAsset::GetProcessed();
}

void AssetRecording::Update()
{
//    qDebug() << "AssetRecording::Update()" << GetLoaded() << GetProcessing() << room_id;
    if (GetLoaded() && !GetProcessing() && !room_id.isEmpty()) {
        SetProcessing(true);
        QtConcurrent::run(this, &AssetRecording::LoadDataThread);
    }

    //increment duration timer while playing
    if (playing) {
        const float dt = float(dt_time.restart()) / 1000.0f;
        play_time_elapsed += dt;
    }
    else {
        //trigger for autoplay
        if (GetLoaded() && GetProcessed() && props->GetAutoPlay()) {
            Play(props->GetLoop());
        }
    }
}

void AssetRecording::Play(const bool loop)
{
//    qDebug() << "AssetRecording::Play" << loop;
    props->SetLoop(loop);
    packet_index = 0;
    playing = true;
    play_time_elapsed = 0.0;
}

void AssetRecording::Seek(const float pos)
{
    if (!recording_data.isEmpty()) {
        packet_index = 0;
        play_time_elapsed = qMin(double(qMax(pos, 0.0f)), recording_data.last().pTime-start_time);

        const double cur_time = start_time + play_time_elapsed;
        for (int i=0; i<recording_data.size(); ++i) {
            if (cur_time>recording_data[i].pTime) {
    //            qDebug() << "cur time less than recording_data i" << i << recording_data.size() << QString::number(cur_time, 'g', 17) << QString::number(recording_data[i].pTime, 'g', 17);
                packet_index = i;
            }
            else {
                break;
            }
        }
    }
//    qDebug() << "AssetRecording::Seek seek" << play_time_elapsed << "packet_index" << packet_index;
}

void AssetRecording::Pause()
{
    playing = !playing;
}

void AssetRecording::Stop()
{
    play_time_elapsed = 0.0f;
    packet_index = 0;
    playing = false;
}

QList <AssetRecordingPacket> AssetRecording::GetPackets()
{
    const double cur_time = start_time + play_time_elapsed;
    QList <AssetRecordingPacket> packet_list;
    for (int i=packet_index; i<recording_data.size(); ++i) {
        if (recording_data[i].pTime <= cur_time) {
            packet_list.push_back(recording_data[i]);
            packet_index = i+1;
        }
        else {
            break;
        }
    }

    //restart if loop is set to true
    if (playing && packet_index >= recording_data.size()-1) {
        if (props->GetLoop()) {
            packet_index = 0;
            play_time_elapsed = 0.0;
        }
    }

//    qDebug() << "AssetRecording::GetPackets()" << packet_list.size();
    return packet_list;
}

bool AssetRecording::GetPlaying() const
{
    return playing;
}

void AssetRecording::LoadDataThread()
{
    if (GetProcessed()) {
        return;
    }

    const QByteArray & ba = GetData();
    QTextStream ifs(ba);

    recording_data.clear();

    while (!ifs.atEnd()) {
        const QString line = ifs.readLine();

        if (line.isEmpty()) {
            continue;
        }

        int time_index = line.indexOf(" ");
        if (time_index < 0) {
            continue;
        }

        AssetRecordingPacket p;
        p.pTime = QString(line.left(time_index)).toDouble();
        QString s = line.right(line.length()-time_index-1);
        if (recording_data.isEmpty()) {
            start_time = p.pTime;

            //get roomid off of first packet
            QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8());
            QJsonObject obj = doc.object();
            QJsonObject dataObject = obj.value("data").toObject();
            QVariantMap m = dataObject.toVariantMap();
//            qDebug() << s.toUtf8() << obj << dataObject << m;

            original_room_id = m["roomId"].toString();
        }

//        qDebug() << "replace" << original_room_id << room_id;
        p.pPacket = s.replace(original_room_id, room_id);
//        p.pPacket = s;

        recording_data.push_back(p);
//        qDebug() << "packet" << p.pTime << p.pPacket;
    }

    SetProcessed(true);
    SetFinished(true);
    ClearData();
}

void AssetRecording::SetRoomID(const QString s)
{
    room_id = s;
}

QString AssetRecording::GetRoomID() const
{
    return room_id;
}

#include "assetghost.h"

void GhostFrame::SetHeadXForm(const QVector3D & y, const QVector3D & z)
{
    head_xform.setColumn(0, QVector3D::crossProduct(y,z).normalized());
    head_xform.setColumn(1, y.normalized());
    head_xform.setColumn(2, z.normalized());
    head_xform.setColumn(3, QVector4D(0,0,0,1));

    //y and z are vectors expressed globally, but since our head transform is body-relative,
    //we need to invert that dir rotation about y (a rotation relative to the body's forward direction)
    const float angle0 = -(90.0f - atan2f(dir.z(), dir.x()) * MathUtil::_180_OVER_PI);
    QMatrix4x4 rot;
    rot.rotate(angle0, 0, 1, 0);
    head_xform = rot * head_xform;
}

AssetGhost::AssetGhost() :
    secs_per_frame(0.2f)
{
    props->SetType(TYPE_ASSETGHOST);
}

AssetGhost::~AssetGhost()
{
}

void AssetGhost::Load()
{
    WebAsset::Load(QUrl(props->GetSrcURL()));
}

void AssetGhost::Unload()
{
    frames.clear();    
    WebAsset::Unload();
}

void AssetGhost::SetFromFrames(const QVector <GhostFrame> & ghost_frames, const float secs_between_frames)
{
    if (frames.size() == 2 && ghost_frames.size() == 2) {
        QList <QString> unprocessed_edits = (frames[1].room_edits + ghost_frames[1].room_edits);
        QList <QString> unprocessed_deletes = (frames[1].room_deletes + ghost_frames[1].room_deletes);

        frames = ghost_frames;

        frames[1].room_edits = unprocessed_edits;
        frames[1].room_deletes = unprocessed_deletes;
    }
    else {
        frames = ghost_frames;
    }
    secs_per_frame = secs_between_frames;

    SetLoaded(true);
    SetProcessed(true);
    SetFinished(true);
}

void AssetGhost::LoadDataThread()
{
    if (GetProcessed()) {
        return;
    }

//    qDebug() << "AssetGhost::LoadDataThread()" << url;
    const QByteArray & ba = GetData();
    QTextStream ifs(ba);

    frames.clear();

    while (!ifs.atEnd()) {
        const QString line = ifs.readLine();

        if (line.isEmpty()) {
            continue;
        }
        else if (line[0] == '{') {
            QVariantMap m = QJsonDocument::fromJson(line.toLatin1()).object().toVariantMap();

            //convert JSON packet o into a GhostFrame
            GhostFrame f;
            AssetGhost::ConvertPacketToFrame(m, f);

            if (f.chat_message.length() > 0 && !frames.empty()) {
                frames.last().chat_message = f.chat_message;
            }
            else if (f.send_portal_url.length() > 0 && !frames.empty()) {
                frames.last().send_portal_url = f.send_portal_url;
                frames.last().send_portal_pos = f.send_portal_pos;
                frames.last().send_portal_fwd = f.send_portal_fwd;
                frames.last().send_portal_jsid = f.userid + "-" + f.send_portal_url;
            }
            else {
                f.time_sec = float(frames.size())*secs_per_frame;
                frames.push_back(f);
            }
        }
        else {
            secs_per_frame = 0.1f;

            QStringList eachline = line.split(" ");
            if (QString::compare(eachline.first(), "CHAT") == 0) {
                if (!frames.empty()) {
                    for (int i=1; i<eachline.size(); ++i) {
                        frames.last().chat_message += QString(" ") + eachline[i];
                    }
                    frames.last().chat_message = frames.last().chat_message.trimmed();
                }
            }
            else if (eachline.size() == 13 || eachline.size() == 16) {
                GhostFrame frame;
                frame.time_sec = float(frames.size())*secs_per_frame;
                frame.pos = QVector3D(eachline[1].toFloat(), eachline[2].toFloat(), eachline[3].toFloat());
                frame.dir = QVector3D(eachline[4].toFloat(), eachline[5].toFloat(), eachline[6].toFloat());
                frame.SetHeadXForm(QVector3D(eachline[10].toFloat(), eachline[11].toFloat(), eachline[12].toFloat()),
                                    QVector3D(eachline[7].toFloat(), eachline[8].toFloat(), eachline[9].toFloat()));
                frames.push_back(frame);
            }
        }
    }

//    qDebug() << "AssetGhost::LoadDataThread() - Loaded frames/packets:" << frames.size() << packets.size();
    SetProcessed(true);
    ClearData();
}

void AssetGhost::ClearFrames()
{
    frames.clear();
}

int AssetGhost::GetFrameIndex(const float time_sec)
{    
    const int i = int(time_sec/secs_per_frame);
    if (i < 0 || i >= frames.size()) {
        return -1;
    }         
    return i;
}

GhostFrame & AssetGhost::GetFrameByIndex(const int i)
{
    if (i >= 0 && i<frames.size()) {
        return frames[i];
    }    
    else {
        qDebug() << "AssetGhost::GetFrameByIndex() - Warning invalid frame index" << i << "of" << frames.size();
        frames.push_back(GhostFrame());
        return frames.last();
    }
}

int AssetGhost::GetNumFrames() const
{
    return frames.size();
}

void AssetGhost::ClearEditsDeletes()
{
    for (int i=0; i<frames.size(); ++i) {
        frames[i].room_edits.clear();
        frames[i].room_deletes.clear();
    }
}

bool AssetGhost::GetGhostFrame(const float time_sec, GhostFrame & frame)
{
    if (frames.empty()) {
        frame = GhostFrame();
        return true;
    }

    const int i1 = int(time_sec/secs_per_frame);
    const int i2 = i1+1;

    if (i1 < 0 || i1 >= frames.size()) {
        frame = frames.first();
        return true;
    }
    else if (i2 < 0 || i2 >= frames.size()) {
        frame = frames.last();
        return true;
    }
    else {
        //interpolate between these two
        frame = frames[i2];

        const float interval = (frames[i2].time_sec - frames[i1].time_sec);
        const float t = ((interval > 0.0f) ? ((time_sec - frames[i1].time_sec) / interval) : 1.0f); //49.12 - fixes NaN error

        frame.pos = frames[i1].pos * (1.0f - t) + frames[i2].pos * t;
        frame.dir = (frames[i1].dir * (1.0f - t) + frames[i2].dir * t).normalized();        
        frame.head_xform = MathUtil::InterpolateMatrices(frames[i1].head_xform, frames[i2].head_xform, t);
        frame.cursor_active = (frames[i1].cursor_active && frames[i2].cursor_active);
        if (frame.cursor_active) {            
            frame.cursor_xform = MathUtil::InterpolateMatrices(frames[i1].cursor_xform, frames[i2].cursor_xform, t);
            frame.cscale = frames[i1].cscale * (1.0f - t) + frames[i2].cscale * t;
        }
        frame.hands.first = LeapHand::Interpolate(frames[i1].hands.first, frames[i2].hands.first, t);
        frame.hands.second = LeapHand::Interpolate(frames[i1].hands.second, frames[i2].hands.second, t);
        frame.current_sound_level = frames[i1].current_sound_level * (1.0f - t) + frames[i2].current_sound_level * t;

        //62.12 - improve animations playback by using the first of the two being interpolated
        //(i.e. continue to run until stopped, or jump until landed)
        frame.anim_id = frames[i1].anim_id;

        return false;
    }
}

void AssetGhost::Update()
{
    if (GetLoaded() && !GetProcessing()) {
        SetProcessing(true);
        QtConcurrent::run(this, &AssetGhost::LoadDataThread);
    }
}

void AssetGhost::ConvertPacketToFrame(const QVariantMap & map, GhostFrame & frame)
{
    QVariantMap m = map;
    frame.userid = m["userId"].toString();
    frame.roomid = m["roomId"].toString();
//    qDebug() << "AssetGhost::ConvertPacketToFrame()" << frame.userid << frame.roomid;

    if (QString::compare(m["method"].toString(), "chat") == 0) {
        frame.chat_message = m["data"].toString();
        return;
    }
    else if (QString::compare(m["method"].toString(), "portal") == 0) {
        if (m.contains("data")) {
            m = m["data"].toMap();
        }

        frame.send_portal_url = m["url"].toString();

        QStringList list = m["pos"].toString().split(" ");
        if (list.size() >= 3) {
            frame.send_portal_pos = QVector3D(list[0].toFloat(), list[1].toFloat(), list[2].toFloat());
        }

        list = m["fwd"].toString().split(" ");
        if (list.size() >= 3) {
            frame.send_portal_fwd = QVector3D(list[0].toFloat(), list[1].toFloat(), list[2].toFloat());
        }
        return;
    }

    if (m.contains("data")) {
        m = m["data"].toMap();
    }
    else if (m.contains("position")) {
        m = m["position"].toMap();
    }
    else {
        return;
    }

    if (m.contains("pos")) {
        QStringList list = m["pos"].toString().split(" ");
        if (list.size() >= 3) {            
            frame.pos = QVector3D(list[0].toFloat(), list[1].toFloat(), list[2].toFloat());
        }
    }

    if (m.contains("dir")) {
        QStringList list = m["dir"].toString().split(" ");
        if (list.size() >= 3) {
            frame.dir = QVector3D(list[0].toFloat(), list[1].toFloat(), list[2].toFloat());
        }
    }

    QVector3D y(0,1,0);
    QVector3D z(0,0,1);

    if (m.contains("view_dir")) {
        QStringList list = m["view_dir"].toString().split(" ");
        if (list.size() >= 3) {
            z = QVector3D(list[0].toFloat(), list[1].toFloat(), list[2].toFloat());
        }
    }

    if (m.contains("up_dir")) {
        QStringList list = m["up_dir"].toString().split(" ");
        if (list.size() >= 3) {
            y = QVector3D(list[0].toFloat(), list[1].toFloat(), list[2].toFloat());
        }
    }       

    //65.2 - ignoring possibility of head translation for now
//    if (m.contains("head_pos")) {
//        QStringList list = m["head_pos"].toString().split(" ");
//        if (list.size() >= 3) {
//            frame.head_xform.setColumn(3, QVector4D(list[0].toFloat(), list[1].toFloat(), list[2].toFloat(), 1));
//        }
//    }

    frame.SetHeadXForm(y, z);

    if (m.contains("hmd_type")) {
        frame.hmd_type = m["hmd_type"].toString();
    }   

    //hacky thing so that animations for portal spawning and jumping, which are time-based, work
    if (m.contains("anim_id")) {
        frame.anim_id = m["anim_id"].toString();
    }

    if (m.contains("cpos")) {
        frame.cursor_active = true;
        frame.cursor_xform.setColumn(3, QVector4D(MathUtil::GetStringAsVector(m["cpos"].toString()), 1));
    }
    if (m.contains("cxdir")) {
        frame.cursor_xform.setColumn(0, MathUtil::GetStringAsVector(m["cxdir"].toString()));
    }
    if (m.contains("cydir")) {
        frame.cursor_xform.setColumn(1, MathUtil::GetStringAsVector(m["cydir"].toString()));
    }
    if (m.contains("czdir")) {
        frame.cursor_xform.setColumn(2, MathUtil::GetStringAsVector(m["czdir"].toString()));
    }
    if (m.contains("cscale")) {
        frame.cscale = m["cscale"].toString().toFloat();
    }
    if (m.contains("speaking")) {
        frame.speaking = m["speaking"].toBool();
    }
    if (m.contains("audio")) {
        QByteArray b = QByteArray::fromBase64(m["audio"].toByteArray());
        frame.sound_buffers.push_back(b); //Old
        frame.current_sound_level = MathUtil::GetSoundLevel(b);
    }
    if (m.contains("audio_opus")) {
        frame.sound_buffers.push_back(m["audio_opus"].toByteArray());
        frame.current_sound_level = (m.contains("sound_level"))?m["sound_level"].toFloat():1.0f;
    }

    int i=0;

    while (m.contains("audio_opus"+QString::number(i)) && i < 30) {
        frame.sound_buffers.push_back(m["audio_opus"+QString::number(i)].toByteArray());
        frame.current_sound_level = (m.contains("sound_level"))?m["sound_level"].toFloat():1.0f;
        ++i;
    }

    if (m.contains("typing")) {
        frame.typing = true;
    }

    if (m.contains("room_edit")) {
//        qDebug() << "AssetGhost::ConvertPacketToFrame INCOMING ROOM EDIT" << m["room_edit"].toString();
        frame.room_edits.push_back(MathUtil::DecodeString(m["room_edit"].toString()));
        frame.editing = true;
    }

    if (m.contains("room_delete")) {
//        qDebug() << "AssetGhost::ConvertPacketToFrame - INCOMING ROOM DELETE" << MathUtil::DecodeString(m["room_delete"].toString());
        frame.room_deletes.push_back(MathUtil::DecodeString(m["room_delete"].toString()));
    }

    if (m.contains("hand0")) {
        QVariantMap hand_map = m["hand0"].toMap();
        frame.hands.first.SetJSON( hand_map);
        frame.hands.first.is_active = true;
    }

    if (m.contains("hand1")) {
        QVariantMap hand_map = m["hand1"].toMap();
        frame.hands.second.SetJSON( hand_map);
        frame.hands.second.is_active = true;
    }

    //we should make the assetghost from the custom data, if we have it
    if (m.contains("avatar")) {
        frame.avatar_data = MathUtil::DecodeString(m["avatar"].toString());
    }
}

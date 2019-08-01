#ifndef ASSETGHOST_H
#define ASSETGHOST_H

#include <QtNetwork>
#include <QtConcurrent/QtConcurrent>

#include "asset.h"
#include "webasset.h"
#include "leaphands.h"
#include "audioutil.h"

struct GhostFrame
{
    GhostFrame() :
        time_sec(0.0f),
        pos(0,0,0),
        dir(0,0,1),        
        cursor_active(false),        
        cscale(1.0f),
        current_sound_level(0.0f),
        speaking(false),
        typing(false),
        editing(false)
    {
    }

    void SetHeadXForm(const QVector3D & y, const QVector3D & z);

    float time_sec;
    QVector3D pos;
    QVector3D dir; //torso related

    QMatrix4x4 head_xform; //head

    QString hmd_type; //head-related    

    bool cursor_active;
    QMatrix4x4 cursor_xform; //cursor
    float cscale;

    float current_sound_level;
    bool speaking;
    bool typing;
    bool editing;

    QPair <LeapHand, LeapHand> hands; //hands-related

    QString chat_message;

    QString send_portal_url;
    QString send_portal_jsid;
    QVector3D send_portal_pos;
    QVector3D send_portal_fwd;

    QString anim_id;
    QList <QByteArray> sound_buffers;

    QString roomid;
    QString userid;

    QString avatar_data;

    QList <QString> room_edits;
    QList <QString> room_deletes;
};

class AssetGhost : public Asset
{
    Q_OBJECT

public:

    AssetGhost();
    virtual ~AssetGhost();   

    void Load();
    void Unload();

    void SetFromFrames(const QVector <GhostFrame> & ghost_frames, const float secs_between_frames);

    void Update();

    int GetNumFrames() const;
    int GetFrameIndex(const float time_sec);
    GhostFrame & GetFrameByIndex(const int i);
    bool GetGhostFrame(const float time_sec, GhostFrame & frame);   
    void ClearEditsDeletes();
    void ClearFrames();

    //this could be very useful everywhere
    static void ConvertPacketToFrame(const QVariantMap & m, GhostFrame & frame);

private:

    void LoadDataThread();

    float secs_per_frame;
    QVector <GhostFrame> frames;
};

#endif // ASSETGHOST_H

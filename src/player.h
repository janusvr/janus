#ifndef PLAYER_H
#define PLAYER_H

#include <QtGui>

#include "domnode.h"
#include "settingsmanager.h"
#include "assetshader.h"
#include "leaphands.h"

class Player : public QObject
{
    Q_OBJECT

public:

    Player();   

    void SetProperties(QPointer <DOMNode> props);
    QPointer <DOMNode> GetProperties();   

    void DrawLeapHandsGL(QPointer <AssetShader> shader);

    void SetViewGL(const bool render_left_eye, const float eye_ipd, const QMatrix4x4 xform, const bool no_pitch = false );

    void UpdateDir();
    void UpdateEyePoint();

    QVector3D GetRightDir() const;   

    QMatrix4x4 GetTransform() const;

    void SetCursorActive(const bool b, const int cursor_index);
    bool GetCursorActive(const int cursor_index) const;
    void SetCursorObject(const QString & s, const int cursor_index);
    QString GetCursorObject(const int cursor_index) const;
    void SetCursorPos(const QVector3D & p, const int cursor_index);
    QVector3D GetCursorPos(const int cursor_index) const;
    void SetCursorXDir(const QVector3D & p, const int cursor_index);
    QVector3D GetCursorXDir(const int cursor_index) const;
    void SetCursorYDir(const QVector3D & p, const int cursor_index);
    QVector3D GetCursorYDir(const int cursor_index) const;
    void SetCursorZDir(const QVector3D & p, const int cursor_index);
    QVector3D GetCursorZDir(const int cursor_index) const;
    void SetCursorScale(const float f, const int cursor_index);
    float GetCursorScale(const int cursor_index) const;

    float ComputeCursorScale(const int cursor_index) const;   

    QTime & GetComfortTimer();      

    void SpinView(const float f, const bool scale_rotation_speed);
    void TiltView(const float f);    

    bool GetWalking() const;

    void DoSpinLeft(const bool b);
    void DoSpinRight(const bool b);

    QVector3D GetViewToWorldPoint(const QVector3D & p) const;
    QVector3D GetViewToWorldDir(const QVector3D & d) const;

    void ResetCentre();

    void Update(const float move_speed);

    QPair <LeapHand, LeapHand> GetHands() const;
    LeapHand & GetHand(const int i);
    const LeapHand & GetHand(const int i) const;

    float GetScaleVelX() const;
    void SetScaleVelX(float value);

    float GetScaleVelY() const;
    void SetScaleVelY(float value);

    bool GetJump() const;
    void SetJump(bool value);

    float GetTheta() const;
    void SetTheta(float value);

    float GetPhi() const;
    void SetPhi(float value);

    bool GetSpinLeft() const;
    void SetSpinLeft(bool value);

    bool GetSpinRight() const;
    void SetSpinRight(bool value);

    bool GetFlyUp() const;
    void SetFlyUp(bool value);

    bool GetFlyDown() const;
    void SetFlyDown(bool value);

    float GetCorrectionTheta() const;
    void SetCorrectionTheta(float value);

    bool GetHMDCalibrated() const;
    void SetHMDCalibrated(bool value);

    bool GetWalkForward() const;
    void SetWalkForward(bool value);

    bool GetWalkLeft() const;
    void SetWalkLeft(bool value);

    bool GetWalkRight() const;
    void SetWalkRight(bool value);

    bool GetWalkBack() const;
    void SetWalkBack(bool value);

    bool GetFollowMode() const;
    void SetFollowMode(bool value);

    QString GetFollowModeUserID() const;
    void SetFollowModeUserID(const QString &value);

    void SetHMDType(const QString s);
    QString GetHMDType();

    void SetDeltaTime(const float f);
    inline float GetDeltaTime() const { return delta_time; }

    void SetRecording(const bool b);
    inline bool GetRecording() const { return recording; }

    void SetRunning(const bool b);
    inline bool GetRunning() const { return running; }

    bool GetFlying() const;
    void SetFlying(bool value);

    bool GetHMDEnabled() const;
    void SetHMDEnabled(bool value);

    QVector3D GetImpulseVel() const;
    void SetImpulseVel(const QVector3D &value);

    bool GetSpeaking() const;
    void SetSpeaking(bool value);

    bool GetTyping() const;
    void SetTyping(bool value);

    bool GetEnteringText() const;
    void SetEnteringText(bool value);

    QString GetDeviceType();
    void SetDeviceType(const QString &value);

    float GetPlayerCollisionRadius() const;
    void SetPlayerCollisionRadius(float value);

private:

    QPointer <QObject> CreateControllerQObject();

    QPointer <DOMNode> props;

    float theta;
    float phi;
    float correction_theta;
    bool walk_forward;
    bool walk_left;
    bool walk_right;
    bool walk_back;
    bool fly_up;
    bool fly_down;
    bool spin_left;
    bool spin_right;
    bool follow_mode;
    QString follow_mode_userid;
    bool comfort_spinleft;
    bool comfort_spinright;
    QTime comfort_timer;      
    QTime time;
    bool flytransition;    
    float flyduration;
    bool hmd_calibrated;

    QVector3D hand0_pos_old;
    QVector3D hand1_pos_old;
    float hand_update_time;

    float cursor0_scale;
    float cursor1_scale;

    float scale_velx;
    float scale_vely;

    bool jump;
    bool speaking_no_mic;

    float delta_time;
    bool speaking;
    bool typing;
    bool entering_text;
    bool recording;
    bool running;
    bool flying;
    bool walking;
    bool hmd_enabled;
    QVector3D impulse_vel;
    float player_collision_radius;

    QPair <LeapHand, LeapHand> hands;  
};

#endif // PLAYER_H

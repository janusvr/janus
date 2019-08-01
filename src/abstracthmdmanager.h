#ifndef HMDMANAGER_H
#define HMDMANAGER_H

#include <QObject>
#include <QMatrix4x4>

class AbstractHMDManager : public QObject
{
public:

    AbstractHMDManager();
    ~AbstractHMDManager();

    virtual bool Initialize() = 0;
    virtual void InitializeGL() = 0;

    virtual bool GetEnabled() const = 0;
    virtual QSize GetTextureSize() const = 0;
    virtual float GetIPD() const = 0;
    virtual QString GetHMDString() const = 0;
    virtual QString GetHMDType() const = 0;

    virtual void EnterVR() = 0;
    virtual void ExitVR() = 0;
    virtual void Update() = 0;
	virtual void PostPresent() = 0;

    virtual QMatrix4x4 GetHMDTransform() const = 0;
    virtual QMatrix4x4 GetControllerTransform(const int i) const = 0;
    virtual QMatrix4x4 GetLastControllerTransform(const int i) const = 0;

    virtual int GetNumControllers() const = 0;
    virtual bool GetControllerTracked(const int i) = 0;

    virtual QVector2D GetControllerThumbpad(const int i) const = 0;
    virtual bool GetControllerThumbpadTouched(const int i) const = 0;
    virtual bool GetControllerThumbpadPressed(const int i) const = 0;

    virtual QVector2D GetControllerStick(const int i) const = 0;
    virtual bool GetControllerStickTouched(const int i) const = 0;
    virtual bool GetControllerStickPressed(const int i) const = 0;

    virtual float GetControllerTrigger(const int i) const = 0;
    virtual float GetControllerGrip(const int i) const = 0;
    virtual bool GetControllerMenuPressed(const int i) = 0;

    virtual bool GetControllerButtonAPressed() const = 0;
    virtual bool GetControllerButtonBPressed() const = 0;
    virtual bool GetControllerButtonXPressed() const = 0;
    virtual bool GetControllerButtonYPressed() const = 0;

    virtual bool GetControllerButtonATouched() const = 0;
    virtual bool GetControllerButtonBTouched() const = 0;
    virtual bool GetControllerButtonXTouched() const = 0;
    virtual bool GetControllerButtonYTouched() const = 0;

    virtual void BeginRenderEye(const int eye) = 0;
    virtual void EndRenderEye(const int eye) = 0;

    virtual void BeginRendering() = 0;
    virtual void EndRendering() = 0;

    virtual void ReCentre() = 0;

    void SetNearDist(const float f, bool const p_is_avatar);
    float GetNearDist(bool const p_is_avatar) const;
    void SetFarDist(const float f, bool const p_is_avatar);
    float GetFarDist(bool const p_is_avatar) const;

    virtual void TriggerHapticPulse(const int i, const int val) = 0;

    //Platform specific (e.g. OVR Platform)
    virtual void Platform_ProcessMessages() = 0;
    virtual bool Platform_GetEntitled() const = 0;
    virtual bool Platform_GetShouldQuit() const = 0;

    const QMatrix4x4& GetEyeViewMatrix(const int p_eye_index) const;
    const QMatrix4x4& GetEyeProjectionMatrix(const int p_eye_index, const bool p_is_avatar) const;

    bool m_using_openVR;
    unsigned int m_color_texture_id;
    QVector<QVector4D> m_eye_viewports;

protected:

    float m_near_clip;
    float m_far_clip;
    float m_avatar_near_clip;
    float m_avatar_far_clip;
    QVector<QMatrix4x4> m_eye_view_matrices;
    QVector<QMatrix4x4> m_eye_projection_matrices;
};

#endif // HMDMANAGER_H

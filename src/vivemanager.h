#ifndef VIVERENDERER_H
#define VIVERENDERER_H

#include <QtCore>
#include <QtGui>
#include <openvr.h>

#include "rendererinterface.h"
#include "mathutil.h"
#include "abstracthmdmanager.h"

class ViveManager : public AbstractHMDManager
{

public:

    ViveManager();
    ~ViveManager();

    bool Initialize();
    void InitializeGL();

    bool GetEnabled() const;
    QSize GetTextureSize() const;
    float GetIPD() const;
    void SetHMDString(const QString s);
    QString GetHMDString() const;
    QString GetHMDType() const;

    void EnterVR();
    void ExitVR();
    void Update();

    QMatrix4x4 GetHMDTransform() const;
    QMatrix4x4 GetControllerTransform(const int i) const;
    QMatrix4x4 GetLastControllerTransform(const int i) const;

    void BeginRenderEye(const int eye);
    void EndRenderEye(const int eye);

    void BeginRendering();
    void EndRendering();

	void PostPresent();
	void ReCentre();

    int GetNumControllers() const;
    bool GetControllerActive(const int i);
    bool GetControllerTracked(const int i);

    QVector2D GetControllerThumbpad(const int i) const;
    bool GetControllerThumbpadTouched(const int i) const;
    bool GetControllerThumbpadPressed(const int i) const;

    QVector2D GetControllerStick(const int i) const;
    bool GetControllerStickTouched(const int i) const;
    bool GetControllerStickPressed(const int i) const;

    float GetControllerTrigger(const int i) const;
    float GetControllerGrip(const int i) const;
    bool GetControllerMenuPressed(const int i);

    bool GetControllerButtonAPressed() const;
    bool GetControllerButtonBPressed() const;
    bool GetControllerButtonXPressed() const;
    bool GetControllerButtonYPressed() const;

    bool GetControllerButtonATouched() const;
    bool GetControllerButtonBTouched() const;
    bool GetControllerButtonXTouched() const;
    bool GetControllerButtonYTouched() const;

    void TriggerHapticPulse(const int i, const int val);   

    void Platform_ProcessMessages();
    bool Platform_GetEntitled() const;
    bool Platform_GetShouldQuit() const;

private:

    QMatrix4x4 ConvertSteamVRMatrixToQMatrix4x4(const vr::HmdMatrix34_t & matPose) const;
    QString GetTrackedDeviceString( vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL ) const;
    void Shutdown();

    QString hmd_string;
    bool enabled;

    vr::IVRSystem * m_pHMD;
    vr::IVRCompositor *m_pCompositor;

    QMatrix4x4 m_mat4HMDPose;

    char m_rDevClassChar[ vr::k_unMaxTrackedDeviceCount ];
    vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
    QMatrix4x4 m_rmat4DevicePose[ vr::k_unMaxTrackedDeviceCount ];    
    QMatrix4x4 last_m_rmat4DevicePose[ vr::k_unMaxTrackedDeviceCount ];

    int m_iTrackedControllerCount;
    int m_iTrackedControllerCount_Last;
    int m_iValidPoseCount;
    int m_iValidPoseCount_Last;
    QString m_strPoseClasses;

    uint32_t m_nRenderWidth;
    uint32_t m_nRenderHeight;   

    vr::TrackedDeviceIndex_t controller_ind_left;
    vr::TrackedDeviceIndex_t controller_ind_right;
    vr::VRControllerState_t controller_states[2];
    vr::TrackedDevicePose_t controller_poses[2];    

    vr::TrackingUniverseOrigin tracking_origin;
    QMatrix4x4 depth_invert_matrix;
    float viewport_width_modifier;
};

#endif // VIVERENDERER_H

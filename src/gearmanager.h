#ifndef GEARRENDERER_H
#define GEARRENDERER_H

#include <VrApi.h>
#include <VrApi_Version.h>
#include <VrApi_Types.h>
#include <VrApi_Ext.h>
#include <VrApi_Helpers.h>
#include <VrApi_Input.h>
#include <VrApi_SystemUtils.h>

#include "OVR_Platform.h"

#include <jni.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <QtAndroidExtras/QtAndroidExtras>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "mathutil.h"
#include "rendererinterface.h"
#include "abstracthmdmanager.h"
#include "jniutil.h"

#include <QtPlatformHeaders/QEGLNativeContext>
#include <QOpenGLContext>
#include <unistd.h>

#define MAX_FENCES 4

typedef struct
{
    GLsync		Sync;
} ovrFence;

//class MainWindow;

class GearManager : public AbstractHMDManager
{

public:

    GearManager();
    ~GearManager();

    bool Initialize();
	void PostPresent();
	void InitializeGL();

    bool GetEnabled() const;
    QSize GetTextureSize() const;
    float GetIPD() const;
    QString GetHMDString() const;
    QString GetHMDType() const;

    void EnterVR();
    void ExitVR();
    void Update();

    QMatrix4x4 GetHMDTransform() const;
    QMatrix4x4 GetControllerTransform(const int i) const;
    QMatrix4x4 GetLastControllerTransform(const int i) const;

    int GetNumControllers() const;    
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

    void BeginRenderEye(const int eye);
    void EndRenderEye(const int eye);

    void BeginRendering();
    void EndRendering();

    void ReCentre();   

    void TriggerHapticPulse(const int, const int);

    //OVR Platform
    void Platform_ProcessMessages();
    bool Platform_GetEntitled() const;
    bool Platform_GetShouldQuit() const;

    void ShowQuitMenu();

private:

    QMatrix4x4 GetMatrixFromPose(const ovrPosef & pose, const bool flip_z);
    QMatrix4x4 GetProjectionMatrix(const int eye, const bool p_is_avatar);
    QMatrix4x4 OVRMatrixToMatrix(ovrMatrix4f m);

    ovrJava java;
    ovrTextureSwapChain *colorTextureSwapChain[VRAPI_FRAME_LAYER_EYE_MAX];
    ovrMobile * ovr;
    ovrFence fences[MAX_FENCES];

    double predicted_display_time;
    int fence_index;

    QSize idealTextureSize;
    QOpenGLFramebufferObject * eye_FBOs[1];

    bool initialized;
    bool gl_initialized;
    bool enter_vr;
    bool exit_vr;

    long long frame_index;

    QMatrix4x4 hmd_xform;
    QMatrix4x4 last_hmd_xform;
    QMatrix4x4 controller_xform;
    QMatrix4x4 last_controller_xform;

    ovrLayerProjection2 layer;
    ovrTracking2 tracking;

    jobject surfaceRef;
    ANativeWindow* surface;
    jobject m_objectRef;

    bool showing_vr;
    bool paused;

    bool controller_tracked;
    bool head_controller_tracked;

    QVector2D head_max_thumbpad;
    bool head_menu_pressed;
    bool head_thumbpad_pressed; //Trackpad clicked
    bool head_thumbpad_hovered;
    QVector2D head_thumbpad;

    QVector2D max_thumbpad;
    bool menu_pressed;
    bool thumbpad_pressed; //Trackpad clicked
    bool thumbpad_hovered;
    QVector2D thumbpad;
    bool trigger_pressed; //Trigger pressed

    uint8_t last_recenter_count;

    bool entitled;
};

#endif // GEARRENDERER_H

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QVariantMap>

#include "mathutil.h"

class SettingsManager
{
public:

    SettingsManager();

    static void LoadSettings();
    static void SaveSettings();   

    static bool GetMultiplayerEnabled();    
    static bool GetPartyModeEnabled();
    static bool GetSelfAvatar();
    static qint64 GetDeallocateAmount();
    static bool GetComfortMode();
    static bool GetHapticsEnabled();
    static bool GetCacheEnabled();
    static bool GetAntialiasingEnabled();
    static bool GetEnhancedDepthPrecisionEnabled();
    static bool GetDecoupleHeadEnabled();
    static void SetSoundsEnabled(const bool b);
    static bool GetSoundsEnabled();
    static float GetVolumeEnv();
    static float GetVolumeVOIP();
    static float GetVolumeMic();
    static float GetMicSensitivity();
    static void SetMicAlwaysOn(const bool b);
    static bool GetMicAlwaysOn();
    static bool GetPositionalEnvEnabled();
    static bool GetPositionalVOIPEnabled();
    static bool GetShadersEnabled();
    static bool GetAssetImagesEnabled();
    static bool GetInvertYEnabled();
    static bool GetLeapOnHMDEnabled();
    static bool GetEditModeEnabled();
    static bool GetEditModeIconsEnabled();
    static bool GetCrosshairEnabled();
    static bool GetGamepadEnabled();
    static bool GetPortalHotkeys();
    static void SetServer(const QString s);
    static QString GetServer();
    static void SetPort(const int i);
    static int GetPort();
    static int GetRate();
    static float GetIPD();
    static bool GetHaptics();
    static bool GetPerfLog();
    static QString GetPlaybackDevice();
    static QString GetCaptureDevice();
    static bool GetViveTrackpadMovement();
    static QString GetHomeURL();
    static QString GetWebsurfaceURL();
    static bool GetRenderPortalRooms();

    static void SetLaunchURL(const QString & s);
    static QString GetLaunchURL();

    static void SetMousePitchEnabled(const bool b);
    static bool GetMousePitchEnabled();

    static float GetRotationSpeed();
    static float GetFOV();

    static bool GetUpdateWebsurfaces();
    static bool GetUpdateVOIP();
    static bool GetUpdateCMFT();
    static bool GetUpdateCustomAvatars();  

    static QVariantMap settings;
};

#endif // SETTINGSMANAGER_H

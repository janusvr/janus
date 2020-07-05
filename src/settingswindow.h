#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QtGui>
#include <QtWidgets>

#include "game.h"

class SettingsWindow  : public QMainWindow
{
    Q_OBJECT
public:
    SettingsWindow(Game * g);
    void closeEvent(QCloseEvent *event);

public slots:

    void Update();

    void SlotSetUserID();
    void SlotSetCustomPortalShader();
    void SlotSetEditMode();
    void SlotSetEditModeIcons();
    void SlotSetUpdateWebsurfaces();
    void SlotSetUpdateCMFT();
    void SlotSetUpdateVOIP();
    void SlotSetUpdateCustomAvatars();
    void SlotSetUpdateAssetImages();
    void SlotSetCrosshair();
    void SlotSetFOV();
    void SlotSetSelfAvatar();
    void SlotSetPartyMode();    
    void SlotSetMultiplayer();
    void SlotSetHomeURL();
    void SlotSetWebsurfaceURL();
    void SlotSetDownloadCache();
    void SlotSetComfortMode();
    void SlotSetHaptics();
    void SlotSetAntialiasing();
    void SlotSetEnhancedDepthPrecision();
    void SlotSetGamepad();
    void SlotSetMicVoiceActivated();
    void SlotSetMicSensitivity();
    void SlotSetVolumeMic();
    void SlotSetVolumeEnv();
    void SlotSetPositionalEnv();
    void SlotSetVolumeVoip();
    void SlotSetPositionalVoip();
    void SlotSetInvertPitch();
    void SlotResetAvatar();
    void SlotReloadAvatar();
    void SlotSetViveTrackpadMovement();
    void SlotSetRenderPortalRooms();

private:

    QWidget * GetAudioWidget();
    QWidget * GetAvatarWidget();
    QWidget * GetDeveloperWidget();
    QWidget * GetInterfaceWidget();
    QWidget * GetNetworkWidget();
    QWidget * GetVRWidget();

    Game * game;

    QLineEdit * lineedit_userid;
    QLineEdit * lineedit_customportalshader;
    QCheckBox * checkbox_editmode;
    QCheckBox * checkbox_editmodeicons;
    QCheckBox * checkbox_updatewebsurfaces;
    QCheckBox * checkbox_updatevoip;
    QCheckBox * checkbox_updatecmft;
    QCheckBox * checkbox_updatecustomavatars;
    QCheckBox * checkbox_updateassetimages;
    QCheckBox * checkbox_crosshair;
    QSlider * slider_fov;
    QCheckBox * checkbox_gamepad;
    QCheckBox * checkbox_micvoiceactivated;
    QLabel * label_microphonesensitivity;
    QSlider * slider_microphonesensitivity;
    QSlider * slider_volumemic;
    QSlider * slider_volumeenv;
    QSlider * slider_volumevoip;
    QCheckBox * checkbox_positionalenv;
    QCheckBox * checkbox_positionalvoip;
    QCheckBox * checkbox_invertpitch;
    QCheckBox * checkbox_partymode;
    QCheckBox * checkbox_selfavatar;    
    QCheckBox * checkbox_multiplayer;
    QCheckBox * checkbox_downloadcache;
    QCheckBox * checkbox_comfortmode;
    QCheckBox * checkbox_haptics;
    QCheckBox * checkbox_antialiasing;
    QCheckBox * checkbox_enhanceddepthprecision;
    QPushButton * button_resetavatar;
    QPushButton * button_reloadavatar;
    QCheckBox * checkbox_vivetrackpadmovement;
    QLineEdit * lineedit_homeurl;
    QLineEdit * lineedit_websurfaceurl;
    QCheckBox * checkbox_renderportalrooms;

    QTabWidget * tab_widget;
};

#endif // SETTINGSWINDOW_H

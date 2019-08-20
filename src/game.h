#ifndef GAME_H
#define GAME_H

#include <QtGui>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>

#include "environment.h"
#include "player.h"
#include "soundmanager.h"
#include "textgeom.h"
#include "assetobject.h"
#include "assetghost.h"
#include "assetscript.h"
#include "multiplayermanager.h"
#include "menu.h"
#include "virtualmenu.h"
#include "bookmarkmanager.h"
#include "controllermanager.h"
#include "filteredcubemapmanager.h"
#include "soundmanager.h"
#include "settingsmanager.h"

enum FadeState {
    FADE_NONE,    
    FADE_RELOAD1,
    FADE_RELOAD2,
    FADE_RESETPLAYER1,
    FADE_RESETPLAYER2,
    FADE_FORWARD_PLAYER1,
    FADE_FORWARD_PLAYER2,
    FADE_TELEPORT1,
    FADE_TELEPORT2
};

enum JVR_GameState {
    JVR_STATE_DEFAULT, //pressed nothing
    JVR_STATE_INTERACT_TELEPORT,
    JVR_STATE_EDIT_TEXT,
    JVR_STATE_SELECT_ASSET,
    JVR_STATE_DRAGDROP,
    JVR_STATE_UNIT_TRANSLATE,
    JVR_STATE_UNIT_ROTATE,
    JVR_STATE_UNIT_SCALE,
    JVR_STATE_UNIT_COLOUR,
    JVR_STATE_UNIT_COLLISIONID,
    JVR_STATE_UNIT_COLLISIONSCALE,
    JVR_STATE_UNIT_LIGHTING,
    JVR_STATE_UNIT_CULL_FACE,
    JVR_STATE_UNIT_BLEND_SRC,
    JVR_STATE_UNIT_BLEND_DEST,
    JVR_STATE_UNIT_MIRROR
};

struct PrivateWebsurface
{
    QPointer <AssetWebSurface> asset;
    QPointer <AssetObject> plane_obj;
    QPointer <RoomObject> obj;    
};

class Game : public QObject
{

    Q_OBJECT

public:

    Game();
    ~Game();

    void initializeGL();

    void mousePressEvent(QMouseEvent * e, const int cursor_index, const QSize p_windowSize = QSize(0,0), const QPointF p_mousePos = QPointF(0,0));
    void mouseMoveEvent(QMouseEvent * e, const float x, const float y, const int cursor_index, const QSize p_windowSize = QSize(0,0), const QPointF p_mousePos = QPointF(0,0));
    void mouseReleaseEvent(QMouseEvent * e, const int cursor_index, const QSize p_windowSize = QSize(0,0), const QPointF p_mousePos = QPointF(0,0));
    void wheelEvent(QWheelEvent * e);
    void keyPressEvent(QKeyEvent * e);
    void keyReleaseEvent(QKeyEvent * e);

    QPointer <Environment> GetEnvironment();
    QPointer <VirtualMenu> GetVirtualMenu();
    QPointer <Player> GetPlayer();
    QPointer <MultiPlayerManager> GetMultiPlayerManager();
    QPointer <ControllerManager> GetControllerManager();
    QPointer <BookmarkManager> GetBookmarkManager();
    MenuOperations & GetMenuOperations();

    void ResetCursor(const QPoint p);

    void SetMouseDoPitch(const bool b);
    bool GetMouseDoPitch();

    bool GetPlayerEnteringText();
    void EndKeyboardFocus();

    void DrawGL(const float ipd, const QMatrix4x4 head_xform, const bool set_modelmatrix, const QSize p_windowSize, const QPointF p_mousePos);

    void ResetPlayer();
    void TeleportPlayer();

    void RenameJSID(const QString & old_js_id, const QString & new_js_id);

    float GetCurrentNearDist();
    float GetCurrentFarDist(); 

    void SetDrawCursor(const bool b);

    void SetState(const JVR_GameState & s);
    JVR_GameState GetState() const;

    void ToggleSoundEnabled();

    void Initialize();

    void AddPrivateWebsurface();
    void RemovePrivateWebsurface();
    void SetPrivateWebsurfacesVisible(const bool b);
    bool GetPrivateWebsurfacesVisible() const;
    void DrawPrivateWebsurfacesGL(QPointer <AssetShader> shader);
    void UpdatePrivateWebsurfaces();

    void Update();

    void UpdateMultiplayer();

    float UpdateCursorRaycast(const QMatrix4x4 transform, const int cursor_index);
    QMatrix4x4 GetMouseCursorTransform() const;
    void ComputeMouseCursorTransform(const QSize p_windowSize, const QPointF p_mousePos);      

    QString GetVersion() const;    
    void SetOnlineVersion(const QString & s);      

    bool GetAllowTeleport(const int cursor_index);

    void StartResetPlayer();
    void StartResetPlayerForward();
    void StartTeleportPlayer();
    void StartReloadPortal();
    void StartReinitializeEnvironment();
    void SetDoExit(const bool b);
    bool GetDoExit() const;

    QString GetGlobalUUID();   

    QPointer<AssetWebSurface> GetWebSurfaceSelected();

    bool SetSelected(QPointer <Room> room, const QString & selected, const bool b);
    QString GetSelected(const int i);   

    void SetGamepadButtonPress(const bool b);

    void SaveScreenThumb(const QString out_filename);

    void SetWindowSize(const QSize s);
    void SaveBookmark();

    QString GetRoomScreenshotPath(QPointer <Room> r);

    //used for MainWindow UI interop
    void CreateNewWorkspace(const QString name);
    QPointer <RoomObject> CreatePortal(const QUrl url, const bool send_multi);
    void SaveRoom(const QString out_filename);

    void SendChatMessage(const QString s);    

    QPointer <Asset> CreateAssetFromURL(const QString url);
    void DragAndDropFromWebsurface(const QString drop_or_pin, const int cursor_index);
    void DragAndDropAssetObject(const QString id, const int i); //from asset palette in AssetWindow
    void DragAndDrop(const QString url_str, const QString drop_or_pin, const int i);
    void UpdateDragAndDropPosition(QPointer <RoomObject> o, const int cursor_index);

    bool GetRecording() const;
    void StartRecording(const bool record_everyone);
    void StopRecording();   

    void SetUserID(const QString s);
    void ResetAvatar();

    void SetRoomDeleteCode(const QString s);

    void DoImport(const QString url);

private:   

    //Update method broken up into these methods for each specific task
    void UpdateCursorAndTeleportTransforms();
    void UpdateFollowMode();
    void UpdateImportList();
    void UpdateControllers();    
    void UpdateAssetRecordings();
    void UpdateAudio();
    void UpdateAssets();    

    void DrawCursorGL();
    void DrawFadingGL();

    void UpdateOverlays();
    void DrawOverlaysGL();

    void EditModeTranslate(QPointer <RoomObject> obj, const int x, const int y, const int z);
    void EditModeRotate(QPointer <RoomObject> obj, const int x, const int y, const int z);
    void EditModeScale(QPointer <RoomObject> obj, const int x, const int y, const int z);
    void EditModeCollisionScale(QPointer <RoomObject> obj, const int x, const int y, const int z);   

    void ClearSelection(const int cursor_index);   

    void UpdateVirtualMenu();
    void DrawVirtualMenu();   

    //interact ops   
    void StartOpInteractionDefault(const int i);
    void EndOpInteractionDefault(const int i);

    void StartOpInteractionTeleport(const int i);
    void EndOpInteractionTeleport(const int i);

    void StartOpSpatialControllerEdit(const int i);
    void EndOpSpatialControllerEdit(const int i);

    float controller_x[2];
    float controller_y[2];
    float last_controller_x[2];
    float last_controller_y[2];          

    QMap<int, bool> keys;    

    bool show_position_mode;

    QTime teleport_held_time;
    int teleport_hold_time_required;

    QString selected[2];
    QString copy_selected;
    QPointer <AssetVideo> video_selected[2];
    QPointer <AssetWebSurface> websurface_selected[2];
    QPointer <RoomObject> undo_object[2];

    QSize win_size;
    QPointF last_cursor_win;
    QPointF cursor_win;
    QPointF mouse_move_accum;
    QPointF cursor_uv[2];

    JVR_GameState state;

    int unit_scale;
    QVector <float> unit_translate_amount;
    QVector <float> unit_rotate_amount;
    QVector <float> unit_scale_amount;

    QPointer <Player> player;
    QPointer <Environment> env;
    QPointer <MultiPlayerManager> multi_players;
    QPointer <VirtualMenu> virtualmenu;    
    QPointer <BookmarkManager> bookmarks;
    QPointer <ControllerManager> controller_manager;

    MenuOperations menu_ops;

    bool do_exit;
    QTime fade_time;
    FadeState fadestate;     

    QPointer <RoomObject> teleport_portal;
    QVector3D teleport_p;
    QVector3D teleport_x;
    QVector3D teleport_y;
    QVector3D teleport_z;

    TextGeom info_text_geom;
    TextGeom info2_text_geom;

    unsigned int global_uuid;

    bool spatial_active[2];
    QMatrix4x4 spatial_basetransform[2];
    QMatrix4x4 spatial_reltransform[2];
    QMatrix4x4 spatial_ctrltransform[2];
    QString spatial_collid[2];

    float spatial_startf[2];

    QMatrix4x4 mouse_cursor_transform;

    bool gamepad_button_press;   

    QMatrix4x4 dragdrop_xform;

    QList <PrivateWebsurface> private_websurfaces;

    QList <QPointer <WebAsset> > import_list; //contains list of queued HTML files to load containing JML

    bool draw_cursor;
    int cursor_active;
    bool rmb_held;
    QTime rmb_held_time;

    static QEvent::Type keypress;

    QTime deltat_time;
    float delta_time;
};

#endif // GAME_H

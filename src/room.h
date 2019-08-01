#ifndef ROOM_H
#define ROOM_H

#include <QtCore>
#include <QtScript>

#include "assetscript.h"
#include "assetskybox.h"
#include "soundmanager.h"
#include "roomtemplate.h"
#include "roomobject.h"
#include "player.h"
#include "multiplayermanager.h"
#include "htmlpage.h"
#include "domnode.h"
#include "lightmanager.h"
#include "filteredcubemapmanager.h"
#include "roomphysics.h"
#include "performancelogger.h"
#include "assetwebsurface.h"

//59.7 - note: circ dep with these classes
class AssetObject;
class AssetScript;
class AssetSkybox;
class AssetVideo;
class AssetWebSurface;
class MultiPlayerManager;

class RoomTemplate;
class RoomPhysics;

class Room : public QObject
{
    Q_OBJECT

public:

    Room();    
    ~Room();   

    static void initializeGL();
    void Clear();

    bool HasJSFunctionContains(const QString & s, const QString & code);    
    void CallJSFunction(const QString & s, QPointer <Player> player, MultiPlayerManager * multi_players, QList <QPointer <DOMNode> > nodes = QList <QPointer <DOMNode> >());

    void SetProperties(const QVariantMap & d);
    void SetProperties(QPointer <DOMNode> props);
    QPointer <DOMNode> GetProperties();   

    void SetRoomTemplate(const QString & name);
    QPointer <RoomTemplate> GetRoomTemplate() const;

    QString GetSaveFilename() const;

    void SetAssetShader(const QPointer <AssetShader> a);
    QPointer <AssetShader> GetAssetShader();

    void AddAsset(const QString asset_type, const QVariantMap & property_list, const bool do_sync = false);        

    void AddNewAssetScript();

    void AddAssetGhost(QPointer <AssetGhost> a);
    void AddAssetImage(QPointer <AssetImage> a);
    void AddAssetObject(QPointer <AssetObject> a);
    void AddAssetRecording(QPointer <AssetRecording> a);
    void AddAssetScript(QPointer <AssetScript> a);
    void AddAssetShader(QPointer <AssetShader> a);
    void AddAssetSound(QPointer <AssetSound> a);
    void AddAssetVideo(QPointer <AssetVideo> a);    
    void AddAssetWebSurface(QPointer<AssetWebSurface> a);

    void RemoveAsset(QPointer <Asset> a);

    QList <QPointer <Asset> > GetAllAssets();

    QHash <QString, QPointer <AssetGhost> > & GetAssetGhosts();
    QHash <QString, QPointer <AssetImage> > & GetAssetImages();
    QHash <QString, QPointer <AssetObject> > & GetAssetObjects();
    QHash <QString, QPointer <AssetRecording> > & GetAssetRecordings();
    QHash <QString, QPointer <AssetScript> > & GetAssetScripts();
    QHash <QString, QPointer <AssetShader> > & GetAssetShaders();
    QHash <QString, QPointer <AssetSound> > & GetAssetSounds();
    QHash <QString, QPointer <AssetVideo> > & GetAssetVideos();    
    QHash<QString, QPointer<AssetWebSurface> > & GetAssetWebSurfaces();

    QPointer <Asset> GetAsset(const QString id) const;
    QPointer <AssetGhost> GetAssetGhost(const QString id) const;
    QPointer <AssetImage> GetAssetImage(const QString id) const;
    QPointer <AssetObject> GetAssetObject(const QString id) const;
    QPointer <AssetRecording> GetAssetRecording(const QString id) const;
    QPointer <AssetScript> GetAssetScript(const QString id) const;
    QPointer <AssetShader> GetAssetShader(const QString id) const;
    QPointer <AssetSound> GetAssetSound(const QString id) const;
    QPointer <AssetVideo> GetAssetVideo(const QString id) const;    
    QPointer <AssetWebSurface> GetAssetWebSurface(const QString id) const;

    QString AddRoomObject(QPointer <RoomObject> o);
    void AddRoomObjects(QList <QPointer <RoomObject> > & objects);

    bool GetMountPoint(QVector3D & pos, QVector3D & dir);
    int GetNumMountPointsFree() const;

    QPointer <RoomObject> GetRoomObject(const QString js_id);
    QHash <QString, QPointer <RoomObject> > & GetRoomObjects();

    void DoEdit(const QString & s);
    void DoDelete(const QString & s);

    bool DeleteSelected(const QString & selected, const bool do_sync=true, const bool play_delete_sound=true);
    QString PasteSelected(const QString & selected, const QVector3D & p, const QVector3D & x, const QVector3D & y, const QVector3D & z, const QString & js_id);
    void EditText(const QString & selected, const QString & c, const bool backspace);   

    void SelectAssetForObject(const QString & selected, const int offset);
    void SelectAssetForObject(const QString & selected, const QString & new_id);
    void SelectCollisionAssetForObject(const QString & selected, const QString & new_coll_id);

    QString GetSelectedCode(const QString & selected) const;

    void DrawGL(MultiPlayerManager* multi_players, QPointer <Player> player, const bool render_left_eye, const bool draw_player, const bool draw_portal_decorations);

    void SetPlayerPosTrans(const QVector3D p);
    void SetUseClipPlane(const bool b, const QVector4D p = QVector4D(0,0,0,0));

    void BindShader(QPointer <AssetShader> shader, const bool disable_fog = false);
    void UnbindShader(QPointer <AssetShader> shader);

    void DrawCollisionModelGL(QPointer <AssetShader> shader);
    void DrawSkyboxGL(QPointer <AssetShader> shader, const QMatrix4x4 & model_matrix);   

    void SetCubemap(const QVector <QString> & skybox_image_ids, CUBEMAP_TYPE p_skybox_type);
    QPointer <AssetSkybox> GetCubemap(CUBEMAP_TYPE p_skybox_type); 

    bool SetSelected(const QString & selected, const bool b);
    void SyncAll();

    QPointer <RoomPhysics> GetPhysics();

    QPointer <QScriptEngine> GetScriptEngine();

    void UpdateAssets();
    void UpdateObjects(QPointer <Player> player, MultiPlayerManager*  multi_players, const bool player_in_room);
    void UpdateJS(QPointer <Player> player, MultiPlayerManager *multi_players);
    void UpdatePhysics(QPointer <Player> player);
    void UpdateAutoPlay();

    void SetPlayerInRoom(QPointer <Player> player);

    void StopAll();
    void SetSoundsEnabled(const bool b);
    void ResetSoundTriggers();

    bool SaveXML(const QString & filename);
    void SaveXML(QTextStream & ofs);

    bool SaveJSON(const QString & filename);

    void OnCollisionEnter(QPointer <RoomObject> envobject, QPointer <RoomObject> other_envobject, QPointer <Player> player);
    void OnCollisionExit(QPointer <RoomObject> envobject, QPointer <RoomObject> other_envobject, QPointer <Player> player);

    bool RunKeyPressEvent(QKeyEvent * e, QPointer <Player> player, MultiPlayerManager * multi_players);
    bool RunKeyReleaseEvent(QKeyEvent * e, QPointer <Player> player, MultiPlayerManager * multi_players);

    unsigned int GetRoomNumTris() const;    

    void SetPlayerLastTransform(const QMatrix4x4 & m); //remmebers player's last position
    QMatrix4x4 GetPlayerLastTransform() const;   

    void StartURLRequest();    

    void ImportCode(const QString code, const QString src_url);
    void UpdateCode(const QString code);

    //loading/processing status related
    float GetProgress() const;
    bool GetReady() const;
    bool GetReadyForScreenshot() const;

    void SetStarted(const bool b);
    bool GetStarted() const;

    void SetLoaded(const bool b);
    bool GetLoaded() const;

    void SetProcessing(bool b);
    bool GetProcessing() const;

    void SetProcessed(bool b);    
    bool GetProcessed() const;

    QPointer <HTMLPage> GetPage() const;   

    bool GetTranslatorBusy() const;
    void SetTranslatorBusy(bool value);

    void Create();
    void Create_Default_Workspace();

    void RenameJSID(const QString & old_js_id, const QString & new_js_id);

    PerformanceLogger & GetPerformanceLogger();

    void GetLights(LightContainer* p_container);

    QPair <QVector3D, QVector3D> GetResetVolume();

    //custom translator support
    QString GetURL() const;

    //adding things    
    QString AddText_Interactive(const QVector3D & pos, const QVector3D & xdir,  const QVector3D & ydir,  const QVector3D & zdir, const QString & js_id);
    QString AddImage_Interactive(const QVector3D & pos, const QVector3D & xdir,  const QVector3D & ydir,  const QVector3D & zdir, const QString & js_id);
    QString AddObject_Interactive(const QVector3D & pos, const QVector3D & xdir,  const QVector3D & ydir,  const QVector3D & zdir, const QString & js_id);

    //connections
    void SetEntranceObject(QPointer <RoomObject> o);
    QPointer <RoomObject> GetEntranceObject() const;

    void SetParentObject(QPointer <RoomObject> o);
    QPointer <RoomObject> GetParentObject() const;

    void SetParent(QPointer <Room> r);
    QPointer <Room> GetParent() const;
    QList <QPointer <Room> > & GetChildren();
    QList <QPointer <Room> > GetAllChildren();
    QPointer <Room> GetLastChild() const;
    void SetLastChild(QPointer <Room> r);

    void AddChild(QPointer <Room> r);
    void RemoveChild(QPointer <Room> r);    

    QMap <QString, QPointer <Room> > GetVisibleRooms();

    QPointer <Room> GetConnectedRoom(QPointer <RoomObject> p);
    QPointer <RoomObject> GetConnectedPortal(QPointer <RoomObject> p);

    QList <QScriptValue> & GetQueuedFunctions();

    //statics
    static void LoadSkyboxes();
    static void LoadTemplates();

    static QPointer <AssetShader> GetTransparencyShader();
    static QPointer <AssetShader> GetPortalShader();
    static QPointer <AssetShader> GetCubemapShader();
    static QPointer <AssetShader> GetCubemapShader2();
    static QPointer <AssetShader> GetSkyboxShader();

private:   

    QVariantMap GetJSONCode(const bool show_defaults) const;

    void LinkToAssets(QPointer <RoomObject> o);    

    void Create_Error(const int code);
    void Create_WebSurface();
    void Create_Default(const QVariantMap fireboxroom);
    void Create_Default_Assets_Helper(const QVariantMap & assets);
    void Create_Default_Helper(const QVariantMap & d, QPointer <RoomObject> p);
    void Create_Reddit();
    void Create_RedditComments();
    void Create_Imgur();
    void Create_Vimeo();
    void Create_Youtube();
    void Create_Flickr();
    void Create_DirectoryListing();

    void BindCubemaps(QPointer <AssetShader> shader);   

    void DoEditsDeletes(QPointer <RoomObject> obj);   

    void create_filtered_cubemaps_from_skybox();
    void remove_intermediate_cubemap_files();    

    void AddPrimitiveAssetObjects();

    void SetAllObjectsLocked(const bool b);

    void LogErrorOnException(QPointer <AssetScript> script);

    //private room-specific properties
    QVector3D player_pos_trans;
    bool use_clip_plane;
    QVector4D plane_eqn;  

    //all Assets and Objects for the Room
    QHash <QString, QPointer <AssetGhost> > assetghosts;
    QHash <QString, QPointer <AssetImage> > assetimages;
    QHash <QString, QPointer <AssetObject> > assetobjects;
    QHash <QString, QPointer <AssetRecording> > assetrecordings;
    QHash <QString, QPointer <AssetScript> > assetscripts;
    QHash <QString, QPointer <AssetShader> > assetshaders;
    QHash <QString, QPointer <AssetSound> > assetsounds;
    QHash <QString, QPointer <AssetVideo> > assetvideos;    
    QHash <QString, QPointer <AssetWebSurface> > assetwebsurfaces;

    QHash <QString, QPointer <RoomObject> > envobjects; //indexes to envobject via js_id    

    //all objects for JS accessibility    
    QPointer <DOMNode> props;

    //functions queued by JS environment
    QList <QScriptValue> queued_functions;

    //skybox
    QPointer <AssetSkybox> cubemap;
    QPointer <AssetSkybox> cubemap_radiance;
    QPointer <AssetSkybox> cubemap_irradiance;   

    QPointer <AssetShader> assetshader;   

    QPointer <HTMLPage> page;   

    QPointer <RoomPhysics> physics;
    QPointer <QScriptEngine> script_engine;

    PerformanceLogger perf_logger;

    QMatrix4x4 player_lastxform;

    QString room_template;

    //Connections
    QPointer <RoomObject> entrance_object;
    QPointer <RoomObject> parent_object;

    QPointer <Room> parent; //my parent's node
    QList <QPointer <Room> > children; //my children nodes
    QPointer <Room> last_child; //last visited child node

    bool scripts_ready;    
    bool translator_busy;

    //statics    
    static QList <QPointer <AssetSkybox> > skyboxes;    
    static QHash <QString, QPointer <RoomTemplate> > room_templates;
    static QHash <QString, QPointer <AssetObject> > object_primitives; //primitives

    static QPointer <AssetShader> transparency_shader; //general and normal shaders
    static QPointer <AssetShader> portal_shader;
    static QPointer <AssetShader> cubemap_shader;
    static QPointer <AssetShader> cubemap_shader2;
    static QPointer <AssetShader> skybox_shader;
};

#endif // ROOM_H

#ifndef ASSETSCRIPT_H
#define ASSETSCRIPT_H

#include <QtNetwork>
#include <QtCore>
#include <QtScript>
#include <QtScriptTools>

#include "asset.h"
#include "htmlpage.h"
#include "scriptvalueconversions.h"
#include "scriptbuiltins.h"
#include "cookiejar.h"

#include "player.h"
#include "roomobject.h"
#include "room.h"
#include "domnode.h"

class Room; //forward declaration

class AssetScript : public Asset
{
    Q_OBJECT

public:

    AssetScript(QPointer <Room> room);
    ~AssetScript();

    static QPointer <QScriptEngine> GetNewScriptEngine(QPointer <Room> r);
    static QVariant GetRegisteredElements(QPointer <Room> r);

    void Load();
    void Destroy();

    QScriptValue GetGlobalProperty(const QString & name);
    bool HasGlobalProperty(const QString & name);
    bool HasFunction(const QString & name);    

    QScriptValue GetRoomProperty(const QString & name);
    bool HasRoomProperty(const QString & name);
    bool HasRoomFunction(const QString & name);
    bool HasRoomFunctionContains(const QString & name, const QString & code);

    QList<QPointer <RoomObject> > RunScriptCodeOnObjects(const QString & code, QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap<QString, DOMNode *> remote_players);
    QList<QPointer <RoomObject> > RunFunctionOnObjects(const QString & fn_name, QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap <QString, DOMNode *> remote_players,  const QScriptValueList & args = QScriptValueList());

    QList<QPointer <RoomObject> > UpdateAsynchronousCreatedObjects(QHash <QString, QPointer <RoomObject> > & envobjects);

//    QList<QPointer <RoomObject> > OnCollisionWithPlayer(QPointer<ObjectProperties> obj, Player & player,  Room & room, QHash <QString, QPointer <RoomObject> > & objects);
//    QList<QPointer <RoomObject> > OnCollision(QPointer<ObjectProperties> obj1, QPointer<ObjectProperties> obj2, QHash <QString, QPointer <RoomObject> > & objects);

    QList<QPointer <RoomObject> > OnKeyEvent(QString name, QKeyEvent * e, QHash <QString, QPointer <RoomObject> > & objects, QPointer <Player> player, QMap<QString, DOMNode *> remote_players, bool * defaultPrevented);

    void SetOnLoadInvoked(const bool b);
    bool GetOnLoadInvoked() const;   

    bool GetFinished();

    void Update();   

    void SetJSCode(const QString code);
    QString GetJSCode();

    void UpdateInternalDataStructures(QPointer <Player> player, QMap<QString, DOMNode *> remote_players);

    void ProcessNewAssets();

    QList <QPointer <RoomObject> > DoRoomLoad(QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap <QString, DOMNode *> remote_players);
    QList <QPointer <RoomObject> > DoRoomUpdate(QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap <QString, DOMNode *> remote_players, const QScriptValueList & args);
    QList <QPointer <RoomObject> > DoRoomOnPlayerEnterEvent(QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap <QString, DOMNode *> remote_players, const QScriptValueList & args);
    QList <QPointer <RoomObject> > DoRoomOnPlayerExitEvent(QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap <QString, DOMNode *> remote_players, const QScriptValueList & args);

private:

    void HandleCookieChanges();
    void HandleParentChanges(QHash <QString, QPointer <RoomObject> > & envobjects);

    void WipeRemovedObjects(QHash <QString, QPointer <RoomObject> > & envobjects);
    QList<QPointer <RoomObject> > FlushNewObjects();

    QScriptValue RunScriptCode(const QString & code);
    QScriptValue RunFunction(const QString & name, const QScriptValueList & args = QScriptValueList());

    QScriptValue global_scope;
    QScriptValue roomObject;    
    bool on_load_invoked;
    QPointer <Room> room;
    QPointer <QScriptEngine> script_engine;
    QString last_code;

    QScriptValue room_load_fn;
    QScriptValue room_update_fn;
    QScriptValue room_onplayerenterevent_fn;
    QScriptValue room_onplayerexitevent_fn;
};

#endif // ASSETSCRIPT_H

#include "assetscript.h"

AssetScript::AssetScript(QPointer <Room> r) :
    on_load_invoked(false),
    room(r)
{
    props->SetType(TYPE_ASSETSCRIPT);
    script_engine = r->GetScriptEngine();
    global_scope = script_engine->globalObject();
    roomObject = global_scope.property("room");
}

AssetScript::~AssetScript()
{
    Destroy();
}

QPointer <QScriptEngine> AssetScript::GetNewScriptEngine(QPointer <Room> r)
{
    QPointer <QScriptEngine> se = new QScriptEngine();
    se->setProcessEventsInterval(1);

    //    qRegisterMetaType<ScriptableVector>("Vector");

    qScriptRegisterMetaType(se, PlayerToScriptValue, PlayerFromScriptValue);
    qScriptRegisterMetaType(se, DOMNodeToScriptValue, DOMNodeFromScriptValue);
    qScriptRegisterMetaType(se, DOMNodeMapToScriptValue, DOMNodeMapFromScriptValue);

    qScriptRegisterSequenceMetaType<QList < DOMNode * > >(se);

    QScriptValue global_scope = se->globalObject();

    global_scope.setProperty("room", se->toScriptValue(r->GetProperties().data()));
    global_scope.setProperty("print", se->newFunction(MyPrint));

    //59.3 - dummy/no-op for "console" object
    QScriptValue console = se->newObject();
    global_scope.setProperty("console", console);
    console.setProperty("log", se->newFunction(MyConsoleLog));

//    global_scope.setProperty("room", se->toScriptValue<DOMNode *>(r->GetProperties().data()));
    QScriptValue roomObject = global_scope.property("room");
    roomObject.setProperty("preventDefault", se->newFunction(PreventDefault));
    roomObject.setProperty("openLink", se->newFunction(OpenLink));
    roomObject.setProperty("closeLink", se->newFunction(CloseLink));
    roomObject.setProperty("createObject", se->newFunction(CreateObject));
    roomObject.setProperty("removeObject", se->newFunction(RemoveObject));
    roomObject.setProperty("addCookie", se->newFunction(AddCookie));
    roomObject.setProperty("playSound", se->newFunction(ScriptPlaySound));
    roomObject.setProperty("seekSound", se->newFunction(ScriptSeekSound));
    roomObject.setProperty("pauseSound", se->newFunction(ScriptPauseSound));
    roomObject.setProperty("stopSound", se->newFunction(ScriptStopSound));
    roomObject.setProperty("playVideo", se->newFunction(ScriptPlayVideo));
    roomObject.setProperty("seekVideo", se->newFunction(ScriptSeekVideo));
    roomObject.setProperty("pauseVideo", se->newFunction(ScriptPauseVideo));
    roomObject.setProperty("stopVideo", se->newFunction(ScriptStopVideo));
    roomObject.setProperty("playRecording", se->newFunction(ScriptPlayRecording));
    roomObject.setProperty("seekRecording", se->newFunction(ScriptSeekRecording));
    roomObject.setProperty("pauseRecording", se->newFunction(ScriptPauseRecording));
    roomObject.setProperty("stopRecording", se->newFunction(ScriptStopRecording));
    roomObject.setProperty("cookies", se->newObject());
    roomObject.setProperty("getObjectById", se->newFunction(GetObjectById));
    roomObject.setProperty("loadNewAsset", se->newFunction(LoadNewAsset));

    global_scope.setProperty("room", roomObject);

    QScriptValue xmlHttpRequestConstructor = se->newFunction(XmlHttpRequestConstructor);
    global_scope.setProperty("XMLHttpRequest", xmlHttpRequestConstructor);

    global_scope.setProperty("Vector", se->newFunction(VectorConstructor));
    global_scope.setProperty("V", se->newFunction(VectorConstructor));
    global_scope.setProperty("translate", se->newFunction(Translate));
    global_scope.setProperty("equals", se->newFunction(Equals));
    global_scope.setProperty("scalarMultiply", se->newFunction(ScalarMultiply));
    global_scope.setProperty("scale", se->newFunction(Scale));
    global_scope.setProperty("copy", se->newFunction(Copy));
    global_scope.setProperty("add", se->newFunction(Add));
    global_scope.setProperty("cross", se->newFunction(Cross));
    global_scope.setProperty("normalized", se->newFunction(Normalized));
    global_scope.setProperty("distance", se->newFunction(Distance));

    global_scope.setProperty("removeKey", se->newFunction(RemoveKey));
    global_scope.setProperty("uniqueId", se->newFunction(UniqueId));
    global_scope.setProperty("debug", se->newFunction(Debug));

    return se;
}

void AssetScript::Load()
{    
    //53.12 - Setup cookies here, as by now the proper base_url which defines the right domain is set
//    qDebug() << "AssetScript::Load()" << src_url;
    QUrl cookieDomain = QUrl(props->GetBaseURL());
    foreach (QNetworkCookie cookie, CookieJar::cookie_jar->cookiesForUrl(cookieDomain)) {
        roomObject.property("cookies").setProperty(QString(cookie.name()), QScriptValue(script_engine, QString(cookie.value())));
    }

    WebAsset::Load(QUrl(props->GetSrcURL()));
}

void AssetScript::Destroy()
{
    if (script_engine->isEvaluating()) {
        script_engine->abortEvaluation();
    }
}

QScriptValue AssetScript::GetRoomProperty(const QString & name)
{
    QScriptValue currentObject = script_engine->globalObject().property("room");
    foreach (QString prop, name.split(".")) {
        currentObject = currentObject.property(prop);

        if (!currentObject.isValid())
            break;
    }

    return currentObject;
}

QScriptValue AssetScript::GetGlobalProperty(const QString & name)
{
    QScriptValue currentObject = script_engine->globalObject();
    foreach (QString prop, name.split(".")) {
        currentObject = currentObject.property(prop);

        if (!currentObject.isValid())
            break;
    }

    return currentObject;
}

bool AssetScript::HasGlobalProperty(const QString & name)
{
    return GetGlobalProperty(name).isValid();
}

bool AssetScript::HasFunction(const QString & name)
{
    return GetGlobalProperty(name).isFunction();
}

bool AssetScript::HasRoomProperty(const QString & name)
{
    return roomObject.property(name).isValid();
}

bool AssetScript::HasRoomFunction(const QString & name)
{
//    qDebug() << "AssetScript::HasRoomFunction" << roomObject.toString();
    return roomObject.property(name).isFunction();
}

bool AssetScript::HasRoomFunctionContains(const QString & name, const QString & code)
{
    return roomObject.property(name).toString().contains(code);
}

QScriptValue AssetScript::RunScriptCode(const QString & code)
{
//    qDebug() << "AssetScript::RunScriptCode() " << code;
    last_code = code;
    return script_engine->evaluate(code, props->GetSrc());
}

QScriptValue AssetScript::RunFunction(const QString & name, const QScriptValueList & args)
{
//    qDebug() << "AssetScript::RunFunction" << this->GetFullURL() << name;
    last_code = name;
    QScriptValue fn = GetGlobalProperty(name);
    if (!fn.isFunction()) {
        return QScriptValue();
    }
    return fn.call(QScriptValue(), args);
}

void AssetScript::ProcessNewAssets()
{
    QScriptValueIterator itr(global_scope.property("__new_assets"));

    while(itr.hasNext())
    {
        itr.next();

        if (itr.flags() & QScriptValue::SkipInEnumeration) {
            continue;
        }

        QScriptValue asset = itr.value();
        QString id = itr.name();        
        QString asset_type = asset.property("type").toString();
        QScriptValue props = asset.property("properties");

        int state = asset.property("state").toInteger();       

        if (state==0) {
            //loadNewAsset() code evaluated; need to create new asset
            QVariantMap propsMap = props.toVariant().toMap();
            if (asset_type != "assetscript") {
                room->AddAsset(asset_type, propsMap);
            }
            asset.setProperty(QString("state"), QScriptValue(1));
        }
        else if (state==1) {
            //asset has been created. Check if asset loaded: if yes, call callback function
            bool processed = false;
            bool error = false;

            if (asset_type == "assetscript") {
                processed = true;
                error = true;
            }
            else {
                QPointer <Asset> a = room->GetAsset(id);
                if (a.isNull()) {
                    processed = true;
                    error = true;
                }
                else if (a->GetFinished()) {
                    processed = true;
                }
            }

            //59.7 - if processed, call the callback function, and remove this
            if (processed) {
                QScriptValue thisObject = script_engine->newObject();
                thisObject.setProperty("id", QScriptValue(id));
                thisObject.setProperty("properties", QScriptValue(props));
                thisObject.setProperty("type", QScriptValue(asset_type));
                thisObject.setProperty("error", QScriptValue(error));

                QScriptValue callbackFunction = asset.property("callback");
                if (!callbackFunction.isUndefined()) {
                    callbackFunction.call(thisObject, QScriptValueList());
                }

                itr.remove();
            }
        }
    }
}

void AssetScript::HandleCookieChanges()
{
    // flush new cookies
    QScriptValueIterator itr(roomObject.property("_new_cookies"));
    QList<QNetworkCookie> newCookies;

    //create the list of new cookies to add
    while (itr.hasNext()) {
        itr.next();

        if (itr.flags() & QScriptValue::SkipInEnumeration) {
            continue;
        }

        QString cookieDomain = QUrl(props->GetBaseURL()).host();

        QNetworkCookie newCookie(itr.name().toLatin1(), itr.value().toString().toLatin1());
        newCookie.setDomain(cookieDomain);
        newCookie.setPath("/");
//        qDebug() << "AssetScript::HandleCookieChanges(): Adding a new cookie" << cookieDomain << itr.name() << itr.value().toString();
        newCookies << newCookie;
    }

    //add the new list of cookies, via the pointer set to the global cookie jar
    if (!newCookies.isEmpty()) {
        //Note: do not change from base_url (doesn't work using base_url.host())
//        qDebug() << "AssetScript::HandleCookieChanges()" << newCookies.first().domain() << newCookies << base_url;
        CookieJar::cookie_jar->setCookiesFromUrl(newCookies, QUrl(props->GetBaseURL()));
        CookieJar::cookie_jar->SaveToDisk();
    }

//    cookies->PrintAllCookies();
    roomObject.setProperty("_new_cookies", script_engine->newObject());
}

void AssetScript::HandleParentChanges(QHash <QString, QPointer <RoomObject> > & envobjects)
{    
    QScriptValueIterator itr(global_scope.property("__parent_changed"));
    while (itr.hasNext())
    {
        itr.next();

        if (itr.flags() & QScriptValue::SkipInEnumeration) {
            continue;
        }

        QString jsid = itr.name();
        QString oldParentJSID = itr.value().property("old_parent_id").toString();
        QString newParentJSID = itr.value().property("new_parent_id").toString();

//        qDebug() << "AssetScript::HandleParentChanges()" << jsid << oldParentJSID << newParentJSID;
        if (envobjects.contains(jsid))  //make sure the object is not freshly created
        {
//            qDebug()<<jsid<<"underwent a DOM change";
            if (envobjects.contains(oldParentJSID))     //this should always be true
                envobjects[oldParentJSID]->RemoveChildByJSID(jsid);

            if(envobjects.contains(newParentJSID))      //this should always be true
                envobjects[newParentJSID]->AppendChild(envobjects[jsid]);   //this sets the parent as well
            else if(newParentJSID == QString("__room"))
                envobjects[jsid]->SetParentObject(NULL);
        }
    }

    global_scope.setProperty("__parent_changed", QScriptValue());
}

void AssetScript::WipeRemovedObjects(QHash <QString, QPointer <RoomObject> > & envobjects)
{
    QList <QString> to_delete;
    QHash <QString, QPointer <RoomObject> >::iterator it;
    for (it=envobjects.begin(); it!=envobjects.end(); ++it) {
        QPointer <RoomObject> obj = it.value();

        if (obj.isNull()) {
            continue;
        }

        if (obj->IsDirty()) {
            to_delete.push_back(it.key());
        }
    }        

    for (int i=0; i<to_delete.size(); ++i) {
        if (envobjects[to_delete[i]]) {
            delete envobjects[to_delete[i]];
        }
        envobjects.remove(to_delete[i]);        
    }
}

QList<QPointer <RoomObject> > AssetScript::FlushNewObjects()
{        
//    qDebug() << "AssetScript::FlushNewObjects()" << this;
    QList<QPointer <RoomObject> > objectsAdded;
    QScriptValueIterator itr(roomObject.property("_new_ids"));
    while (itr.hasNext()) {
        itr.next();
        if (itr.flags() & QScriptValue::SkipInEnumeration) {
            continue;
        }

        const QString newId = itr.value().toString();
        QPointer <DOMNode> newDOMNode(qobject_cast<DOMNode *>(global_scope.property("__dom").property(newId).toQObject()));

        if (newDOMNode.isNull()) {   //meaning that the object has already been removed via room.removeObject
            continue;
        }
        //dom.insert(newId, newDOMNode);

        QPointer <RoomObject> newObject = RoomObject::CreateFromProperties(newDOMNode);
        newObject->SetType(newDOMNode->GetType());

//        qDebug() << "AssetScript::FlushNewObjects()" << newObject << newObject->GetProperties()->GetTypeAsString() << newId << newObject->GetProperties()->GetJSID();
        room->GetRoomObjects()[newId] = newObject;
        objectsAdded << newObject;
    }

    roomObject.property("_new_ids").setProperty("length", 0);

    return objectsAdded;
}

void AssetScript::SetJSCode(const QString code)
{
    SetData(code.toLatin1());
    SetProcessing(false);
    SetProcessed(false);
    SetFinished(false);
}

void AssetScript::Update()
{
    if (GetLoaded() && !GetProcessing() && !GetError()) {
//        qDebug() << "AssetScript::Update() - Running script code" << this->GetFullURL();
        //60.0 - bugfix, CLEAR onLoad/update remnants from previous script before injecting,
        //       otherwise there is a bug with multiple erroneous function calls
        roomObject.setProperty("onLoad", QScriptValue());
        roomObject.setProperty("update", QScriptValue());
        roomObject.setProperty("onPlayerEnterEvent", QScriptValue());
        roomObject.setProperty("onPlayerExitEvent", QScriptValue());

        RunScriptCode(QString(GetData()));

        room_load_fn = roomObject.property("onLoad");
        room_update_fn = roomObject.property("update");
        room_onplayerenterevent_fn = roomObject.property("onPlayerEnterEvent");
        room_onplayerexitevent_fn = roomObject.property("onPlayerExitEvent");

        QScriptSyntaxCheckResult syntax_result = script_engine->checkSyntax(last_code);
        if (syntax_result.state() != QScriptSyntaxCheckResult::Valid) {
            QString err = " JS error: Syntax check failed (line "+
                    QString::number(syntax_result.errorLineNumber()) +
                    " column "+
                    QString::number(syntax_result.errorColumnNumber()) +
                    "): " +
                    syntax_result.errorMessage();

            MathUtil::ErrorLog(err);
            script_engine->clearExceptions();
        }

        SetProcessing(true);
        SetProcessed(true);
        SetFinished(true);
//        webasset.ClearData();
    }
}

QList<QPointer <RoomObject> > AssetScript::UpdateAsynchronousCreatedObjects(QHash <QString, QPointer <RoomObject> > & envobjects)
{
//    qDebug() << "AssetScript::UpdateAsynchronousCreatedObjects()";
    ProcessNewAssets();
    HandleCookieChanges();
    HandleParentChanges(envobjects);
    WipeRemovedObjects(envobjects);
    return FlushNewObjects();
}

void AssetScript::UpdateInternalDataStructures(QPointer <Player> player, QMap <QString, DOMNode *> remote_players)
{
    //56.0 - ensure DOM/roomobjects always get updated (including object properties pointers for portals)
    //60.0 - INSANELY IMPORTANT - make sure do not overwrite the dom map and remove objects created by JS via CreateObject!    
    QMap <QString, DOMNode* > dom_map;
    DOMNodeMapFromScriptValue(global_scope.property("__dom"), dom_map);

    QHash <QString, QPointer <DOMNode> >::iterator iter;
    for (QPointer <RoomObject> & o : room->GetRoomObjects()) {
        if (o) {
            //            qDebug() << node << node->GetS("js_id") << node->GetS("id");
            dom_map[o->GetProperties()->GetJSID()] = o->GetProperties().data();
        }
    }
    //    qDebug() << "AssetScript::UpdateInternalDataStructures" << dom_map;

    if (!global_scope.property("player").isValid()) {
        global_scope.setProperty("player", script_engine->toScriptValue(player->GetProperties().data()));
    }
    if (!global_scope.property("room").isValid()) {
        global_scope.setProperty("room", script_engine->toScriptValue(room->GetProperties().data()));
    }
    global_scope.setProperty("__dom", script_engine->toScriptValue(dom_map));
    roomObject.setProperty("objects", script_engine->toScriptValue(dom_map));
    //remote player map
//    qDebug() << "AssetScript::UpdateInternalDataStructures" << remote_players << remote_players.size();
    roomObject.setProperty("players", script_engine->toScriptValue(remote_players));
    roomObject.setProperty("playerCount", remote_players.size());
}

QList<QPointer <RoomObject> > AssetScript::RunScriptCodeOnObjects(const QString & code, QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap <QString, DOMNode *> remote_players)
{
    UpdateInternalDataStructures(player, remote_players);
    RunScriptCode(code);
    return UpdateAsynchronousCreatedObjects(envobjects);
}

QList<QPointer <RoomObject> > AssetScript::RunFunctionOnObjects(const QString & fnName, QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap<QString, DOMNode *> remote_players, const QScriptValueList & args)
{
    if (!HasFunction(fnName)) {
        const QString err = "Couldn't run undefined function on objects:" + fnName;
        MathUtil::ErrorLog(err);
        return QList<QPointer <RoomObject> >();
    }

    UpdateInternalDataStructures(player, remote_players);
    RunFunction(fnName, args);
//    qDebug() << "AssetScript::RunFunctionOnObjects" << this << fnName;
    return UpdateAsynchronousCreatedObjects(envobjects);
}

//QList<QPointer <RoomObject> > AssetScript::OnCollisionWithPlayer(QPointer<ObjectProperties> obj, Player & player, Room & room, QHash <QString, QPointer <RoomObject> > & objects)
//{
//    if (obj.isNull()) {
//        MathUtil::ErrorLog("Couldn't handle collision, one or more objects involved undefined");
//        return QList<QPointer <RoomObject> >();
//    }

//    // call first with player, obj then again with obj2, obj1 so both cases can be handled
//    QScriptValue playerScript = GetGlobalProperty("player");
//    QScriptValue objScript = global_scope.property("__dom").property(obj->GetJSID());

//    QList<QPointer <RoomObject> > addedObjects;

//    // general onCollision event
//    if (HasRoomFunction("onCollision")) {
//        addedObjects += RunFunctionOnObjects("room.onCollision", objects, player, room, QScriptValueList() << objScript << playerScript);
//        if (obj.isNull()) {
//            return addedObjects;
//        }
//        addedObjects += RunFunctionOnObjects("room.onCollision", objects, player, room, QScriptValueList() << playerScript << objScript);
//        if (obj.isNull()) {
//            return addedObjects;
//        }
//    }

//    // player oncollision Property
//    if (player.GetOnCollision().length() > 0) {
//        addedObjects += RunFunctionOnObjects(player.GetOnCollision(), objects, player, room,QScriptValueList() << objScript);
//        if (obj.isNull()) {
//            return addedObjects;
//        }
//    }

//    // object oncollision Property
//    if (obj->oncollision.length() > 0) {
//        addedObjects += RunFunctionOnObjects(obj->oncollision, objects, player, room, QScriptValueList() << objScript << playerScript);
//    }

//    return addedObjects;
//}

//QList<QPointer <RoomObject> > AssetScript::OnCollision(QPointer<ObjectProperties> obj1, QPointer<ObjectProperties> obj2, QHash <QString, QPointer <RoomObject> > & objects)
//{
//    if (obj1 == 0 || obj2 == 0) {
//        MathUtil::ErrorLog("Couldn't handle collision, one or more objects involved undefined");
//        return QList<QPointer <RoomObject> >();
//    }

//    QScriptValue obj1Script = global_scope.property("__dom").property(obj1->GetJSID());
//    QScriptValue obj2Script = global_scope.property("__dom").property(obj2->GetJSID());
//    QList<QPointer <RoomObject> > addedObjects;

//    // general onCollision event
//    if (HasRoomFunction("onCollision")) {
//        addedObjects += RunFunctionOnObjects("room.onCollision", objects, QScriptValueList() << obj1Script << obj2Script);
//        if (obj1 == 0 || obj2 == 0) {
//            return addedObjects;
//        }
//        addedObjects += RunFunctionOnObjects("room.onCollision", objects, QScriptValueList() << obj2Script << obj1Script);
//        if (obj1 == 0 || obj2 == 0) {
//            return addedObjects;
//        }
//    }

//    // first object oncollision Property
//    QString collisionCallback1 = obj1->oncollision;
//    if (collisionCallback1.length() > 0) {
//        addedObjects += RunFunctionOnObjects(collisionCallback1, objects, QScriptValueList() << obj1Script << obj2Script);
//        if (obj1 == 0 || obj2 == 0) {
//            return addedObjects;
//        }
//    }

//    // second object oncollision Property
//    QString collisionCallback2 = obj2->oncollision;
//    if (collisionCallback2.length() > 0) {
//        addedObjects += RunFunctionOnObjects(collisionCallback2, objects, QScriptValueList() << obj2Script << obj1Script);
//    }

//    return addedObjects;
//}

QList<QPointer <RoomObject> > AssetScript::OnKeyEvent(QString name, QKeyEvent * e, QHash <QString, QPointer <RoomObject> > & objects, QPointer <Player> player, QMap <QString, DOMNode *> remote_players, bool * defaultPrevented)
{
    if (!HasRoomFunction(name)) {
        *defaultPrevented = false;
        return QList<QPointer <RoomObject> >();
    }

    QScriptValue eventScriptObject = KeyEventToScriptValue(script_engine, e);
    QList<QPointer <RoomObject> > objectsAdded = RunFunctionOnObjects("room." + name, objects, player, remote_players, QScriptValueList() << eventScriptObject);
    *defaultPrevented = eventScriptObject.property("_defaultPrevented").toBool();
    return objectsAdded;
}

void AssetScript::SetOnLoadInvoked(const bool b)
{
    on_load_invoked = b;
}

bool AssetScript::GetOnLoadInvoked() const
{
    return on_load_invoked;
}

bool AssetScript::GetFinished()
{
    return WebAsset::GetFinished() && !GetError();
}

QString AssetScript::GetJSCode()
{
    return GetData();
}

QList <QPointer <RoomObject> > AssetScript::DoRoomLoad(QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap <QString, DOMNode *> remote_players)
{
//    qDebug() << "AssetScript::DoRoomLoad" << this;
    UpdateInternalDataStructures(player, remote_players);
    if (room_load_fn.isFunction()) {
//        qDebug() << "AssetScript::DoRoomLoad" << this << room_load_fn.toString();
        room_load_fn.call(roomObject);
    }
    return UpdateAsynchronousCreatedObjects(envobjects);
}

QList <QPointer <RoomObject> > AssetScript::DoRoomUpdate(QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap <QString, DOMNode *> remote_players, const QScriptValueList & args)
{
    UpdateInternalDataStructures(player, remote_players);
    if (room_update_fn.isFunction()) {
        room_update_fn.call(roomObject, args);
    }
    return UpdateAsynchronousCreatedObjects(envobjects);
}

QList <QPointer <RoomObject> > AssetScript::DoRoomOnPlayerEnterEvent(QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap <QString, DOMNode *> remote_players, const QScriptValueList & args)
{
    UpdateInternalDataStructures(player, remote_players);
    if (room_onplayerenterevent_fn.isFunction()) {
        room_onplayerenterevent_fn.call(roomObject, args);
    }
    return UpdateAsynchronousCreatedObjects(envobjects);
}

QList <QPointer <RoomObject> > AssetScript::DoRoomOnPlayerExitEvent(QHash <QString, QPointer <RoomObject> > & envobjects, QPointer <Player> player, QMap <QString, DOMNode *> remote_players, const QScriptValueList & args)
{
    UpdateInternalDataStructures(player, remote_players);
    if (room_onplayerexitevent_fn.isFunction()) {
        room_onplayerexitevent_fn.call(roomObject, args);
    }
    return UpdateAsynchronousCreatedObjects(envobjects);
}

#include "scriptbuiltins.h"

QList <ScriptLogMessage> ScriptBuiltins::script_print_log;
QString ScriptBuiltins::janus_queued_functions = "__janus_queued_fns";

QScriptValue MyPrint(QScriptContext * context, QScriptEngine * engine)
{
    ScriptLogMessage m;
    m.msg = context->argument(0).toString();
    m.output_to_chat = true;
    ScriptBuiltins::script_print_log.push_back(m);
    return engine->undefinedValue();
}

QScriptValue MyConsoleLog(QScriptContext * context, QScriptEngine * engine)
{
    ScriptLogMessage m;
    m.msg = context->argument(0).toString();
    m.output_to_chat = false;
    ScriptBuiltins::script_print_log.push_back(m);
    return engine->undefinedValue();
}

QScriptValue CreateObject(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue roomObject = engine->globalObject().property("room");

    // object type is mandatory, other properties are optional
    const QString typeName = context->argument(0).toString().trimmed().toLower();

    QScriptValue propsHash = context->argument(1);    
    if (!propsHash.isValid()) {
        propsHash = engine->newObject();
    }
    propsHash.setProperty("type", typeName);
//    qDebug() << "CreateObject" << typeName << propsHash.toString() << propsHash.property("js_id").toString() << propsHash.property("url").toString();

    // if the user didn't specify a javascript id for the object, generate one
    if (!propsHash.property("js_id").isValid()) {
        propsHash.setProperty("js_id", QString::number(RoomObject::NextUUID()));        
    }
//    qDebug() << "CreateObject2" << typeName << propsHash.toString() << propsHash.property("js_id").toString() << propsHash.property("url").toString();
    QScriptValue newId = propsHash.property("js_id");

//    QScriptValue existingNode = roomObject.property("getObjectById").call(roomObject, QScriptValueList()<< QScriptValue(newId));
    QScriptValue existingNode = engine->globalObject().property("__dom").property(newId.toString());
    if (existingNode.isValid()) {
        context->throwError(QString("An object with the specified js_id '") + newId.toString() + QString("' already exists!"));
        return engine->undefinedValue();
    }

    // we need to cue up the ids of added objects in an intermediary array
    // so we can flush them later into the room's EnvObject list
    QScriptValue newIds = roomObject.property("_new_ids");
    if (!newIds.isValid()) {
        newIds = engine->newArray(0);
        roomObject.setProperty("_new_ids", newIds);
    }    
    newIds.property("push").call(newIds, QScriptValueList() << newId);
//    qDebug() << "CreateObject pushing" << newId.toString();

    QPointer <DOMNode> newNode = new DOMNode();
//    newNode->setProperty("type", typeName); //60.1 - needed so DOMNodeFromScriptValue sets the correct type via SetType()
    newNode->SetType(DOMNode::StringToElementType(typeName));
    DOMNodeFromScriptValue(propsHash, newNode);    
    QScriptValue newDOMNodeScriptValue = engine->newQObject(newNode, QScriptEngine::QtOwnership, QScriptEngine::AutoCreateDynamicProperties);    

    QScriptValue parentValue = context->argument(2);
    if (parentValue.isUndefined()) {
        roomObject.property("appendChild").call(roomObject, QScriptValueList() << newDOMNodeScriptValue);
        engine->globalObject().property("__dom").setProperty(newId.toString(), newDOMNodeScriptValue);
        roomObject.property("objects").setProperty(newId.toString(), newDOMNodeScriptValue);
    }
    else {
//        qDebug() << "CreateObject with specific parent used!" << parentValue.toVariant();
        parentValue.property("appendChild").call(parentValue, QScriptValueList() << newDOMNodeScriptValue);
        engine->globalObject().property("__dom").setProperty(newId.toString(), newDOMNodeScriptValue);
//        roomObject.property("objects").setProperty(newId.toString(), newDOMNodeScriptValue);
    }

//    qDebug() << "CreateObject" << newNode->GetJSID();
    return newDOMNodeScriptValue;
}

QScriptValue RemoveObject(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue objects = engine->globalObject().property("__dom"); //this is the list of objects in the room, indexed by jsid
    QPointer<QObject> obj;
    if (context->argument(0).isQObject() && context->argument(0).isValid()) {
        obj = context->argument(0).toQObject();
    }

    QScriptValue cuedRemoveObject = engine->newObject();
    //get the js_id of the object if object passed, otherwise it's a string for the js_id
    if (obj) {
        cuedRemoveObject.setProperty("name", obj->property("js_id").toString());
    }
    else {
        cuedRemoveObject.setProperty("name", context->argument(0).toString());
    }
    cuedRemoveObject.setProperty("op", "removeObject");
    if (context->argumentCount() >= 2) {
        cuedRemoveObject.setProperty("sync", context->argument(1).toBool());
    }
    else {
        cuedRemoveObject.setProperty("sync", QScriptValue(true));
    }

    return ScriptAddFunctionToQueue(engine, cuedRemoveObject);
}

QScriptValue AddCookie(QScriptContext * context, QScriptEngine * engine)
{
    QString cookieName = context->argument(0).toString();
    QScriptValue cookieValue = context->argument(1);

    QScriptValue roomObject = engine->globalObject().property("room");
    roomObject.property("cookies").setProperty(cookieName, cookieValue);

    // keep track of new cookies in a separate list so they can be flushed
    // to the cookie jar later
    QScriptValue newCookieHash = roomObject.property("_new_cookies");
    if (!newCookieHash.isValid()) {
        roomObject.setProperty("_new_cookies", engine->newObject());
        newCookieHash = roomObject.property("_new_cookies");
    }

    newCookieHash.setProperty(cookieName, cookieValue);

    return engine->undefinedValue();
}

QScriptValue XmlHttpRequestConstructor(QScriptContext * , QScriptEngine * engine)
{
//    qDebug() << "XmlHttpRequestConstructor";
    ScriptableXmlHttpRequest * xhr = new ScriptableXmlHttpRequest(engine);
    QScriptValue v = engine->newQObject(xhr, QScriptEngine::QtOwnership);
    xhr->SetScriptValue(v);
    return v;
}

QScriptValue RemoveKey(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue hash = context->argument(0);
    QString key = context->argument(1).toString();

    hash.setProperty(key, QScriptValue());

    return engine->undefinedValue();
}

QScriptValue UniqueId(QScriptContext *, QScriptEngine * engine)
{
    return QScriptValue(engine, RoomObject::NextUUID());
}

QScriptValue Debug(QScriptContext * context, QScriptEngine * engine)
{
    engine->globalObject().property("print").call(QScriptValue(), QScriptValueList() << context->argument(0));
    return context->argument(0);
}

QScriptValue ScriptAddFunctionToQueue(QScriptEngine * engine, QScriptValue cuedObject)
{
    QScriptValue roomObject = engine->globalObject().property("room");
    if (!roomObject.property(ScriptBuiltins::janus_queued_functions).isValid()) {
        roomObject.setProperty(ScriptBuiltins::janus_queued_functions, engine->newArray(20));
    }

    // cue sounds for later playback
    int length = roomObject.property(ScriptBuiltins::janus_queued_functions).property("length").toInteger();
    roomObject.property(ScriptBuiltins::janus_queued_functions).setProperty(length, cuedObject);

    return engine->undefinedValue();
}

QScriptValue PreventDefault(QScriptContext * , QScriptEngine * engine)
{
    return engine->undefinedValue();
}

QScriptValue OpenLink(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue cuedObject = engine->newObject();
    cuedObject.setProperty("name", context->argument(0).toString());    
    cuedObject.setProperty("op", "openLink");
    return ScriptAddFunctionToQueue(engine, cuedObject);
}

QScriptValue CloseLink(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue cuedObject = engine->newObject();
    cuedObject.setProperty("name", context->argument(0).toString());
    cuedObject.setProperty("op", "closeLink");
    return ScriptAddFunctionToQueue(engine, cuedObject);
}

QScriptValue ScriptPlaySound(QScriptContext * context, QScriptEngine * engine)
{    
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "playSound");

    if (context->argumentCount() >= 2) {
        cuedSoundObject.setProperty("loop", context->argument(1).toBool());
    } else {
        cuedSoundObject.setProperty("loop", QScriptValue(false));
    }

    return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue ScriptPauseSound(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "pauseSound");
   return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue ScriptSeekSound(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "seekSound");
    if (context->argumentCount() >= 2) {
        cuedSoundObject.setProperty("pos", context->argument(1).toNumber());
    }
    else {
        cuedSoundObject.setProperty("pos", QScriptValue(0.0f));
    }

    return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue ScriptStopSound(QScriptContext * context, QScriptEngine * engine)
{        
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "stopSound");
    return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue ScriptPlayVideo(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "playVideo");
    return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue ScriptSeekVideo(QScriptContext * context, QScriptEngine * engine)
{    
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "seekVideo");
    if (context->argumentCount() >= 2) {
        cuedSoundObject.setProperty("pos", context->argument(1).toNumber());
//        qDebug() << "ScriptSeekVideo" << context->argument(1).toNumber();
    }
    else {
        cuedSoundObject.setProperty("pos", QScriptValue(0.0f));
    }

    return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue ScriptPauseVideo(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "pauseVideo");
    return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue ScriptStopVideo(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "stopVideo");
    return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue ScriptPlayRecording(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "playRecording");

    if (context->argumentCount() >= 2) {
        cuedSoundObject.setProperty("loop", context->argument(1).toBool());
    }
    else {
        cuedSoundObject.setProperty("loop", QScriptValue(false));
    }

    return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue ScriptPauseRecording(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "pauseRecording");
    return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue ScriptSeekRecording(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "seekRecording");
    if (context->argumentCount() >= 2) {
        cuedSoundObject.setProperty("pos", context->argument(1).toNumber());
    }
    else {
        cuedSoundObject.setProperty("pos", QScriptValue(0.0f));
    }

    return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue ScriptStopRecording(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue cuedSoundObject = engine->newObject();
    cuedSoundObject.setProperty("name", context->argument(0).toString());
    cuedSoundObject.setProperty("op", "stopRecording");
    return ScriptAddFunctionToQueue(engine, cuedSoundObject);
}

QScriptValue GetObjectById(QScriptContext * context, QScriptEngine *engine)
{    
    QString id = context->argument(0).toString();    
    if (engine->globalObject().property("__dom").isValid())
    {
        if (engine->globalObject().property("__dom").property(id).isValid()) {
//            qDebug() << "GetObjectById id" << id << "exists!";
            return engine->globalObject().property("__dom").property(id);
        }
        else {
//            qDebug() << "GetObjectById id" << id << "undefined!";
            return engine->undefinedValue();
        }
    }
    else
    {
//        qDebug()<<"DOM is undefined!";
        return engine->undefinedValue();
    }
}

QScriptValue LoadNewAsset(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue globalObject = engine->globalObject();

    if (context->argumentCount()<2)
    {
        context->throwError("loadNewAsset() requires at least two arguments.");
        return QScriptValue();
    }

    if (!context->argument(0).isString())
    {
        context->throwError("Asset type must be a string.");
        return QScriptValue();
    }

    //release 60.0 - bugfix for some reason the spec calls to use the room object tag name, not the asset* tag name
    QString assetType = context->argument(0).toString().toLower();
    if (assetType.left(5) != "asset") {
        assetType = "asset" + assetType;
    }
    QScriptValue assetProps = context->argument(1);

    QScriptValue assetId = assetProps.property(QString("id"));

    if (!assetId.isValid() || !assetId.isString())
    {
        context->throwError("Unspecified or invalid Asset ID." + assetProps.property(QString("id")).toString());
        return QScriptValue();
    }

    QScriptValue vertexSrc = assetProps.property("vertex_src");

    if (assetType == "assetshader" && !vertexSrc.isValid())
    {
        context->throwError("The vertex_src property must be specified for a Shader asset.");
        return QScriptValue();
    }

    QScriptValue assetCallback = engine->undefinedValue();

    if (context->argumentCount()>3)
    {
        assetCallback = context->argument(3);

        if (!assetCallback.isFunction())
        {
            context->throwError("The third (optional) argument to loadNewAsset() must be a function.");
            return QScriptValue();
        }
    }

    QScriptValue newAssets = globalObject.property("__new_assets");
    if (!newAssets.isValid()) {
        newAssets = engine->newObject();
        globalObject.setProperty("__new_assets", newAssets);
    }

//    qDebug() << "LoadNewAsset" << assetProps.toVariant().toMap();
    QScriptValue asset = engine->newObject();

    asset.setProperty(QString("type"), assetType);
    asset.setProperty(QString("properties"), assetProps);
    asset.setProperty(QString("callback"), assetCallback);
    asset.setProperty(QString("state"), QScriptValue(0));

    newAssets.setProperty(assetId.toString(), asset);

    return asset;
}

QScriptValue VectorConstructor(QScriptContext * context, QScriptEngine * engine)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    if (context->argumentCount() == 0) {

    }
    else if (context->argumentCount() == 1) {
        QScriptValue vectorHashOrNumber = context->argument(0);
        if (vectorHashOrNumber.isNumber()) {
            x = y = z = vectorHashOrNumber.toNumber();

        } else {
            x = vectorHashOrNumber.property("x").toNumber();
            y = vectorHashOrNumber.property("y").toNumber();
            z = vectorHashOrNumber.property("z").toNumber();
        }
    }
    else if (context->argumentCount() >= 3) {
        x = context->argument(0).toNumber();
        y = context->argument(1).toNumber();
        z = context->argument(2).toNumber();
        if (context->argumentCount() >= 4) {
            w = context->argument(3).toNumber();
        }
    }

//    qDebug() << "VectorConstructor" << x << y << z << w;
    const QScriptValue result;
    //return engine->newQObject(result, new ScriptableVector(x, y, z, w), QScriptEngine::ScriptOwnership);
    return engine->newQObject(result, new ScriptableVector(x, y, z, w), QScriptEngine::QtOwnership);
}

QScriptValue Translate(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue v1 = context->argument(0);
    QScriptValue v2 = context->argument(1);
    float x, y, z;

    x = v1.property("x").toNumber() + v2.property("x").toNumber();
    y = v1.property("y").toNumber() + v2.property("y").toNumber();
    z = v1.property("z").toNumber() + v2.property("z").toNumber();

    const QScriptValue result;
    return engine->newQObject(result, new ScriptableVector(x, y, z), QScriptEngine::QtOwnership);
}

QScriptValue Equals(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue v1 = context->argument(0);
    QScriptValue v2 = context->argument(1);

    bool equal = (v1.property("x").toNumber() == v2.property("x").toNumber()) &&
            (v1.property("y").toNumber() == v2.property("y").toNumber()) &&
            (v1.property("z").toNumber() == v2.property("z").toNumber());

    return QScriptValue(engine, equal);
}

QScriptValue ScalarMultiply(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue v1 = context->argument(0);
    QScriptValue v2 = context->argument(1);
    float x, y, z;

    if (v2.isNumber()) {
        float n = v2.toNumber();
        v2 = engine->newQObject(v2, new ScriptableVector(n, n, n), QScriptEngine::QtOwnership);
    }

    x = v1.property("x").toNumber() * v2.property("x").toNumber();
    y = v1.property("y").toNumber() * v2.property("y").toNumber();
    z = v1.property("z").toNumber() * v2.property("z").toNumber();

    const QScriptValue result;
    return engine->newQObject(result, new ScriptableVector(x, y, z), QScriptEngine::QtOwnership);
}

QScriptValue Scale(QScriptContext * context, QScriptEngine * )
{
    QScriptValue v1 = context->argument(0);
    QScriptValue v2 = context->argument(1);

    if (v2.isNumber()) {
        const float f = context->argument(1).toNumber();
        v1.setProperty("x", v1.property("x").toNumber() * f);
        v1.setProperty("y", v1.property("y").toNumber() * f);
        v1.setProperty("z", v1.property("z").toNumber() * f);
    }

    return v1;
}

QScriptValue Copy(QScriptContext * context, QScriptEngine * )
{
    QScriptValue v1 = context->argument(0);

    if (context->argumentCount() == 4 && context->argument(1).isNumber() && context->argument(2).isNumber() && context->argument(3).isNumber()) {
        const float fx = context->argument(1).toNumber();
        const float fy = context->argument(2).toNumber();
        const float fz = context->argument(3).toNumber();
        v1.setProperty("x", fx);
        v1.setProperty("y", fy);
        v1.setProperty("z", fz);
    }
    else if (context->argumentCount() == 2 && context->argument(1).isNumber()) {
        const float f = context->argument(1).toNumber();
        v1.setProperty("x", f);
        v1.setProperty("y", f);
        v1.setProperty("z", f);
    }
    else {
        QScriptValue v1 = context->argument(0);
        QScriptValue v2 = context->argument(1);

        v1.setProperty("x", v2.property("x").toNumber());
        v1.setProperty("y", v2.property("y").toNumber());
        v1.setProperty("z", v2.property("z").toNumber());
    }

    return v1;
}

QScriptValue Add(QScriptContext * context, QScriptEngine * )
{
    QScriptValue v1 = context->argument(0);

//    qDebug() << "before" << v1.property("x").toNumber() << v1.property("y").toNumber() << v1.property("z").toNumber();
    if (context->argumentCount() == 4 && context->argument(1).isNumber() && context->argument(2).isNumber() && context->argument(3).isNumber()) {
//        qDebug() << "1";
        const float fx = context->argument(1).toNumber();
        const float fy = context->argument(2).toNumber();
        const float fz = context->argument(3).toNumber();
        v1.setProperty("x", v1.property("x").toNumber() + fx);
        v1.setProperty("y", v1.property("y").toNumber() + fy);
        v1.setProperty("z", v1.property("z").toNumber() + fz);
    }
    else if (context->argumentCount() == 2 && context->argument(1).isNumber()) {
//        qDebug() << "2";
        const float f = context->argument(1).toNumber();
        v1.setProperty("x", v1.property("x").toNumber() + f);
        v1.setProperty("y", v1.property("y").toNumber() + f);
        v1.setProperty("z", v1.property("z").toNumber() + f);
    }
    else {
//        qDebug() << "3";
        QScriptValue v1 = context->argument(0);
        QScriptValue v2 = context->argument(1);

        v1.setProperty("x", v1.property("x").toNumber() + v2.property("x").toNumber());
        v1.setProperty("y", v1.property("y").toNumber() + v2.property("y").toNumber());
        v1.setProperty("z", v1.property("z").toNumber() + v2.property("z").toNumber());
    }
//    qDebug() << "result" << v1.property("x").toNumber() << v1.property("y").toNumber() << v1.property("z").toNumber();

    return v1;
}

QScriptValue Cross(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue v1 = context->argument(0);
    QScriptValue v2 = context->argument(1);

    QVector3D cp = QVector3D::crossProduct(QVector3D(v1.property("x").toNumber(), v1.property("y").toNumber(), v1.property("z").toNumber()),
                                           QVector3D(v2.property("x").toNumber(), v2.property("y").toNumber(), v2.property("z").toNumber()));

    QScriptValue result;
    return engine->newQObject(result, new ScriptableVector(cp.x(), cp.y(), cp.z()), QScriptEngine::QtOwnership);
}

QScriptValue Normalized(QScriptContext * context, QScriptEngine * engine)
{
    QScriptValue v = context->argument(0);

    float x = v.property("x").toNumber();
    float y = v.property("y").toNumber();
    float z = v.property("z").toNumber();
    QVector3D vector = QVector3D(x, y, z).normalized();

    QScriptValue result;
    return engine->newQObject(result, new ScriptableVector(vector.x(), vector.y(), vector.z()), QScriptEngine::QtOwnership);
}

QScriptValue Distance(QScriptContext * context, QScriptEngine * engine)
{
    QVector3D v1 = VectorFromScriptValue(context->argument(0));
    QVector3D v2 = VectorFromScriptValue(context->argument(1));

    return QScriptValue(engine, v1.distanceToPoint(v2));
}

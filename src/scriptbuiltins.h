#ifndef SCRIPTBUILTINS_H
#define SCRIPTBUILTINS_H

#include <QtCore>
#include <QScriptEngine>
#include <set>

#include "scriptvalueconversions.h"
#include "scriptablexmlhttprequest.h"
#include "roomobject.h"

QScriptValue MyPrint(QScriptContext * context, QScriptEngine * engine);
QScriptValue MyConsoleLog(QScriptContext * context, QScriptEngine * engine);

QScriptValue PreventDefault(QScriptContext * context, QScriptEngine * engine);

QScriptValue OpenLink(QScriptContext * context, QScriptEngine * engine);
QScriptValue CloseLink(QScriptContext * context, QScriptEngine * engine);

QScriptValue CreateObject(QScriptContext * context, QScriptEngine * engine);
QScriptValue RemoveObject(QScriptContext * context, QScriptEngine * engine);
QScriptValue Children(QScriptContext * context, QScriptEngine * engine);

QScriptValue AddCookie(QScriptContext * context, QScriptEngine * engine);

QScriptValue XmlHttpRequestConstructor(QScriptContext * context, QScriptEngine * engine);

QScriptValue VectorConstructor(QScriptContext * context, QScriptEngine * engine);
QScriptValue Translate(QScriptContext * context, QScriptEngine * engine);
QScriptValue Equals(QScriptContext * context, QScriptEngine * engine);
QScriptValue ScalarMultiply(QScriptContext * context, QScriptEngine * engine);
QScriptValue Scale(QScriptContext * context, QScriptEngine * engine);
QScriptValue Copy(QScriptContext * context, QScriptEngine * engine);
QScriptValue Add(QScriptContext * context, QScriptEngine * engine);
QScriptValue Cross(QScriptContext * context, QScriptEngine * engine);
QScriptValue Normalized(QScriptContext * context, QScriptEngine * engine);
QScriptValue Distance(QScriptContext * context, QScriptEngine * engine);

QScriptValue RemoveKey(QScriptContext * context, QScriptEngine * engine);

QScriptValue ScriptAddFunctionToQueue(QScriptEngine * engine, QScriptValue cuedObject);
QScriptValue UniqueId(QScriptContext * context, QScriptEngine * engine);
QScriptValue Debug(QScriptContext * context, QScriptEngine * engine);

QScriptValue ScriptPlaySound(QScriptContext * context, QScriptEngine * engine);
QScriptValue ScriptSeekSound(QScriptContext * context, QScriptEngine * engine);
QScriptValue ScriptPauseSound(QScriptContext * context, QScriptEngine * engine);
QScriptValue ScriptStopSound(QScriptContext * context, QScriptEngine * engine);

QScriptValue ScriptPlayVideo(QScriptContext * context, QScriptEngine * engine);
QScriptValue ScriptSeekVideo(QScriptContext * context, QScriptEngine * engine);
QScriptValue ScriptPauseVideo(QScriptContext * context, QScriptEngine * engine);
QScriptValue ScriptStopVideo(QScriptContext * context, QScriptEngine * engine);

QScriptValue ScriptPlayRecording(QScriptContext * context, QScriptEngine * engine);
QScriptValue ScriptSeekRecording(QScriptContext * context, QScriptEngine * engine);
QScriptValue ScriptPauseRecording(QScriptContext * context, QScriptEngine * engine);
QScriptValue ScriptStopRecording(QScriptContext * context, QScriptEngine * engine);

QScriptValue GetObjectById(QScriptContext * context, QScriptEngine * engine);

QScriptValue LoadNewAsset(QScriptContext *context, QScriptEngine *engine);

struct ScriptLogMessage
{
    ScriptLogMessage() :
        output_to_chat(false)
    {
    }

    QString msg;
    bool output_to_chat;
};

class ScriptBuiltins{
public:
    static QList <ScriptLogMessage> script_print_log;
    static QString janus_queued_functions;    
};

#endif // SCRIPTBUILTINS_H

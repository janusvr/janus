#include "scriptablexmlhttprequest.h"

ScriptableXmlHttpRequest::ScriptableXmlHttpRequest(QScriptEngine * e)
{
//    qDebug() << "ScriptableXmlHttpRequest::ScriptableXmlHttpRequest()" << this << e;
    script_engine = e;
    connect(&webasset, SIGNAL(Finished()), this, SLOT(Finished()));
}

ScriptableXmlHttpRequest::~ScriptableXmlHttpRequest()
{
//    qDebug() << "ScriptableXmlHttpRequest::~ScriptableXmlHttpRequest()";
}

void ScriptableXmlHttpRequest::SetScriptValue(QScriptValue v)
{
    this_object = v;
}

QScriptValue ScriptableXmlHttpRequest::addEventListener(QScriptValue event, QScriptValue function)
{
//    qDebug() << "ScriptableXmlHttpRequest::addEventListener()";
    event_listeners[event.toString().toLower()].push_back(function);
    return QScriptValue();
}

QScriptValue ScriptableXmlHttpRequest::removeEventListener(QScriptValue event, QScriptValue function)
{
//    qDebug() << "ScriptableXmlHttpRequest::removeEventListener()";
    if (event_listeners.contains(event.toString().toLower())) {
        QList <QScriptValue> & l = event_listeners[event.toString().toLower()];
        for (int i=0; i<l.size(); ++i) {
            if (l[i].equals(function)) {
                l.removeAt(i);
                --i;
            }
        }
    }

    return QScriptValue();
}

QScriptValue ScriptableXmlHttpRequest::open(QScriptValue m, QScriptValue u)
{
//    qDebug() << "ScriptableXmlHttpRequest::open()" << m.toString() << u.toString();
    method = m.toString().toLower();
    url = u.toString();
    return QScriptValue();
}

QScriptValue ScriptableXmlHttpRequest::send()
{
//    qDebug() << "ScriptableXmlHttpRequest::send()";
    if (!QUrl(url).isLocalFile()) {
        webasset.SetURL(url);
        const QString m = method.toLower();
        if (m == "get") {
            webasset.Load(url);
        }
        else if (m == "post") {
            webasset.DoHTTPPost(url, post_data);
        }
        else if (m == "put") {
            webasset.DoHTTPPut(url, post_data);
        }
        else if (m == "delete") {
            webasset.DoHTTPDelete(url);
        }
        else {
            MathUtil::ErrorLog("Error: XMLHttpRequest with unsupported method: " + method);
        }
    }
    else {
        MathUtil::ErrorLog("Error: XMLHttpRequest with local file not allowed: " + url);
    }
    return QScriptValue();
}

QScriptValue ScriptableXmlHttpRequest::send(QScriptValue s)
{
    post_data = s.toString().toLatin1();
    return send();
}

void ScriptableXmlHttpRequest::Finished()
{
    responseText = webasset.GetData();
//    qDebug() << "ScriptableXmlHttpRequest::Finished()" << responseText;

    if (event_listeners.contains("load")) {
        QList <QScriptValue> & l = event_listeners["load"];
        for (int i=0; i<l.size(); ++i) {
//            qDebug() << "load event listeners calling:" << l[i].toString();
            if (script_engine) {
                QScriptValueList args;
                QScriptValue ev = script_engine->newObject();
                ev.setProperty("target", this_object);
                args << ev;
                l[i].call(script_engine->globalObject(), args);
            }
        }
    }
}

#ifndef SCRIPTABLEXMLHTTPREQUEST_H
#define SCRIPTABLEXMLHTTPREQUEST_H

#include <QtCore>
#include <QtScript>
#include <QScriptValue>
#include <QScriptable>

#include "webasset.h"

class ScriptableXmlHttpRequest : public QObject, protected QScriptable
{
    Q_OBJECT

    Q_PROPERTY(QString responseText MEMBER responseText)

public:

    explicit ScriptableXmlHttpRequest(QScriptEngine * e);
    ~ScriptableXmlHttpRequest();

    void SetScriptValue(QScriptValue v);

    Q_INVOKABLE QScriptValue addEventListener(QScriptValue, QScriptValue);
    Q_INVOKABLE QScriptValue removeEventListener(QScriptValue, QScriptValue);
    Q_INVOKABLE QScriptValue open(QScriptValue, QScriptValue);
    Q_INVOKABLE QScriptValue send();
    Q_INVOKABLE QScriptValue send(QScriptValue);

public slots:

    void Finished();

private:

    QString responseText;

    QMap <QString, QList <QScriptValue> > event_listeners;
    QString method;
    QString url;
    QByteArray post_data;

    QPointer <QScriptEngine> script_engine;
    QScriptValue this_object;

    WebAsset webasset;

};

Q_DECLARE_METATYPE(ScriptableXmlHttpRequest*)

#endif // SCRIPTABLEXMLHTTPREQUEST_H

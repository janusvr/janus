#include "scriptvalueconversions.h"

QScriptValue DOMNodeMapToScriptValue(QScriptEngine* eng, const QMap<QString, DOMNode* > & map)
{
    QScriptValue a = eng->newObject();
    QMap<QString, DOMNode* >::const_iterator it(map.begin());
    for(; it != map.end(); ++it)
    {
        QString prop = it.key();
        prop.replace(' ', '_');

        QScriptValue obj = eng->newObject();
        a.setProperty(prop, eng->newQObject(obj, map[prop]));        
    }
    return a;
}

void DOMNodeMapFromScriptValue(const QScriptValue & value, QMap<QString, DOMNode *> &map)
{
    QScriptValueIterator itr(value);
    while(itr.hasNext())
    {
        itr.next();
        map[itr.name()] = qscriptvalue_cast<DOMNode* >(itr.value());
    }
}

QScriptValue PlayerToScriptValue(QScriptEngine * eng, const QPointer<DOMNode> & obj)
{
    return eng->newQObject(obj, QScriptEngine::QtOwnership, QScriptEngine::AutoCreateDynamicProperties);
}

void PlayerFromScriptValue(const QScriptValue & obj, QPointer<DOMNode> & o)
{
    QScriptValueIterator itr(obj);
    while (itr.hasNext()) {
        itr.next();

        const QVariant value = itr.value().toVariant();
        o->setProperty(itr.name().toStdString().c_str(), value);
    }
}

QScriptValue VectorToScriptValue(const QVector3D & vector, QScriptEngine* engine,
                                 const QString x_fieldname, const QString y_fieldname, const QString z_fieldname)
{
    QScriptValue vectorObject = engine->newObject();

    vectorObject.setProperty(x_fieldname, vector.x());
    vectorObject.setProperty(y_fieldname, vector.y());
    vectorObject.setProperty(z_fieldname, vector.z());

    return vectorObject;
}

QVector3D VectorFromScriptValue(const QScriptValue value,
                                const QString x_fieldname, const QString y_fieldname, const QString z_fieldname)
{
    QVector3D vec;

    vec.setX(value.property(x_fieldname).toNumber());
    vec.setY(value.property(y_fieldname).toNumber());
    vec.setZ(value.property(z_fieldname).toNumber());

    return vec;
}

QScriptValue EventPreventDefault(QScriptContext * context, QScriptEngine * )
{
    context->thisObject().setProperty("_defaultPrevented", QScriptValue(true)); //42.3 - call thisObject not callee
    return QScriptValue();
}

QScriptValue KeyEventToScriptValue(QScriptEngine * eng, const QKeyEvent * e)
{
    QScriptValue eventObject = eng->newObject();
    eventObject.setProperty("keyCode", QScriptValue(QString(e->key())));
    eventObject.setProperty("_defaultPrevented", QScriptValue(false));
    eventObject.setProperty("preventDefault", eng->newFunction(EventPreventDefault));
    return eventObject;
}

QScriptValue DOMNodeToScriptValue(QScriptEngine *engine, const QPointer <DOMNode> &node)
{
    return engine->newQObject(node, QScriptEngine::QtOwnership, QScriptEngine::AutoCreateDynamicProperties);
}

void DOMNodeFromScriptValue(const QScriptValue &obj, QPointer <DOMNode> &node)
{
//    qDebug() << "DOMNodeFromScriptValue" << (qvariant_cast<ScriptableVector *>(obj.toVariant().toMap()["pos"]))->toQVector3D();    
    //special condition
    if (obj.property("js_id").isValid() && obj.property("js_id").toString()==QString("__room")) {
        node->SetType(TYPE_ROOM);
    }
    node->SetProperties(obj.toVariant().toMap());
//    qDebug() << "DOMNodeFromScriptValue" << node->GetS("js_id");
}

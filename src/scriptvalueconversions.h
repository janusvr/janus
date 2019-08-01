#ifndef SCRIPTVALUECONVERSIONS_H
#define SCRIPTVALUECONVERSIONS_H

#include <QScriptEngine>

#include "mathutil.h"
#include "domnode.h"

QScriptValue DOMNodeMapToScriptValue(QScriptEngine * eng, const QMap<QString, DOMNode *> &map);
void DOMNodeMapFromScriptValue(const QScriptValue & value, QMap<QString, DOMNode* > & map);

QScriptValue PlayerToScriptValue(QScriptEngine * eng, const QPointer<DOMNode> & obj);
void PlayerFromScriptValue(const QScriptValue & obj, QPointer<DOMNode> & o);

QScriptValue VectorToScriptValue(const QVector3D & vector, QScriptEngine * engine,
                                 const QString x_fieldname = "x", const QString y_fieldname = "y", const QString z_fieldname = "z");
QVector3D VectorFromScriptValue(const QScriptValue value,
                                const QString x_fieldname = "x", const QString y_fieldname = "y", const QString z_fieldname = "z");

QScriptValue EventPreventDefault(QScriptContext * context, QScriptEngine * engine);
QScriptValue KeyEventToScriptValue(QScriptEngine * eng, const QKeyEvent * e);

QScriptValue DOMNodeToScriptValue(QScriptEngine *engine, const QPointer <DOMNode> &node);
void DOMNodeFromScriptValue(const QScriptValue &obj, QPointer <DOMNode> &node);

#endif // SCRIPTVALUECONVERSIONS_H

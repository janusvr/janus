#include "scriptablevector.h"

ScriptableVector::ScriptableVector(QObject * parent) :
    QObject(parent),
    vector_data()
{
}

ScriptableVector::ScriptableVector(float x, float y, float z, QObject * parent) :
    QObject(parent),
    vector_data(x, y, z, 1.0f)
{
//    qDebug() << "Calling SCRIPTABLE VECTOR constructor with" << x << y << z << "1.0 hardcoded";
}

ScriptableVector::ScriptableVector(float x, float y, float z, float w, QObject * parent) :
    QObject(parent),
    vector_data(x, y, z, w)
{
//    qDebug() << "Calling SCRIPTABLE VECTOR constructor with" << x << y << z << w;
}

ScriptableVector::~ScriptableVector()
{

}

ScriptableVector& ScriptableVector::operator= (const ScriptableVector & source)
{
    vector_data = source.vector_data;
//    qDebug() << "Calling SCRIPTABLE VECTOR assignment operator!" << vector_data << source.vector_data;
    return *this;
}

bool ScriptableVector::operator== (const ScriptableVector & rhs)
{
    return (vector_data == rhs.vector_data);
}

bool ScriptableVector::operator!= (const ScriptableVector & rhs)
{
    return (vector_data != rhs.vector_data);
}

void ScriptableVector::Copy(const ScriptableVector * v)
{
    if (v == NULL) {
        return;
    }

    vector_data = v->vector_data;
}

void ScriptableVector::SetX(float new_x)
{
    if (std::isfinite(new_x)) {
        vector_data.setX(new_x);
    }
}

void ScriptableVector::SetY(float new_y)
{
    if (std::isfinite(new_y)) {
        vector_data.setY(new_y);
    }
}

void ScriptableVector::SetZ(float new_z)
{
    if (std::isfinite(new_z)) {
        vector_data.setZ(new_z);
    }
}

void ScriptableVector::SetW(float new_w)
{
    if (std::isfinite(new_w)) {
        vector_data.setW(new_w);
    }
}

QString ScriptableVector::toString()
{
    return QString("Vector(%1, %2, %3, %4)").arg(GetX()).arg(GetY()).arg(GetZ()).arg(GetW());
}

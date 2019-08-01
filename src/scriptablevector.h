#ifndef SCRIPTABLEVECTOR_H
#define SCRIPTABLEVECTOR_H

#include <QtCore>
#include <QVector4D>
#include <QVector3D>
#include <QScriptValue>

class ScriptableVector : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float x READ GetX WRITE SetX)
    Q_PROPERTY(float y READ GetY WRITE SetY)
    Q_PROPERTY(float z READ GetZ WRITE SetZ)
    Q_PROPERTY(float w READ GetW WRITE SetW)

public:
    explicit ScriptableVector(QObject * parent = 0);
    explicit ScriptableVector(float x, float y, float z, QObject * parent = 0);
    explicit ScriptableVector(float x, float y, float z, float w, QObject * parent = 0);
    ~ScriptableVector();

    //Copy constructor
    ScriptableVector(const ScriptableVector & source) :
        QObject()
    {
        //qDebug() << "Calling SCRIPTABLE VECTOR copy constructor!";
        vector_data = source.vector_data;
    }

    //Assignment operator
    ScriptableVector& operator= (const ScriptableVector & source);

    //Comparison operators
    bool operator== (const ScriptableVector & rhs);
    bool operator!= (const ScriptableVector & rhs);

    void Copy(const ScriptableVector * v);

    void SetX(float new_x);
    inline float GetX() const { return vector_data.x(); }

    void SetY(float new_y);
    inline float GetY() const { return vector_data.y(); }

    void SetZ(float new_z);
    inline float GetZ() const { return vector_data.z(); }

    void SetW(float new_w);
    inline float GetW() const { return vector_data.w(); }

//    inline QVector3D * GetQVector3D() { return &vector_data; }
    inline QVector3D toQVector3D() { return vector_data.toVector3D(); }

    inline QVector4D toQVector4D() { return vector_data; }

    inline void SetFromOther(const ScriptableVector & v)
    {
        SetX(v.GetX());
        SetY(v.GetY());
        SetZ(v.GetZ());
        SetW(v.GetW());
    }
    inline void SetFromOther(const QVector4D & v) {
        vector_data = v;
    }
    inline void SetFromOther(const QVector3D & v) {
        vector_data = v;
    }

    Q_INVOKABLE QString toString();

private:
    QVector4D vector_data;
};

Q_DECLARE_METATYPE(ScriptableVector *)

#endif // SCRIPTABLEVECTOR_H

#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H

#include <qopengl.h>
#include <QHash>
#include <QVector>
#include <QVector4D>
#include <QVector3D>
#include <memory>

class LightContainer;

class Light
{
    friend class LightContainer;
public:
    Light(QVector3D p_color, float p_angle, QVector3D p_pos, float p_range, QVector3D p_dir, float p_exponent);
    Light();

    void SetColor(QVector3D p_color); // Intensity is the length of the color vector
    QVector3D GetColor() const;

    void SetConeAngle(float p_angle); // disabled = -1, omni = 0, 0 < spot < 1, directional = 1
    float GetConeAngle() const;

    void SetPos(QVector3D p_pos);
    QVector3D GetPos() const;

    void SetRange(float p_range);
    float GetRange() const;

    void SetDir(QVector3D p_dir); // Only used for spot lights
    QVector3D GetDir() const;

    void SetExponent(float p_exponent); // Only used for spot lights
    float GetExponent() const;

private:
    QVector4D m_col_angle; // XYZ color, W cone_angle
    QVector4D m_pos_range; // XYZ pos, W range
    QVector4D m_dir_exponent; //  XYZ direction, W cone_exponent
};

class LightContainer
{
public:
    LightContainer()
    {

    }

    ~LightContainer()
    {

    }

   /* LightContainer(const LightContainer & p_move)
        : m_lights(p_move.m_lights)
    {

    }*/

    //59.5 - Compile bug on Linux declaring this
//    LightContainer(LightContainer&& p_move)
//        : m_lights(std::move(p_move.m_lights))
//    {

//    }

    //59.5 - Compile bug on Linux declaring this
//    LightContainer& operator=(LightContainer&& p_move)
//    {
//        m_lights = std::move(p_move.m_lights);
//        return *this;
//    }

    QVector<Light> m_lights;
};

class LightManager
{
public:

    static LightManager * GetSingleton()
    {
        static LightManager * singleton = new LightManager();
        return singleton;
    }

private:
    LightManager();
};

#endif // LIGHTMANAGER_H

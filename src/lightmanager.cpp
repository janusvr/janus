#include "lightmanager.h"

LightManager::LightManager()
{

}



Light::Light(QVector3D p_color, float p_angle, QVector3D p_pos, float p_range, QVector3D p_dir, float p_exponent)
    : m_col_angle(p_color.x(), p_color.y(), p_color.z(), p_angle),
      m_pos_range(p_pos.x(), p_pos.y(), p_pos.z(), p_range),
      m_dir_exponent(p_dir.x(), p_dir.y(), p_dir.z(), p_exponent)
{

}

Light::Light()
    : m_col_angle(0.0f, 0.0f, 0.0f, -1.0f),
      m_pos_range(0.0f, 0.0f, 0.0f, 0.5f),
      m_dir_exponent(0.0f, -1.0f, 0.0f, 1.0f)
{

}

void Light::SetColor(QVector3D p_color)
{
    m_col_angle.setX(p_color.x());
    m_col_angle.setY(p_color.y());
    m_col_angle.setZ(p_color.z());
}

QVector3D Light::GetColor() const
{
    return m_col_angle.toVector3D();
}

void Light::SetConeAngle(float p_angle)
{
    m_col_angle.setW(p_angle);
}

float Light::GetConeAngle() const
{
    return m_col_angle.w();
}

void Light::SetPos(QVector3D p_pos)
{
    m_pos_range.setX(p_pos.x());
    m_pos_range.setY(p_pos.y());
    m_pos_range.setZ(p_pos.z());
}

QVector3D Light::GetPos() const
{
    return m_pos_range.toVector3D();
}

void Light::SetRange(float p_range)
{
    m_pos_range.setW(p_range);
}

float Light::GetRange() const
{
    return m_pos_range.w();
}

void Light::SetDir(QVector3D p_dir)
{
    m_dir_exponent.setX(p_dir.x());
    m_dir_exponent.setY(p_dir.y());
    m_dir_exponent.setZ(p_dir.z());
}

QVector3D Light::GetDir() const
{
    return m_dir_exponent.toVector3D();
}

void Light::SetExponent(float p_exponent)
{
    m_dir_exponent.setW(p_exponent);
}

float Light::GetExponent() const
{
    return m_dir_exponent.w();
}

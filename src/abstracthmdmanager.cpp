#include "abstracthmdmanager.h"

AbstractHMDManager::AbstractHMDManager() :
    m_using_openVR(false),
    m_color_texture_id(0),
    m_near_clip(0.01f),
    m_far_clip(1000.0f),
    m_avatar_near_clip(0.01f),
    m_avatar_far_clip(1000.0f)
{
}

AbstractHMDManager::~AbstractHMDManager()
{
}

void AbstractHMDManager::SetNearDist(const float f, bool const p_is_avatar)
{
    (p_is_avatar == true)
            ? m_avatar_near_clip = f
            : m_near_clip = f;
}

float AbstractHMDManager::GetNearDist(bool const p_is_avatar) const
{
    return (p_is_avatar == true)
            ? m_avatar_near_clip
            : m_near_clip;
}

void AbstractHMDManager::SetFarDist(const float f, bool const p_is_avatar)
{
    (p_is_avatar == true)
            ? m_avatar_far_clip = f
            : m_far_clip = f;
}

float AbstractHMDManager::GetFarDist(bool const p_is_avatar) const
{
    return (p_is_avatar == true)
            ? m_avatar_far_clip
            : m_far_clip;
}

const QMatrix4x4& AbstractHMDManager::GetEyeViewMatrix(const int p_eye_index) const
{
    return m_eye_view_matrices[p_eye_index];
}

const QMatrix4x4& AbstractHMDManager::GetEyeProjectionMatrix(const int p_eye_index, const bool p_is_avatar) const
{
    return m_eye_projection_matrices[(p_is_avatar) ? p_eye_index + 2 : p_eye_index];
}

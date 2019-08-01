#include "spinanimation.h"

QPointer <AssetObject> SpinAnimation::plane_obj;
QVector <QPointer <AssetImage> > SpinAnimation::loading_imgs;
float SpinAnimation::icon_scale = 0.5f;

SpinAnimation::SpinAnimation()
{
}

SpinAnimation::~SpinAnimation()
{
}

void SpinAnimation::initializeGL()
{
    if (plane_obj.isNull()) {
        plane_obj = new AssetObject();
        plane_obj->SetSrc(MathUtil::GetApplicationURL(), "assets/primitives/plane.obj");
        plane_obj->Load();
    }

    if (loading_imgs.isEmpty()) {
        for (int i=0; i<17; ++i) {
            QPointer <AssetImage> img = new AssetImage();
            img->SetSrc(MathUtil::GetApplicationURL(), "assets/load/"+QString::number(i) + ".png");
            img->Load();           
            loading_imgs.push_back(img);
        }
    }
}

float SpinAnimation::GetIconScale()
{
    return icon_scale;
}

void SpinAnimation::DrawPlaneGL(QPointer <AssetShader> shader, const QColor col)
{
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }

    if (plane_obj) {
        shader->SetOverrideTexture(true);
        plane_obj->DrawGL(shader, col);
        shader->SetOverrideTexture(false);
    }
}

void SpinAnimation::DrawIconGL(QPointer <AssetShader> shader, const bool billboard, TextureHandle* texture0, const QColor color)
{        
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }

    if (plane_obj) {
        const bool clip_plane = shader->GetUseClipPlane();
        const bool lighting = shader->GetUseLighting();
        shader->SetUseClipPlane(false);
        shader->SetUseLighting(false);
        shader->SetAmbient(QVector3D(1,1,1));
        shader->SetDiffuse(QVector3D(1,1,1));
        shader->SetEmission(QVector3D(0,0,0));
        shader->SetShininess(20.0f);
        shader->SetSpecular(QVector3D(0,0,0));
        shader->SetUseTextureAll(false);
        shader->SetUseTexture(0, texture0 != nullptr);

        MathUtil::PushModelMatrix();
        if (billboard) {
            const QVector3D p = MathUtil::ModelMatrix().column(3).toVector3D();
            QMatrix4x4 m = MathUtil::ViewMatrix().transposed();
            m.setColumn(3, QVector4D(p, 1));
            m.setRow(3, QVector4D(0,0,0,1));
            m.scale(icon_scale);
            MathUtil::LoadModelMatrix(m);
        }

        shader->UpdateObjectUniforms();

        RendererInterface::m_pimpl->BindTextureHandle(0, texture0);
        shader->SetOverrideTexture(true);
        plane_obj->DrawGL(shader, color);
        shader->SetOverrideTexture(false);

        MathUtil::PopModelMatrix();

        shader->SetUseTexture(0, false);
        shader->SetUseClipPlane(clip_plane);
        shader->SetUseLighting(lighting);
    }
}

void SpinAnimation::DrawGL(QPointer <AssetShader> shader, const float value, const bool billboard)
{
    const int s = loading_imgs.size();
    if (s > 0) {
        const int tex_index = int(value * float(s-1)) % s;
        if (tex_index >= 0 && tex_index < s && loading_imgs[tex_index]) {
            auto tex = loading_imgs[tex_index]->GetTextureHandle(true);
            DrawIconGL(shader, billboard, tex, QColor(255,255,255));
        }
    }
}

void SpinAnimation::UpdateAssets()
{
    if (plane_obj) {
        plane_obj->Update();
        plane_obj->UpdateGL();
    }
    for (QPointer <AssetImage> & a : loading_imgs) {
        if (a) {
            a->UpdateGL();
        }
    }
}

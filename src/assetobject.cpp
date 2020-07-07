#include "assetobject.h"

AssetObject::AssetObject()
{    
    //qDebug() << "AssetObject::AssetObject" << props;
    props->SetType(TYPE_ASSETOBJECT);
    geom = new Geom();
    tex_url_str = QVector <QString> (ASSETSHADER_NUM_TEXTURES, QString(""));    
    img_error = 0;       
    time.start();
}

AssetObject::~AssetObject()
{
    //qDebug() << "AssetObject::~AssetObject()";
    if (geom) {
        delete geom;
    }
}

void AssetObject::SetProperties(const QVariantMap & d)
{
    Asset::SetProperties(d);

    if (d.contains("cubemap_radiance")) {
        SetTextureFile(d["cubemap_radiance"].toString(), 0);
    }
    if (d.contains("cubemap_irradiance")) {
        SetTextureFile(d["cubemap_irradiance"].toString(), 0);
    }
    if (d.contains("tex")) {
        SetTextureFile(d["tex"].toString(), 0);
    }
    for (unsigned int i=0; i<ASSETSHADER_NUM_TEXTURES; ++i) {
        QString tagname = "tex" + QString::number(i);
        if (d.contains(tagname)) {            
            SetTextureFile(d[tagname].toString(), i);
        }
    }
    if (d.contains("mtl")) {
        QString mtl = d["mtl"].toString();
        if (!mtl.isEmpty())
        {
            SetMTLFile(d["mtl"].toString());
        }
    }
    if (d.contains("tex_clamp")) {
        SetTextureClamp(d["tex_clamp"].toBool());
    }
    if (d.contains("tex_linear")) {                
        SetTextureLinear(d["tex_linear"].toBool());
    }
    if (d.contains("tex_compress")) {
        SetTextureCompress(d["tex_compress"].toBool());
    }
    if (d.contains("tex_mipmap")) {
        SetTextureMipmap(d["tex_mipmap"].toBool());
    }
    if (d.contains("tex_premultiply")) {
        geom->SetTexturePreMultiply(d["tex_premultiply"].toBool());
    }
    if (d.contains("tex_alpha")) {
        geom->SetTextureAlphaType(d["tex_alpha"].toString());
    }
    if (d.contains("tex_colorspace")) {
        geom->SetTextureAlphaType(d["tex_colorspace"].toString());
    }

    bool center = false;
    if (d.contains("center") && d["center"].toString() == "true") {
        center = true;
    }
    geom->SetCenter(center);
}

void AssetObject::SetTextureFile(const QString & tex, const unsigned int index)
{    
    //qDebug() << "AssetObject::SetTextureFile() - URL" << tex;
    if (index < ASSETSHADER_NUM_TEXTURES && tex.length() > 0) {
        geom->SetUsesTexFile(true);
        props->SetTex(tex, index);
        tex_url_str[index] = QUrl(props->GetBaseURL()).resolved(tex).toString();
    }
}

void AssetObject::SetMTLFile(const QString & mtl)
{    
    //qDebug() << "AssetObject::SetMTLFile" << mtl << QUrl(props->GetBaseURL()).resolved(mtl).toString();
    props->SetMTL(mtl);
    geom->SetMTLFile(QUrl(props->GetBaseURL()).resolved(mtl).toString());
}

void AssetObject::Load()
{   
    //qDebug() << "AssetObject::Load()" << props->GetSrcURL() << this;
    SetStarted(true);

    for (int i=0; i<tex_url_str.size(); ++i) {
        if (!tex_url_str[i].isEmpty()) {
            geom->SetMaterialTexture(tex_url_str[i], i);
            //qDebug() << "setting" << base_url_str << tex_url_str[i] << i;
        }
    }

    geom->SetPath(props->GetSrcURL());
    if ((!props->GetSrcURL().isEmpty() || geom->GetHasMeshData()) && !geom->GetStarted()) {        
        QtConcurrent::run(geom.data(), &Geom::Load);
    }
    else {
        //qDebug() << "AssetObject::Load() ERROR: Tried to load with an empty url" << props->GetID() << props->GetSrc();
        SetLoaded(true);
        SetProcessed(true);
        SetFinished(true);
        this->SetStatusCode(404);
    }
}

void AssetObject::Unload()
{    
    geom->Unload();
}

bool AssetObject::UpdateGL()
{
    return geom->UpdateGL();
}

void AssetObject::Update()
{
    //qDebug() << "AssetObject::Update()" << props->GetSrcURL() << this;
    if (geom) {
        geom->Update();
    }
}

void AssetObject::DrawGL(QPointer <AssetShader> shader, const QColor col)
{
    //qDebug() << "AssetObject::DrawGL" << shader << src_url;
    if (shader == NULL || !shader->GetCompiled()) {
        return;
    }   

    //qDebug() << "AssetObject::DrawGL" << GetS("src") << GetError();
    if (GetError()) {
        if (img_error.isNull()) {
            img_error = new AssetImage();
            img_error->CreateFromText(QString("<p align=\"center\">error loading: ") + props->GetSrcURL() + QString("</p>"), 24, true, QColor(255,128,192), QColor(25,25,128), 1.0f, 256, 256, true);
        }

        shader->SetUseTextureAll(false);
        shader->SetUseTexture(0, true);
        shader->SetConstColour(QVector4D(col.redF(), col.greenF(), col.blueF(), col.alphaF()));

        const float iw = 1.0f;
        const float ih = iw * img_error->GetAspectRatio();

        Renderer * renderer = Renderer::m_pimpl;
        auto tex_id = img_error->GetTextureHandle(true);
        renderer->BindTextureHandle(0, tex_id);

        MathUtil::PushModelMatrix();
        MathUtil::ModelMatrix().scale(iw * 0.5f, ih * 0.5f, 0.5f);
        shader->UpdateObjectUniforms();
        AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                                renderer->GetTexturedCube3PrimCount(),
                                0,
                                0,
                                0,
                                renderer->GetTexturedCube3VAO(),
                                shader->GetProgramHandle(),
                                shader->GetFrameUniforms(),
                                shader->GetRoomUniforms(),
                                shader->GetObjectUniforms(),
                                shader->GetMaterialUniforms(),
                                renderer->GetCurrentlyBoundTextures(),
                                renderer->GetDefaultFaceCullMode(),
                                renderer->GetDepthFunc(),
                                renderer->GetDepthMask(),
                                renderer->GetStencilFunc(),
                                renderer->GetStencilOp(),
                                renderer->GetColorMask());
        renderer->PushAbstractRenderCommand(a);
        MathUtil::PopModelMatrix();
        shader->SetUseTexture(0, false);
    }
    else {
        geom->DrawGL(shader, col);
    }
}

QString AssetObject::GetTexURL(const unsigned int index) const
{
    if (index < ASSETSHADER_NUM_TEXTURES && index < (unsigned int)(tex_url_str.size())) {
        return tex_url_str[index];
    }
    else {
        return QString();
    }
}

void AssetObject::SetTextureAlphaType(QString p_alpha_type)
{
    geom->SetTextureAlphaType(p_alpha_type);
}

QString AssetObject::GetTextureAlphaType()
{
    return geom->GetTextureAlphaType();
}

void AssetObject::SetTextureClamp(const bool b)
{
    geom->SetTextureClamp(b);
}

bool AssetObject::GetTextureClamp()
{
    return geom->GetTextureClamp();
}

void AssetObject::SetTextureLinear(const bool b)
{
    geom->SetTextureLinear(b);
}

bool AssetObject::GetTextureLinear()
{
    return geom->GetTextureLinear();
}

void AssetObject::SetTextureCompress(const bool b)
{    
    geom->SetTextureCompress(b);
}

bool AssetObject::GetTextureCompress()
{
    return geom->GetTextureCompress();
}

void AssetObject::SetTextureMipmap(const bool b)
{
    geom->SetTextureMipmap(b);
}

bool AssetObject::GetTextureMipmap()
{
    return geom->GetTextureMipmap();
}

QVector3D AssetObject::GetBBoxMin()
{
    return geom->GetBBoxMin();
}

QVector3D AssetObject::GetBBoxMax()
{
    return geom->GetBBoxMax();
}

bool AssetObject::GetFinished()
{
    return geom->GetReady();
}

bool AssetObject::GetTexturesFinished()
{
    return geom->GetTexturesReady();
}

QPointer <Geom> AssetObject::GetGeom()
{
    return geom;
}

int AssetObject::GetNumTris()
{
    return geom->GetNumTris();
}

float AssetObject::GetProgress()
{        
    //qDebug() << "AssetObject::GetProgress()" << this << this->GetS("src") << geom->GetProgress() << geom->GetTextureProgress();
    return GetError() ? 1.0f : ((geom->GetProgress() + geom->GetTextureProgress()) * 0.5f);
}


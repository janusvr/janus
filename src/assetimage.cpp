#include "assetimage.h"

int AssetImage::anisotropic_max = 0;
QPointer<TextureHandle> AssetImage::null_cubemap_tex_handle = nullptr;
QPointer<TextureHandle> AssetImage::null_image_tex_handle = nullptr;
QVector<ContentImporter*> AssetImage::importers;

void AssetImage::InitializeImporters()
{
    AssetImage::importers = QVector<ContentImporter*>(2);
    AssetImage::importers[0] = new TextureImporterQImage();    
    AssetImage::importers[1] = new TextureImporterCMFT();
    //AssetImage::importers[2] = new TextureImporterGLI(); // TODO: for now GLI is just a plugin in the end, if we fail to load in any other way
}

AssetImage::AssetImage() :    
    load_gli(false),
    tex_index(0),
    aspect(0.7f),
    transparent(false),
    max_img_resolution(1024),    
    next_frame_time(-1)
{
    props->SetType(TYPE_ASSETIMAGE);

    InitializeImporters();
    Unload();   

    time.start();
}

AssetImage::~AssetImage()
{
    Unload();
}

QPointer<QObject> AssetImage::LoadAssetImage(const QByteArray& buffer, QString extension, QPointer <DOMNode> props, bool& is_gli)
{
    int count = importers.size();
    ContentImporter* importer = nullptr;

    for (int i = 0; i < count; i++)
    {
        ContentImporter* imp = importers[i];
        bool result;

        //59.4 - attempt to see if we can import
        try {
            result = imp->CanImport(buffer, extension, props);
        }
        catch (int e) {
            qDebug() << "AssetImage::LoadAssetImage - exception calling ContentImporter::CanImport(), code:" << e;
        }

        if (result)
        {
            importer = imp;
            break;
        }
    }

    if (QString::compare(extension, "dds", Qt::CaseInsensitive) == 0 ||
        QString::compare(extension, "ktx", Qt::CaseInsensitive) == 0)
    {
        is_gli = true;
        return QPointer<QObject>();
    }
    else if (QString::compare(extension, "hdr", Qt::CaseInsensitive) == 0)
    {
        importer->Import(buffer, props);
        is_gli = true;
        return QPointer<QObject>();
    }
    else if (importer == nullptr)
    {
        // force the first importer, which is QImage
        importer = importers[0];
    }

    return importer->Import(buffer, props);
}

void AssetImage::initializeGL()
{
    if (AssetImage::null_image_tex_handle == nullptr)
    {        
        QByteArray data;
        data.resize(64 * 64 * 4);
        for (int x = 0; x < 64; ++x)
        {
            for (int y = 0; y < 64; ++y)
            {
                int index = (x + (y * 64)) * 4;
                if ((x / 16 + y / 16) % 2 == 0)
                {
                    data[index]         = (unsigned char)0x99;
                    data[index + 1]     = (unsigned char)0x99;
                    data[index + 2]     = (unsigned char)0x99;
                    data[index + 3]     = (unsigned char)0xff;
                }
                else
                {
                    data[index]         = (unsigned char)0x66;
                    data[index + 1]     = (unsigned char)0x66;
                    data[index + 2]     = (unsigned char)0x66;
                    data[index + 3]     = (unsigned char)0xff;
                }
            }
        }

        QPointer<AssetImageData> null_image_data = QPointer<AssetImageData>(new AssetImageData());
        null_image_data->SetWidth(64);
        null_image_data->SetHeight(64);
        null_image_data->SetPixelSize(4);
        null_image_data->format = QImage::Format::Format_RGBX8888;
        null_image_data->SetFrameData(0, data, true);
        null_image_data->SetTotalTextures(1);
        null_image_data->SetSource("null_image");

        // will use the function in the child to transfer the data to the GPU
        //auto null_image_tex_handle = RendererInterface::m_pimpl->CreateTextureFromAssetImageData(null_image_data, true, true, true, false, TextureHandle::ALPHA_TYPE::NONE, TextureHandle::COLOR_SPACE::SRGB);
//        qDebug() << "m_pimpl" << RendererInterface::m_pimpl;
        AssetImage::null_image_tex_handle = RendererInterface::m_pimpl->CreateTextureFromAssetImageData(null_image_data, true, true, true, false, TextureHandle::ALPHA_TYPE::NONE, TextureHandle::COLOR_SPACE::SRGB);
        for (uint32_t i = 0; i < ASSETSHADER_NUM_TEXTURES; ++i)
        {
            RendererInterface::m_pimpl->BindTextureHandle(i, AssetImage::null_image_tex_handle);
        }
    }

    if (AssetImage::null_cubemap_tex_handle == nullptr)
    {
        const QString file_path = MathUtil::GetApplicationPath() + QString("assets/skybox/null_cubemap.dds");

        QFile face_file(file_path);
        const bool face_file_open_result = face_file.open(QIODevice::ReadOnly);
        if (face_file_open_result)
        {
            QByteArray file_data = face_file.readAll();
            if (!file_data.isNull() && !file_data.isEmpty())
            {
                AssetImage::null_cubemap_tex_handle = RendererInterface::m_pimpl->CreateTextureFromGLIData(file_data, true, true, true, TextureHandle::ALPHA_TYPE::NONE, TextureHandle::COLOR_SPACE::SRGB);
            }
            face_file.close();
        }

        for (uint32_t i = 0; i < ASSETSHADER_NUM_CUBEMAPS; ++i)
        {
            RendererInterface::m_pimpl->BindTextureHandle(i + ASSETSHADER_NUM_TEXTURES, AssetImage::null_cubemap_tex_handle);
        }
    }
}

void AssetImage::CreateFromData(const QByteArray & b)
{
//    qDebug() << "AssetImage::CreateFromData" << b.size();
    SetData(b);
    SetStarted(true);
    SetLoaded(true);
    SetProcessing(false);
    SetProcessed(false);
    SetFinished(false);
}

void AssetImage::CreateFromText(const QString & s, const float font_size, const bool add_markup, const QColor & text_color, const QColor & back_color, const float back_alpha, const int tex_width, const int tex_height, const bool error_background)
{
    //OPTIMIZATION
    //    return;

    QTextDocument text_doc;

    QTextOption text_option;
    text_option.setWrapMode(QTextOption::WordWrap);
    text_doc.setDefaultTextOption(text_option);

    QFont font = text_doc.defaultFont();
    //59.0 - size of font is device independent
//    font.setPointSize(font_size);
    font.setPixelSize(font_size);
    font.setStyleStrategy(QFont::PreferAntialias);
    text_doc.setDefaultFont(font);

    if (add_markup) {
        QString new_s = QString("<font color=") + MathUtil::GetColourAsString(text_color) + ">" + s + QString("</font>");
        text_doc.setHtml(new_s);
    }
    else {
        text_doc.setHtml(s);
    }

    text_doc.setTextWidth(tex_width);

    QImage toRender = QImage(tex_width, tex_height, QImage::Format_ARGB32_Premultiplied);
    toRender.fill((back_color.red() << 16) + (back_color.green() << 8) + back_color.blue() + (int(back_alpha * 255.0f) << 24));

    QRect dim_rect(4, 4, tex_width - 4, tex_height - 4);
    QRect text_bounding_rect;
    text_bounding_rect.setWidth(qMin(text_doc.size().width(), qreal(tex_width)));
    text_bounding_rect.setHeight(qMin(text_doc.size().height(), qreal(tex_width)));

    QPainter painter(&toRender);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setPen(Qt::white);

    //painter.setFont(font);
    //painter.drawText(dim_rect, int(Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap), text, &text_bounding_rect);
    if (error_background) {
        QImage error_img(MathUtil::GetApplicationPath() + "/assets/error.png");
        painter.drawImage(QRect(0, 0, tex_width, tex_height), error_img);
    }
    text_doc.drawContents(&painter, QRectF(0, 0, tex_width, tex_width));
    painter.end();

    if (!error_background) {
        toRender = toRender.copy(text_bounding_rect);
    }

    SetStarted(true);
    SetLoaded(true);
    SetProcessing(true);
    SetFinished(false);
    ClearData();

    aspect = float(toRender.height()) / float(toRender.width());

    // gambiarra: this should not exist in future TODO
    TextureImporterQImage* importer = (TextureImporterQImage*)importers[0];
    textureData = importer->ConvertQt(toRender);
    if (textureData.isNull())
    {
        // didnt load anything, should log as an error
        SetFinished(true);
        return;
    }

    textureData->SetSource(s); // for debugging purposes, so we know where the textures came from

    SetProcessed(true);
}

void AssetImage::Load()
{    
//    qDebug() << "AssetImage::Load()" << src_url;
    if (props->GetSrc().left(5) == "data:") {
        WebAsset::Load(QUrl(props->GetSrc()));
    }
    else {
        if (props->GetSrcURL().isEmpty()) {
//            qDebug() << "AssetImage::Load() ERROR: Tried to load with an empty url" << props->GetID() << props->GetSrc();
            SetLoaded(true);
            SetProcessed(true);
            SetFinished(true);
            this->SetStatusCode(404);
        }
        else {
            WebAsset::Load(QUrl(props->GetSrcURL()));
        }
    }
}

void AssetImage::UnloadTextures()
{        
    if (textureData)
	{
        /*auto num_left_textures = textureData->GetTotalTextures();
		for (int tex_index = 0; tex_index < num_left_textures; ++tex_index)
		{
			auto left_tex_index = textureData->GetLeftTexture(tex_index);
			auto right_tex_index = textureData->GetRightTexture(tex_index);
			
            if (left_tex_index != UINT_MAX
                && left_tex_index != AssetImage::null_image_tex
                && left_tex_index != AssetImage::null_cubemap_tex)
			{
				TextureManager::FreeTexture(left_tex_index);
			}

            if (right_tex_index != UINT_MAX
                && right_tex_index != AssetImage::null_image_tex
                && right_tex_index != AssetImage::null_cubemap_tex
                && right_tex_index != left_tex_index)
			{
				TextureManager::FreeTexture(right_tex_index);
			}
        }*/
		
		delete textureData;
        textureData.clear();
    }
}

void AssetImage::Unload()
{
    //qDebug() << "AssetImage::Unload()" << img_url_str << reply.isNull();
    UnloadTextures();
    WebAsset::Unload();
}

float AssetImage::GetAspectRatio() const
{
    return aspect;
}

bool AssetImage::GetFinished() const
{
    if (!WebAsset::GetFinished() || textureData.isNull()) {
        return false;
    }    

    return textureData->IsUploadFinished();
}

bool AssetImage::GetTransparent() const
{
    return transparent;
}

QPointer <TextureHandle> AssetImage::GetTextureHandle(const bool left_eye)
{
    if (textureData.isNull()) {
        return AssetImage::null_image_tex_handle;
    }

    if (left_eye) {
        auto left_tex_handle = textureData->GetLeftTextureHandle(tex_index);
        return (left_tex_handle != nullptr) ? left_tex_handle : AssetImage::null_image_tex_handle;
    }
    else {
        auto right_tex_handle = textureData->GetRightTextureHandle(tex_index);
        return (right_tex_handle != nullptr) ? right_tex_handle : AssetImage::null_image_tex_handle;
    }
}

void AssetImage::DrawSelectedGL(QPointer <AssetShader> shader)
{
    shader->SetConstColour(QVector4D(0.5f, 1.0f, 0.5f, 0.25f));
    shader->SetChromaKeyColour(QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
    shader->SetUseCubeTextureAll(false);
    shader->SetUseTextureAll(false);
    shader->SetUseLighting(false);
    shader->SetAmbient(QVector3D(1.0f, 1.0f, 1.0f));
    shader->SetDiffuse(QVector3D(1.0f, 1.0f, 1.0f));
    shader->SetSpecular(QVector3D(0.04f, 0.04f, 0.04f));
    shader->SetShininess(20.0f);

    shader->UpdateObjectUniforms();

    RendererInterface * renderer = RendererInterface::m_pimpl;
    AbstractRenderCommand a(PrimitiveType::TRIANGLES,
                            renderer->GetTexturedCubePrimCount(),
                            0,
                            0,
                            0,
                            renderer->GetTexturedCubeVAO(),
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

    shader->SetConstColour(QVector4D(1,1,1,1));
}

bool AssetImage::GetIsStereoImage()
{
     return (props->GetSBS3D() || props->GetOU3D());
}

// this should be pretty much equal to assetobject::DrawGL
void AssetImage::DrawImageGL(QPointer <AssetShader> shader, const bool left_eye)
{
    shader->SetUseTextureAll(false);
    shader->SetUseTexture(0, true);
    shader->SetConstColour(QVector4D(1, 1, 1, 1));

    TextureHandle* tex_id_left = GetTextureHandle(left_eye);
    TextureHandle* tex_id_right = GetTextureHandle(false);

    RendererInterface * renderer = RendererInterface::m_pimpl;
    renderer->BindTextureHandle(0, tex_id_left);

    auto textureSet = renderer->GetCurrentlyBoundTextures();
    if (tex_id_left != tex_id_right && GetIsStereoImage() == true)
    {
        textureSet.SetTextureHandle(0, tex_id_right, 2);
    }

    AbstractRenderCommand a(PrimitiveType::TRIANGLES,
            renderer->GetTexturedCubePrimCount(),
            0,
            0,
            0,
            renderer->GetTexturedCubeVAO(),
            shader->GetProgramHandle(),
            shader->GetFrameUniforms(),
            shader->GetRoomUniforms(),
            shader->GetObjectUniforms(),
            shader->GetMaterialUniforms(),
            textureSet,
            renderer->GetDefaultFaceCullMode(),
            renderer->GetDepthFunc(),
            renderer->GetDepthMask(),
            renderer->GetStencilFunc(),
            renderer->GetStencilOp(),
            renderer->GetColorMask());
    renderer->PushAbstractRenderCommand(a);
}

bool AssetImage::UpdateGL()
{    
    if (!SettingsManager::GetAssetImagesEnabled()) {
        return false;
    }

    if (GetLoaded() && !GetProcessing()) {
//        qDebug() << "AssetImage::UpdateGL() starting thread" << src_url;
        SetProcessing(true);
        if (!GetError()) {
            QtConcurrent::run(this, &AssetImage::LoadImageDataThread); //49.25 - do image loading from a non-UI thread
        }
        else {
            SetProcessed(true);
        }
    }

    if (GetLoaded() && GetProcessing() && GetProcessed() && !GetFinished()) {
        LoadTextures();
        return true;
    }

    if (textureData != nullptr)
    {
        int total = textureData->GetTotalTextures();
        if (total > 1)
        {
            QVector<int> delays = textureData->GetImageDelays();

            // If we are on the first frame, restart the clock and
            // set the next_frame_time to the duration of the first frame
            if (next_frame_time == -1)
            {
                time.restart();
                tex_index = 0;
                next_frame_time = delays[tex_index];
            }

            // This loop will fast forward through frames to catch up the the timer
            // this is useful for cases where the gif frames are shorter than a game-frame and you
            // want to see every second frame for example while still playing the gif at the correct
            // frames per second.
            while (tex_index < delays.size() && (time.elapsed() > next_frame_time))
            {
                ++tex_index;

                //loop around (either by a count, or infinitely if frame_loop == -1
                if (tex_index >= total)
                {
                    int frame_loop = textureData->GetFrameLoop();
                    if (frame_loop == -1)
                    {
                        tex_index = 0;
                    }
                    else if (frame_loop > 0)
                    {
                        tex_index = 0;
                    }
                    else
                    {
                        tex_index = total - 1;
                    }
                }

                // Add the new frames delay to the next_frame_time, this lets us
                // display frames as near to the expected time as possible, a frame
                // that was displayed longer than usual will result in the next frame or frames
                // being shown for shorter times to catch up,
                next_frame_time += delays[tex_index];
            }
        }
    }

    return false;
}

void AssetImage::DrawGL(QPointer <AssetShader> shader, const bool left_eye)
{
    if (GetFinished()) {
        MathUtil::ModelMatrix().scale(1, aspect, 1);
        shader->UpdateObjectUniforms();
        DrawImageGL(shader, left_eye);
    }
}

bool AssetImage::GetIsHDR()
{
    if (textureData.isNull())
    {
        return false;
    }
    return textureData->IsHDR();
}

void AssetImage::LoadImageDataThread()
{
    if (GetProcessed()) {        
        return;
    }

//    qDebug() << "AssetImage::LoadImageDataThread() started" << src_url;
    QString web_asset_url = GetURL().toString();
    QString extension = web_asset_url.right(web_asset_url.size() - (web_asset_url.lastIndexOf(".") +1));
    const QByteArray& buffer = GetData();

    textureData = static_cast<AssetImageData *>(LoadAssetImage(buffer, extension, props, load_gli).data());
    SetProcessed(true);

    if (textureData.isNull())
    {        
        return; //56.0 - we will load with gli instead (don't clear the webasset data)
    }
    ClearData();
    aspect = float(textureData->GetHeight()) / float(textureData->GetWidth());
//    qDebug() << "AssetImage::LoadImageDataThread() completed" << src_url;
}

void AssetImage::LoadTextures()
{    
    if (GetData().isEmpty()) {
        if (!textureData.isNull()) {
            if (textureData->IsUploadSubmitted()) {
                SetFinished(true);
            }
            else
            {
                // update the upload process in the Texture Manager
                CreateTexture(textureData, props);
            }
        }
    }
    else if (!GetFinished() && !GetError() && load_gli) {
        // temporary path for loading gli textures
        const bool tex_linear = props->GetTexLinear();
        const bool tex_mipmap = props->GetTexMipmap();
        const bool tex_clamp = props->GetTexClamp();
        TextureHandle::ALPHA_TYPE tex_alpha = TextureHandle::ALPHA_TYPE::UNDEFINED;
        QString const tex_alpha_string = props->GetTexAlpha();

        TextureHandle::COLOR_SPACE tex_colorspace = TextureHandle::COLOR_SPACE::SRGB;
        QString const tex_colorspace_string = props->GetTexColorspace();

        if (tex_colorspace_string.contains("sRGB")) {
            tex_colorspace = TextureHandle::COLOR_SPACE::SRGB;
        }
        else {
            tex_colorspace = TextureHandle::COLOR_SPACE::LINEAR;
        }

        QPointer<TextureHandle> tex_handle = RendererInterface::m_pimpl->CreateTextureFromGLIData(GetData(), tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
        SetFinished(true);

        if (tex_handle == nullptr) {
            return;
        }

        textureData = QPointer<AssetImageData>(new AssetImageData());
        textureData->SetWidth(RendererInterface::m_pimpl->GetTextureWidth(tex_handle));
        textureData->SetHeight(RendererInterface::m_pimpl->GetTextureHeight(tex_handle));
        textureData->SetTotalTextures(1);
        textureData->SetUploadedTexture(tex_handle, tex_handle);
    }
}

void AssetImage::SetAnisotropicMax(const GLfloat f)
{
    anisotropic_max = f;
}

float AssetImage::GetAnisotropicMax()
{
    return anisotropic_max;
}

QPointer<AssetImageData> AssetImage::GetTextureData()
{
    return textureData;
}

void AssetImage::CreateTexture(QPointer<AssetImageData> data, QPointer <DOMNode> props)
{
    if (data == nullptr || data->IsUploadSubmitted()) {
        return;
    }

    const bool tex_linear = props->GetTexLinear();
    const bool tex_mipmap = props->GetTexMipmap();
    const bool tex_clamp = props->GetTexClamp();

    TextureHandle::ALPHA_TYPE tex_alpha = TextureHandle::ALPHA_TYPE::UNDEFINED;

    // If we have multiple frames we are a GIF which can only have CUTOUT or NONE alpha,
    // So set to CUTOUT to avoid having to parse every frame of a GIF
    auto numTex = data->GetTotalTextures();
    if (numTex > 1) {
        tex_alpha = TextureHandle::ALPHA_TYPE::CUTOUT;
    }
    else {
        QString const tex_alpha_string = props->GetTexAlpha();

        if (tex_alpha_string.contains("none")) {
            tex_alpha = TextureHandle::ALPHA_TYPE::NONE;
        }
        else if (tex_alpha_string.contains("cutout")) {
            tex_alpha = TextureHandle::ALPHA_TYPE::CUTOUT;
        }
        else if (tex_alpha_string.contains("blended")) {
            tex_alpha = TextureHandle::ALPHA_TYPE::BLENDED;
        }
        else {
            tex_alpha = TextureHandle::ALPHA_TYPE::UNDEFINED;
        }
    }

    TextureHandle::COLOR_SPACE tex_colorspace = TextureHandle::COLOR_SPACE::SRGB;
    QString const tex_colorspace_string = props->GetTexColorspace();

    if (tex_colorspace_string.contains("sRGB")) {
        tex_colorspace = TextureHandle::COLOR_SPACE::SRGB;
    }
    else {
        tex_colorspace = TextureHandle::COLOR_SPACE::LINEAR;
    }

    int current = data->GetUploadedTextures();

    //uint ltex = 0;
    //uint rtex = 0;
    QPointer<TextureHandle> ltex_handle(nullptr);
    QPointer<TextureHandle> rtex_handle(nullptr);

    AssetImageDataQ* dataq = dynamic_cast<AssetImageDataQ*>(data.data());
    if (dataq != nullptr)
    {
        // TODO: abstract data comes from QImage, we'll delete this part soon enough
        // (this is already deprecated and unitilized, but is here for in case something breaks)
        QVector<QImage> left = dataq->GetLeftImages();
        QVector<QImage> right = dataq->GetRightImages();

        if (left.size() > current && !left[current].isNull())
        {
            //ltex = CreateTexture(left[current], tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
            ltex_handle = RendererInterface::m_pimpl->CreateTextureQImage(left[current], tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
        }

        if (right.size() > current && !right[current].isNull())
        {
            //rtex = CreateTexture(right[current], tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
            ltex_handle = RendererInterface::m_pimpl->CreateTextureQImage(right[current], tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
        }
    }
    else
    {

        ltex_handle = RendererInterface::m_pimpl->CreateTextureFromAssetImageData(data, true, tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);

        rtex_handle = RendererInterface::m_pimpl->CreateTextureFromAssetImageData(data, false, tex_mipmap, tex_linear, tex_clamp, tex_alpha, tex_colorspace);
    }

    //auto ltex_handle = TextureManager::GetTextureHandle(ltex);
    //auto rtex_handle = TextureManager::GetTextureHandle(rtex);
    if (ltex_handle)
    {
        data->SetUploadedTexture(ltex_handle, (rtex_handle == nullptr) ? ltex_handle : rtex_handle);
    }
    else
    {
//        qDebug() << "TextureManager::CreateTexture() - ltex_handle is NULL";
    }

    // If we have uploaded all the textures, clear out the CPU copy of the pixel data
    if (data->GetUploadedTextures() == data->GetTotalTextures())
    {
        data->ClearPixelData();
    }
}

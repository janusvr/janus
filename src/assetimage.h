#ifndef ASSETIMAGE_H
#define ASSETIMAGE_H

#include <QtNetwork>
#include <QtConcurrent/QtConcurrent>
#include <QPointer>

#include "asset.h"
#include "assetshader.h"
#include "webasset.h"
#include "mathutil.h"

#include "assetimagedata.h"
#include "assetimagedataq.h"

#include "contentimporter.h"
#include "textureimporterqimage.h"
#include "textureimportercmft.h"
#include "textureimportergli.h"

enum class CUBEMAP_TYPE : unsigned long
{
    DEFAULT,
    RADIANCE,
    IRRADIANCE
};

class AssetImage : public Asset
{
    Q_OBJECT

public:
    AssetImage();
    virtual ~AssetImage();   

    static void initializeGL();

    void Load(); //load image through a URL
    void Unload();   

    void CreateFromText(const QString & s, const float font_size, const bool add_markup, const QColor &text_color, const QColor & back_color, const float back_alpha, const int tex_width, const int tex_height, const bool error_background);
    void CreateFromData(const QByteArray & b);

    bool UpdateGL(); //call to continue loading process, even if not going to draw
    void DrawGL(QPointer <AssetShader> shader, const bool left_eye);
    void DrawSelectedGL(QPointer <AssetShader> shader);

	bool GetIsHDR();

    float GetAspectRatio() const;

    QPointer <TextureHandle> GetTextureHandle(const bool left_eye);

    bool GetFinished() const;
    bool GetTransparent() const; 

    QPointer<AssetImageData> GetTextureData();

    static void SetUseAssetImages(const bool b);
    static bool GetUseAssetImages();

	static void SetAnisotropicMax(const GLfloat f);
	static float GetAnisotropicMax();

    static QPointer<TextureHandle> null_cubemap_tex_handle;
    static QPointer<TextureHandle> null_image_tex_handle;

    static QPointer<BaseAssetData> LoadAssetImage(const QByteArray& buffer, QString extension, QPointer <DOMNode> props, bool& is_gli);

    bool GetIsStereoImage();

private:

	static void InitializeImporters();
	static QVector<ContentImporter*> importers;

    void LoadImageDataThread();
    void DrawImageGL(QPointer <AssetShader> shader, const bool left_eye);    
    void LoadTextures();
    void UnloadTextures();
    void CreateTexture(QPointer<AssetImageData> data, QPointer <DOMNode> props);

    enum class IMAGE_FORMAT : unsigned long
    {
        DDS,
        KTX,
        KMG,
        UNINITIALIZED
    };

    QPointer<AssetImageData> textureData;
	bool load_gli; // temporary

    int tex_index;    

    float aspect;

    bool transparent;
    int max_img_resolution;
    int next_frame_time;

    QTime time;

    static int anisotropic_max;    
};

#endif // ASSETIMAGE_H

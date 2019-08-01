#ifndef TEXTUREIMPORTERQIMAGE_H
#define TEXTUREIMPORTERQIMAGE_H

#include <QtNetwork>
#include <QPointer>

#include "contentimporter.h"
#include "assetimagedata.h"

#include "asset.h"
#include "assetshader.h"
#include "webasset.h"
#include "mathutil.h"
#include "rendererinterface.h"

#include "assetimagedata.h"
#include "assetimagedataq.h"

class TextureImporterQImage : public ContentImporter
{
public:
	TextureImporterQImage();
	~TextureImporterQImage();

    bool CanImport(const QByteArray& buffer, QString extension, QPointer <DOMNode> props);
    QPointer<BaseAssetData> Import(const QByteArray& buffer, QPointer <DOMNode> props);
    QPointer<AssetImageData> ConvertQt(QImage image);

private:
    QByteArray GetQImageData(QImage img, uchar& pixelSize);
    void LoadImageDataThread_Helper(QImageReader& img_reader, QImage& left_img, QImage& right_img, QPointer <DOMNode> props);

};
#endif


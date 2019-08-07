#ifndef TEXTUREIMPORTERGLI_H
#define TEXTUREIMPORTERGLI_H

#include "asset.h"
#include "assetshader.h"
#include "webasset.h"

#include "contentimporter.h"

#include "abstractrenderer.h"
#include "assetimagedata.h"
#define gli glm
#include "gli/gli.hpp"

class TextureImporterGLI : public ContentImporter
{
public:
    TextureImporterGLI();
    ~TextureImporterGLI();

    bool CanImport(const QByteArray& buffer, QString extension, QVariantMap vars);
    QPointer<QObject> Import(const QByteArray& buffer, QVariantMap vars);
};

#endif // GLITEXTUREIMPORTER_H

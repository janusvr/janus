#include "textureimportergli.h"

TextureImporterGLI::TextureImporterGLI()
{

}

bool TextureImporterGLI::CanImport(const QByteArray& , QString extension, QVariantMap )
{
	return QString::compare(extension, "dds", Qt::CaseInsensitive) == 0 ||
		   QString::compare(extension, "ktx", Qt::CaseInsensitive) == 0;
}

QPointer<BaseAssetData> TextureImporterGLI::Import(const QByteArray & buffer, QVariantMap vars)
{
    const char * da = (char*)buffer.constData();
    const int size = buffer.length();

	gli::texture Texture = gli::load(da, size);
	gli::gl GL(gli::gl::PROFILE_GL33);
	glm::tvec3<GLsizei> const Extent(Texture.extent());

    if ((Texture.target() != gli::TARGET_2D && Texture.target() != gli::TARGET_CUBE) || Texture.empty()) {
		qDebug("ERROR: GLI texture loading failed due to invalid texture target or no texture data");
        return QPointer<BaseAssetData>();
	}

    QPointer <AssetImageData> data = new AssetImageData();
	data->SetWidth(Extent.x);
	data->SetHeight(Extent.y);

    const int totalFrames = int(Texture.layers() * Texture.faces() * Texture.levels());
	data->PreallocateBuffers(totalFrames);

    if (vars.contains("src")) {
        // right now this is used only for debugging/knowing
        // where the texture came from
        data->SetSource(vars["src"].toString());
    }

    return static_cast<BaseAssetData *>(data.data());
}

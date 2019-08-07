#include "textureimportercmft.h"

TextureImporterCMFT::TextureImporterCMFT(void)
{

}

TextureImporterCMFT::~TextureImporterCMFT()
{

}

bool TextureImporterCMFT::CanImport(const QByteArray& , QString extension, QPointer <DOMNode> )
{
    return QString::compare(extension, "hdr", Qt::CaseInsensitive) == 0;
}

QPointer<QObject> TextureImporterCMFT::Import(const QByteArray& buffer, QPointer <DOMNode> )
{
    char* da = (char*)buffer.constData();
    const int size = buffer.length();

    // Load the .hdr file with cmft
    cmft::Image image;
    cmft::imageLoad(image, da, size, cmft::TextureFormat::RGBA16F);
    // Conver it into a cubemap
    cmft::imageCubemapFromLatLong(image);
    // Save it as a .dds file so that the gli loader can load it
    std::vector<char> mem_dds;
    cmft::imageSaveToMemDDS(image, &mem_dds, cmft::ImageFileType::Enum::DDS);
    // Copy the .dds file into the buffer so that gli has the data it needs
    const size_t mem_size = mem_dds.size();
    ((QByteArray&)buffer).resize(mem_size);
    memcpy((char*)(((QByteArray&)(buffer)).constData()), mem_dds.data(), mem_dds.size());

    return static_cast<QObject *>(nullptr);
}

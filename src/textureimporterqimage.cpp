#include "textureimporterqimage.h"

TextureImporterQImage::TextureImporterQImage()
{
}

TextureImporterQImage::~TextureImporterQImage()
{
}

QByteArray TextureImporterQImage::GetQImageData(QImage img, uchar& pixelSize)
{
    if (img.isNull()) {
        return QByteArray();
	}

    if (   img.format() == QImage::Format_RGBX8888
        || img.format() == QImage::Format_RGBA8888
        || img.format() == QImage::Format_RGBA8888_Premultiplied
        || img.format() == QImage::Format_ARGB32
        || img.format() == QImage::Format_ARGB32_Premultiplied
        || img.format() == QImage::Format_RGB32)
	{
		pixelSize = 4;
	}
	else
	{
		pixelSize = 3;
	}

    const int count = img.width() * img.height() * pixelSize;

    QByteArray nbitsdata;
    nbitsdata.resize(count);
    uchar* nbits = (uchar *)(nbitsdata.data());

	const uchar* bits = img.constBits();
	memcpy(nbits, bits, count);

    return nbitsdata;
}

bool TextureImporterQImage::CanImport(const QByteArray& , QString extension, QPointer <DOMNode> )
{
	// we could potentially read headers here, but a simple extension comparison
	// is good enough

	return (QString::compare(extension, "bmp", Qt::CaseInsensitive) == 0 ||
		QString::compare(extension, "gif", Qt::CaseInsensitive) == 0 ||
		QString::compare(extension, "jpg", Qt::CaseInsensitive) == 0 ||
		QString::compare(extension, "jpeg", Qt::CaseInsensitive) == 0 ||
		QString::compare(extension, "png", Qt::CaseInsensitive) == 0 ||
        QString::compare(extension, "tga", Qt::CaseInsensitive) == 0 ||
		QString::compare(extension, "pbm", Qt::CaseInsensitive) == 0 ||
		QString::compare(extension, "pgm", Qt::CaseInsensitive) == 0 ||
		QString::compare(extension, "ppm", Qt::CaseInsensitive) == 0 ||
		QString::compare(extension, "xbm", Qt::CaseInsensitive) == 0 ||
		QString::compare(extension, "xpm", Qt::CaseInsensitive) == 0);
}

QPointer<AssetImageData> TextureImporterQImage::ConvertQt(QImage image)
{
    QPointer<AssetImageData> data = QPointer<AssetImageData>(new AssetImageData());

	data->PreallocateBuffers(1);

	int count;
    if (   image.format() == QImage::Format_RGBX8888
        || image.format() == QImage::Format_RGBA8888
        || image.format() == QImage::Format_RGBA8888_Premultiplied
        || image.format() == QImage::Format_ARGB32
        || image.format() == QImage::Format_ARGB32_Premultiplied
        || image.format() == QImage::Format_RGB32)
	{
		count = image.width() * image.height() * 4;
		data->SetPixelSize(4);
	}
	else
	{
		count = image.width() * image.height() * 3;
		data->SetPixelSize(3);
	}

    data->format = image.format();    
	uchar* bits = image.bits();

    QByteArray nbitsdata;
    nbitsdata.resize(count);
    uchar* nbits = (uchar *)(nbitsdata.data());

	memcpy(nbits, bits, count);

    data->SetFrameData(0, nbitsdata, true);

	data->SetWidth(image.width());
	data->SetHeight(image.height());
	data->SetTotalTextures(1);

	return data;
}

void TextureImporterQImage::LoadImageDataThread_Helper(QImageReader& img_reader, QImage& left_img, QImage& right_img, QPointer <DOMNode> props)
{
	QImage each_img = img_reader.read();

	if (each_img.isNull()) {
		left_img = each_img;
		right_img = each_img;
		return;
	}

	//56.0 - makes portal thumbs and other images look correct
    QImage::Format format = each_img.format();
    if (each_img.hasAlphaChannel())
    {
        if (props->GetTexPreMultiply()) {
            if (   format != QImage::Format_RGBA8888_Premultiplied
                && format != QImage::Format_ARGB32_Premultiplied) {
                if (format == QImage::Format_ARGB32) {
                    each_img = each_img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
                }
                else
                {
                    each_img = each_img.convertToFormat(QImage::Format_RGBA8888_Premultiplied);
                }
            }
        }
        else {
            if (   format != QImage::Format_RGBA8888
                && format != QImage::Format_ARGB32
                && format != QImage::Format_RGBA8888_Premultiplied
                && format != QImage::Format_ARGB32_Premultiplied) {
                each_img = each_img.convertToFormat(QImage::Format_RGBA8888);
            }
        }
    }
    else {
        if (format != QImage::Format_RGB32
            && format != QImage::Format_RGBX8888)
        {
            each_img = each_img.convertToFormat(QImage::Format_RGBX8888);
        }
    }

	const float w = each_img.width();
	const float h = each_img.height();

    if (props->GetSBS3D()) {
        if (props->GetReverse3D()) {
			left_img = each_img.copy(w / 2, 0, w / 2, h);
			right_img = each_img.copy(0, 0, w / 2, h);
		}
        else {
			left_img = each_img.copy(0, 0, w / 2, h);
			right_img = each_img.copy(w / 2, 0, w / 2, h);
		}
	}
    else if (props->GetOU3D()) {
        if (props->GetReverse3D()) {
			left_img = each_img.copy(0, h / 2, w, h / 2);
			right_img = each_img.copy(0, 0, w, h / 2);
		}
        else {
			left_img = each_img.copy(0, 0, w, h / 2);
			right_img = each_img.copy(0, h / 2, w, h / 2);
		}
	}
    else {
		left_img = each_img;
	}
}

QPointer<BaseAssetData> TextureImporterQImage::Import(const QByteArray& bytes, QPointer <DOMNode> props)
{
    QPointer<BaseAssetData> bdata = QPointer<BaseAssetData>(new AssetImageData());
	AssetImageData* data = (AssetImageData*)bdata.data();

    if (!props->GetSrc().isEmpty()) {
		// right now this is used only for debugging/knowing
		// where the texture came from
        data->SetSource(props->GetSrc());
	}

    //59.6 - Linux compile fix
    QBuffer buffer;
    buffer.setData(bytes);

	QImageReader img_reader;
	img_reader.setDevice(&buffer);

	int nImgs = 1;
	int width = 0;
	int height = 0;

	// extract the data from the Qt
	uchar pixelSize = 0;

	if (img_reader.supportsAnimation())
	{
		nImgs = img_reader.imageCount();
		int loopCount = img_reader.loopCount();
		data->SetFrameLoop(loopCount);

		if (nImgs > 0)
		{
			data->PreallocateBuffers(nImgs);

			for (int i = 0; i < nImgs; ++i)
			{
				img_reader.jumpToImage(i);

				QImage leftQ;
				QImage rightQ;
                LoadImageDataThread_Helper(img_reader, leftQ, rightQ, props);

				width = leftQ.width();
				height = leftQ.height();
                data->format = leftQ.format();
				data->SetFrameData(i, GetQImageData(leftQ, pixelSize), true);
				data->SetFrameData(i, GetQImageData(rightQ, pixelSize), false);
                data->SetImageDelay(i, img_reader.nextImageDelay()); //59.0 - note!  set imagedelay after calling LoadImageDataThread_Helper
			}
		}
	}
	else
	{
		data->PreallocateBuffers(nImgs);

		QImage leftQ;
		QImage rightQ;
        LoadImageDataThread_Helper(img_reader, leftQ, rightQ, props);

		width = leftQ.width();
		height = leftQ.height();

        data->format = leftQ.format();
		data->SetFrameData(0, GetQImageData(leftQ, pixelSize), true);
		data->SetFrameData(0, GetQImageData(rightQ, pixelSize), false);
	}

	data->SetWidth(width);
	data->SetHeight(height);
	data->SetTotalTextures(nImgs);
	data->SetPixelSize(pixelSize);

	return bdata;
}


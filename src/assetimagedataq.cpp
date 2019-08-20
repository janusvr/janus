#include "assetimagedataq.h"

AssetImageDataQ::AssetImageDataQ() :
	left_imgs(1),
	right_imgs(1)
{
}

void AssetImageDataQ::ClearPixelData()
{
    AssetImageData::ClearPixelData();
    left_imgs.resize(0);
    right_imgs.resize(0);
}

QVector<QImage> AssetImageDataQ::GetLeftImages()
{
    return left_imgs;
}

void AssetImageDataQ::SetLeftImages(QVector<QImage> value)
{
    left_imgs = value;
}

QVector<QImage> AssetImageDataQ::GetRightImages()
{
    return right_imgs;
}

void AssetImageDataQ::SetRightImages(QVector<QImage> value)
{
    right_imgs = value;
}

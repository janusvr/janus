#ifndef ASSETIMAGEDATAQ_H
#define ASSETIMAGEDATAQ_H

#include <QtCore>
#include <QImage>

#include "assetimagedata.h"

class AssetImageDataQ : public AssetImageData
{
private:
	QVector<QImage> left_imgs;
	QVector<QImage> right_imgs;

public:
	AssetImageDataQ();

    void ClearPixelData();

    QVector<QImage> GetLeftImages();
    void SetLeftImages(QVector<QImage> value);

    QVector<QImage> GetRightImages();
    void SetRightImages(QVector<QImage> value);
};

#endif // ASSETIMAGEDATAQ_H

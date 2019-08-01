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

	void ClearPixelData()
	{
		AssetImageData::ClearPixelData();

		left_imgs.resize(0);
		right_imgs.resize(0);
	}

	QVector<QImage> GetLeftImages() { return left_imgs; }
	void SetLeftImages(QVector<QImage> value) { left_imgs = value; }

	QVector<QImage> GetRightImages() { return right_imgs; }
	void SetRightImages(QVector<QImage> value) { right_imgs = value; }
};

#endif // ASSETIMAGEDATAQ_H
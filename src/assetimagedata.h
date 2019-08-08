#ifndef ASSETIMAGEDATA_H
#define ASSETIMAGEDATA_H

#include <QtCore>
#include <QImage>

#include "mathutil.h"

class AssetImageData : public QObject
{
private:
	int width;
	int height;
	uchar pixelSize;

	int frameLoop;
	QVector<int> delays;

    QVector<QByteArray> left_frames;
    QVector<QByteArray> right_frames;

	QVector<uint> left_textures;
    QVector<QPointer<TextureHandle>> left_texture_handles;
	QVector<uint> right_textures;
    QVector<QPointer<TextureHandle>> right_texture_handles;

	int uploadedTextures;
	int totalTextures;

	QString source;

	bool is_hdr;

public:
	AssetImageData() :
		width(0),
		height(0),
		pixelSize(0),
		frameLoop(-1),
		delays(0),
		left_frames(0),
		right_frames(0),
		left_textures(0),
		right_textures(0),
		uploadedTextures(0),
		totalTextures(0),
		is_hdr(false)
	{
	}

	AssetImageData(const AssetImageData& source);
	//AssetImageData& operator=(AssetImageData other);

	~AssetImageData();

    bool IsUploadFinished(); // Talking about GPU data upload

    bool IsHDR() { return is_hdr; }
    void SetHDR(bool value)	{ is_hdr = value; }

	bool HasFrameData(int index, bool is_left);

	// Deletes only the pixel data, while keeping everything else the same
    virtual void ClearPixelData();
	virtual void ClearPixelData(int index);

	uint GetLeftTexture(int index);
	uint GetRightTexture(int index);

    QByteArray GetLeftFrameData(int index);
    QByteArray GetRightFrameData(int index);

	int GetTotalTextures() { return totalTextures; }
	void SetTotalTextures(int value);

	bool IsAnimated() { return delays.size() > 1; }

	int GetUploadedTextures() { return uploadedTextures; }
    void SetUploadedTexture(uint left_id, uint right_id);

	bool IsUploadSubmitted();

	QSize GetSize() { return QSize(width, height); }

	int GetWidth() { return width; }
	void SetWidth(int value) { width = value; }
	int GetHeight() { return height; }
	void SetHeight(int value) { height = value; }

	uchar GetPixelSize() { return pixelSize; }
	void SetPixelSize(uchar value) { pixelSize = value; }

	QString GetSource() { return source; }
	void SetSource(QString value) { source = value; }

	QVector<int> GetImageDelays();
	void SetImageDelay(int index, int delay);

	int GetFrameLoop() { return frameLoop; }
	void SetFrameLoop(int value) { frameLoop = value; }

    void PreallocateBuffers(int nFrames);

    void SetFrameData(int index, QByteArray data, bool is_left);
    void SetLeftFrameData(int index, QByteArray data);
    void SetRightFrameData(int index, QByteArray data);

    void SetUploadedTexture(QPointer<TextureHandle> left_id, QPointer<TextureHandle> right_id);
    QPointer<TextureHandle> GetLeftTextureHandle(int index);
    QPointer<TextureHandle> GetRightTextureHandle(int index);

    QImage::Format format;
};

#endif

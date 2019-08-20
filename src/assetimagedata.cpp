#include "assetimagedata.h"

AssetImageData::AssetImageData() :
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

AssetImageData::AssetImageData(const AssetImageData& ):
	is_hdr(false)
{
	// make everything null, as we don't want to copy all this data
	width = 0;
	height = 0;
	pixelSize = 0;
	frameLoop = -1;
	uploadedTextures = 0;
	totalTextures = 0;
	delays = QVector<int>(0);    
    left_frames = QVector<QByteArray>(0);
    right_frames = QVector<QByteArray>(0);
}

AssetImageData::~AssetImageData()
{
    ClearPixelData();
}

bool AssetImageData::IsUploadSubmitted()
{
    return uploadedTextures >= totalTextures;
}

bool AssetImageData::IsUploadFinished()
{
    if (!IsUploadSubmitted())
    {
        return false;
    }

    int const totalTextures = GetTotalTextures();

    // all textures were submitted, but were they all uploaded?
    for (int i = 0; i < totalTextures; i++)
    {
        if (left_texture_handles[i] == nullptr)
        {
            return false;
        }

        if (right_texture_handles[i] == nullptr)
        {
            return false;
        }
    }

    return true;
}

void AssetImageData::ClearPixelData(int index)
{
	if (index == -1)
	{
		ClearPixelData();
		return;
	}

	if (left_frames.size() > index)
	{
        left_frames[index].clear();
	}

	if (right_frames.size() > index)
	{
        right_frames[index].clear();
	}
}
void AssetImageData::ClearPixelData()
{
    int size = left_frames.size();
    for (int i = 0; i < size; i++)
    {
        left_frames[i].clear();
    }
    left_frames.clear();

    size = right_frames.size();
    for (int i = 0; i < size; i++)
    {
        right_frames[i].clear();
    }
    right_frames.clear();
}

void AssetImageData::SetTotalTextures(int value)
{
	totalTextures = value;
    left_texture_handles.resize(value);
    right_texture_handles.resize(value);
}

void AssetImageData::SetUploadedTexture(QPointer<TextureHandle> left_id, QPointer<TextureHandle> right_id)
{
    // Clear out Texture data now that we have the GPU texture create
    left_texture_handles[uploadedTextures] = left_id;
    right_texture_handles[uploadedTextures] = right_id;
    uploadedTextures++;
}

uint AssetImageData::GetLeftTexture(int index)
{
	if (index >= 0 && index < left_textures.size())
	{
		return left_textures[index];
	}
    return 0;
}

QPointer<TextureHandle> AssetImageData::GetLeftTextureHandle(int index)
{
    if (index >= 0 && index < left_texture_handles.size())
    {
        return left_texture_handles[index];
    }
    return 0;
}

uint AssetImageData::GetRightTexture(int index)
{
	if (index >= 0 && index < right_textures.size())
	{
		return right_textures[index];
	}
    return 0;
}

QPointer<TextureHandle> AssetImageData::GetRightTextureHandle(int index)
{
    if (index >= 0 && index < right_texture_handles.size())
    {
        return right_texture_handles[index];
    }
    return 0;
}

void AssetImageData::PreallocateBuffers(int nFrames)
{
    left_frames.resize(nFrames);
	right_frames.resize(nFrames);

	left_textures.resize(nFrames);
	right_textures.resize(nFrames);

    left_texture_handles.resize(nFrames);
    right_texture_handles.resize(nFrames);

	if (nFrames > 1)
	{
		delays.resize(nFrames);
	}
}

void AssetImageData::SetLeftFrameData(int index, QByteArray data)
{
	if (index >= 0)
	{
		left_frames.resize(index + 1);
		left_frames[index] = data;
	}
}

void AssetImageData::SetRightFrameData(int index, QByteArray data)
{
	if (index >= 0)
	{
		right_frames.resize(index + 1);
		right_frames[index] = data;
	}
}

void AssetImageData::SetFrameData(int index, QByteArray data, bool is_left)
{
    is_left ? SetLeftFrameData(index, data) : SetRightFrameData(index, data);
}

QByteArray AssetImageData::GetLeftFrameData(int index)
{
	if (left_frames.size() <= index)
	{
        return QByteArray();
	}
	return left_frames[index];
}

QByteArray AssetImageData::GetRightFrameData(int index)
{
	if (right_frames.size() <= index)
	{
        return QByteArray();
	}
	return right_frames[index];
}

bool AssetImageData::HasFrameData(int index, bool is_left)
{
	return is_left ?
        (left_frames.size() > index && !left_frames[index].isEmpty() ) :
        (right_frames.size() > index && !right_frames[index].isEmpty());
}

QVector<int> AssetImageData::GetImageDelays()
{
	return delays;
}

void AssetImageData::SetImageDelay(int index, int delay)
{
    if (index >= 0 && index < delays.size())
	{
		delays[index] = delay;
	}
}

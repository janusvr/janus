#ifndef AUDIOUTIL_H
#define AUDIOUTIL_H

#include <QtCore>

extern "C"{
    #include <opus.h>
}

#include <AL/al.h>
#include <AL/alc.h>

#include <QtDebug>

struct WavHeader
{
    WavHeader();
    WavHeader(unsigned long sample_rate, unsigned short bits_per_sample, unsigned short channels, unsigned long data_size);
    WavHeader(QByteArray ba);

    QByteArray getByteArray();
    ALenum getALFormat();

    unsigned char  riff_id[4];			//'RIFF'
    quint32        riff_size;			// DataSize + 44
    unsigned char  riff_format[4];		// 'WAVE'

    unsigned char  fmt_id[4];			// 'fmt '
    quint32        fmt_size;			// 16
    quint16        fmt_format;			// 1 (=MS PCM)
    quint16        fmt_channels;		// 1 = mono, 2 = stereo
    quint32        fmt_sample_rate;	// (e.g 44100)
    quint32        fmt_byte_rate;		// SamplingRate * BlockAlign
    quint16        fmt_block_align;		// Channels * BitsPerSample / 8
    quint16        fmt_bits_per_sample;	// 8 or 16

    unsigned char  data_id[4];			// 'data'
    quint32        data_size;		// Size of the following data
};

class AudioUtil
{
public:
    AudioUtil();
    ~AudioUtil();

    static QByteArray encode(QByteArray decoded); // Encode with Opus
    static QByteArray decode(QByteArray encoded); // Decode with Opus

    static bool isWav(QByteArray ba); // Check if wav
    static QByteArray stereoToMono(QByteArray mono); // Convert a stereo WAV byte array to a mono one

    static const int _FRAME_SIZE;
    static const int _SAMPLE_RATE;
    static const int _CHANNELS;
    static const int _APPLICATION;

    static OpusEncoder * encoder;
    static OpusDecoder * decoder;
};

#endif // AUDIOUTIL_H


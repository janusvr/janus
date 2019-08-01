#include "audioutil.h"

const int AudioUtil::_FRAME_SIZE = 2880; //2880; //2646; // 2646 is 60ms of 44.1khz playback... 2880;
const int AudioUtil::_SAMPLE_RATE = 48000; //44100;
const int AudioUtil::_CHANNELS = 1;
const int AudioUtil::_APPLICATION = OPUS_APPLICATION_VOIP;

OpusEncoder * AudioUtil::encoder = NULL;
OpusDecoder * AudioUtil::decoder = NULL;

AudioUtil::AudioUtil()
{
}

AudioUtil::~AudioUtil()
{
}

QByteArray AudioUtil::encode(QByteArray decoded)
{
    //Create Opus encoder state
    int error = 0;
    if (encoder == NULL) {
        encoder = opus_encoder_create(AudioUtil::_SAMPLE_RATE, AudioUtil::_CHANNELS, AudioUtil::_APPLICATION, &error);        
    }
    if (error<0)
    {
        // Failed to create encoder state
        qDebug("ERROR: Failed to create encoder state");
        return QByteArray();
    }

    //these crash if encoder is NULL
    opus_encoder_ctl(encoder, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_MEDIUMBAND));
//        opus_encoder_ctl(encoder, OPUS_SET_BITRATE(48000)); //4KB/sec

    //Encode a frame
    const unsigned int s = AudioUtil::_FRAME_SIZE*AudioUtil::_CHANNELS;
    opus_int16 in[s];
    memcpy(in, decoded.data(), s*sizeof(opus_int16)); //conversion from samples to 16-bit type

    //4000 - recommended maximum number of bytes per packet
    int max_size = 4000; //4000
    QByteArray encoded(max_size, '\0');
    const int len = opus_encode(encoder, in, AudioUtil::_FRAME_SIZE, (unsigned char *)encoded.data(), max_size);
//    qDebug() << "encode len" << decoded.length() << len;
    if (len < 0) {
        //qDebug("ERROR: Failed to encode: %s\n", opus_strerror(len));
        return QByteArray();
    }

    encoded.resize(len);
//    encoded.squeeze();

    //Return encoded data
    return encoded;
}

QByteArray AudioUtil::decode(QByteArray encoded)
{
    //Create Opus decoder state
    int error = 0;
    if (decoder == NULL) {
        decoder = opus_decoder_create(AudioUtil::_SAMPLE_RATE, AudioUtil::_CHANNELS, &error);
    }
    if (error<0)
    {
        // Failed to create decoder state
        qDebug("ERROR: Failed to create decoder state");
        return QByteArray();
    }

    //Decode a frame
    const int decode_size = AudioUtil::_FRAME_SIZE * AudioUtil::_CHANNELS * sizeof(opus_int16);
    QByteArray decoded(decode_size, '\0');
    if (decoded.size() != decode_size)
    {
        qDebug("ERROR: Failed to allocate memory for decoded data");
        return QByteArray();
    }

    const int len = opus_decode(decoder, (unsigned char *)encoded.data(), encoded.size(), (opus_int16*)decoded.data(), AudioUtil::_FRAME_SIZE, 0);
    if (len<0)
    {
        //qDebug("ERROR: Failed to decode: %s\n", opus_strerror(len));
        return QByteArray();
    }

    decoded.resize(len * sizeof(opus_int16));
//    decoded.squeeze();

    //Return decoded data
    return decoded;
}

bool AudioUtil::isWav(QByteArray ba)
{
    if (ba.size() >= 44)
    {
        WavHeader wh(ba.mid(0,44));
        char riff_id[5];
        char riff_format[5];
        char fmt_id[5];
        char data_id[5];
        memcpy(riff_id, wh.riff_id, 4);
        memcpy(riff_format, wh.riff_format, 4);
        memcpy(fmt_id, wh.fmt_id, 4);
        memcpy(data_id, wh.data_id, 4);
        riff_id[4] = '\0';
        riff_format[4] = '\0';
        fmt_id[4] = '\0';
        data_id[4] = '\0';

        if (strcmp(riff_id, "RIFF") == 0 && strcmp(riff_format, "WAVE") == 0 && strcmp(fmt_id, "fmt ") == 0 && strcmp(data_id, "data") == 0) {
            return true;
        }
    }
    return false;
}

QByteArray AudioUtil::stereoToMono(QByteArray stereo)
{
    QByteArray mono;
    if (isWav(stereo)){
        WavHeader wh(stereo.mid(0,44));
        if (wh.fmt_channels == 2){
            //Change header
            wh.riff_size = 44 + wh.data_size/2;
            wh.fmt_channels = 1;
            wh.fmt_block_align = wh.fmt_channels*wh.fmt_bits_per_sample/8;
            wh.fmt_byte_rate = wh.fmt_channels*wh.fmt_bits_per_sample*wh.fmt_sample_rate/8;
            wh.data_size = wh.data_size/2;
            mono.push_back(wh.getByteArray());
            stereo.remove(0,44);

            //Iterate through each pair of samples and average out
            //Weight one channel more to prevent out of phase zeroing out
            if (wh.fmt_bits_per_sample == 16){               
                for (int i = 0; i < (int) wh.data_size; i+=2){
                    short val = 0;
                    memcpy(&val, &stereo.data()[2*i], sizeof(short));
                    mono.push_back(char(val));
                    mono.push_back(char(val>>8));
                }
            }
            else if (wh.fmt_bits_per_sample == 8){               
                for (int i = 0; i < (int) wh.data_size; i++){
                    unsigned char val = 0;
                    memcpy(&val, &stereo.data()[2*i], sizeof(unsigned char));
                    mono.push_back((unsigned char)(val));
                }

            }
        }
        else{
            mono = stereo;
        }
    }
    else{
        mono = stereo;
    }
    return mono;
}

WavHeader::WavHeader()
{
}

WavHeader::WavHeader(unsigned long sample_rate, unsigned short bits_per_sample, unsigned short channels, unsigned long data_size)
{
    memset(this, 0, 44);

    // Set Riff-Chunk
    memcpy(this->riff_id, "RIFF", 4);
    this->riff_size = data_size + 44;
    memcpy(this->riff_format, "WAVE", 4);

    // Set Fmt-Chunk
    memcpy(this->fmt_id, "fmt ", 4);
    this->fmt_size = 16;
    this->fmt_format = 1; //WAVE_FORMAT_PCM = 1
    this->fmt_channels = channels;
    this->fmt_sample_rate = sample_rate;
    this->fmt_block_align = channels*bits_per_sample/8;
    this->fmt_byte_rate = channels*bits_per_sample*sample_rate/8;
    this->fmt_bits_per_sample = bits_per_sample;

    // Set Data-Chunk
    memcpy(this->data_id, "data", 4);
    this->data_size = data_size;
}

WavHeader::WavHeader(QByteArray ba)
{
    QDataStream ds(&ba, QIODevice::ReadOnly);
    ds.setByteOrder(QDataStream::LittleEndian);

    ds.readRawData((char *)riff_id, 4);
    ds >> riff_size;
    ds.readRawData((char *)riff_format, 4);

    ds.readRawData((char *)fmt_id, 4);
    ds >> fmt_size;
    ds >> fmt_format;
    ds >> fmt_channels;
    ds >> fmt_sample_rate;
    ds >> fmt_byte_rate;
    ds >> fmt_block_align;
    ds >> fmt_bits_per_sample;

    ds.readRawData((char *)data_id, 4);
    ds >> data_size;
}

QByteArray WavHeader::getByteArray()
{
    QByteArray ba;
    QDataStream ds(&ba, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);

    ds.writeRawData((const char *)riff_id, 4);
    ds << riff_size;
    ds.writeRawData((const char *)riff_format, 4);

    ds.writeRawData((const char *)fmt_id, 4);
    ds << fmt_size;
    ds << fmt_format;
    ds << fmt_channels;
    ds << fmt_sample_rate;
    ds << fmt_byte_rate;
    ds << fmt_block_align;
    ds << fmt_bits_per_sample;

    ds.writeRawData((const char *)data_id, 4);
    ds << data_size;

    return ba;
}

ALenum WavHeader::getALFormat()
{
    ALenum format = 0;
    if(this->fmt_bits_per_sample == 8)
    {
        if(this->fmt_channels == 1)
            format = AL_FORMAT_MONO8;
        else if(this->fmt_channels == 2)
            format = AL_FORMAT_STEREO8;
    }
    else if(this->fmt_bits_per_sample == 16)
    {
        if(this->fmt_channels == 1)
            format = AL_FORMAT_MONO16;
        else if(this->fmt_channels == 2)
            format = AL_FORMAT_STEREO16;
    }
    if(format == 0)
    {
        qDebug() << "Error: Incompatible WAV format (" << this->fmt_bits_per_sample << "," << this->fmt_channels << "," << this->fmt_size << "," << this->fmt_sample_rate << ")";
        return AL_FORMAT_MONO16;
    }    
    return format;
}

#ifndef VIDEOFFMPEG_H
#define VIDEOFFMPEG_H

#include <QString>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}


class VideoFFmpeg
{
public:
    VideoFFmpeg(QString url);
private:
    QString m_url;
    AVFormatContext *formatContext = nullptr;
    AVCodecContext *codecContext = nullptr;
    SwsContext *swsContext = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *packet = nullptr;
    int videoStreamIndex = -1;
};

#endif // VIDEOFFMPEG_H

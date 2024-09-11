#ifndef SAVEALARMVIDEO_H
#define SAVEALARMVIDEO_H

#include <QString>
#include <QThread>
#include <QMutex>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}

#define MAX_PACKETS 150
#define MAX_SAVE_VIDEO_TEN 300

typedef struct{
   AVPacket packets[MAX_PACKETS];
   int packet_count;
   int start_index;
}PacketBuffer;

typedef struct{
   AVPacket packets[MAX_SAVE_VIDEO_TEN];
   int packet_count;
}S_PacketBuffer;

class saveAlarmVideo : public QThread
{
public:
    saveAlarmVideo();
    void add_packet(AVFormatContext *inputFormatCtx, int videoIndex, AVPacket *packet);
    void saveVideo(QString fileName, AVFormatContext *inputFormatCtx, int videoIndex);
    bool isSaveVideo = false;

protected:
    void run();



private:
    PacketBuffer m_packbufferPre;
    PacketBuffer m_saveVidePre;
    PacketBuffer* m_packbufferPer = nullptr;
    AVFormatContext *o_fmt_ctx = nullptr;
    AVStream *o_video_stream = nullptr;

    AVFormatContext *m_inputFormatCtx = nullptr;
    int m_videoIndex = -1;
    QString m_videoName;
    void init_packet_buffer(PacketBuffer *buffer);

    bool isExternQueen = false;

    S_PacketBuffer m_toSaveBuffer;
    int m_count = 0;
    QMutex mutex;

};

#endif // SAVEALARMVIDEO_H

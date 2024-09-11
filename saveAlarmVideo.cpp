#include "saveAlarmVideo.h"
#include <QDebug>

saveAlarmVideo::saveAlarmVideo()
{
    init_packet_buffer(&m_packbufferPre);
    init_packet_buffer(&m_saveVidePre);
    m_count = 0;
    qDebug() << "saveAlarmVideo()";
}

void saveAlarmVideo::init_packet_buffer(PacketBuffer *buffer)
{
    m_packbufferPer = buffer;
    buffer->start_index = 0;
    buffer->packet_count = 0;
}


void saveAlarmVideo::add_packet(AVFormatContext *inputFormatCtx, int videoIndex, AVPacket *packet)
{
    QMutexLocker locker(&mutex); // 加锁
//     qDebug() << "PTS: " << packet->pts << " DTS: " << packet->dts;
    AVPacket *new_packet = av_packet_clone(packet);
    if (inputFormatCtx)
    {
        // 获取输入流的时间基
        AVRational time_base = inputFormatCtx->streams[videoIndex]->time_base;

        // 设置时间戳
        new_packet->pts = av_rescale_q(m_count, time_base, AV_TIME_BASE_Q); // 根据帧数计算 PTS
        new_packet->pts = new_packet->dts; // 如果没有其他解码时间戳，可以将 DTS 设置为 PTS
    }

//    AVPacket *new_packet = av_packet_clone(packet);
    if(m_packbufferPer->packet_count < MAX_PACKETS)
    {
        qDebug() << "packet_count = " << m_packbufferPer->packet_count;

        m_packbufferPer->packets[m_packbufferPer->packet_count++] = *new_packet;
    }else {
        // 替换最旧的包
        av_packet_unref(&m_packbufferPer->packets[m_packbufferPer->start_index]);
        m_packbufferPer->packets[m_packbufferPer->start_index] = *new_packet;
        m_packbufferPer->start_index = (m_packbufferPer->start_index + 1) % MAX_PACKETS;
        isExternQueen = true;
        qDebug() << "start_index = " << m_packbufferPer->start_index;
    }

    if(isSaveVideo == true)
    {
         av_interleaved_write_frame(o_fmt_ctx, packet);
         m_count++;
//         qDebug() << "m_count = " << m_count;
         if(m_count == MAX_PACKETS)
         {
            av_write_trailer(o_fmt_ctx);
            qDebug() << "Done";
//            avformat_close_input(&o_fmt_ctx);
            if(o_fmt_ctx->pb)
            {
                avio_close(o_fmt_ctx->pb);
            }

            avformat_free_context(o_fmt_ctx);

            isSaveVideo = false;
            m_count = 0;
         }


    }
}

void saveAlarmVideo::run()
{
    AVStream *i_stream=m_inputFormatCtx->streams[m_videoIndex];

    o_fmt_ctx = avformat_alloc_context();
    avformat_alloc_output_context2(&o_fmt_ctx, nullptr, nullptr, m_videoName.toLocal8Bit().data());
    o_video_stream = avformat_new_stream(o_fmt_ctx, nullptr);

    avcodec_parameters_copy(o_video_stream->codecpar,i_stream->codecpar);

    // 设置输出流的时间基和帧率
    o_video_stream->time_base = i_stream->time_base; // 设置时间基
    o_video_stream->r_frame_rate = i_stream->r_frame_rate; // 设置帧率


    o_video_stream->codecpar->codec_tag = 0;

    avio_open(&o_fmt_ctx->pb, m_videoName.toLocal8Bit().data(), AVIO_FLAG_WRITE);

    if(avformat_write_header(o_fmt_ctx, nullptr))
    {
       qDebug()<<"avformat_write_header run error";
    }
    qDebug() << "hello";

    {
        QMutexLocker locker(&mutex); // 加锁
        if(isExternQueen == true)
        {
            int base = m_packbufferPer->start_index;
            for (int i = base; i < base + MAX_PACKETS; i++)
            {
                qDebug() << "i = " << i;
                av_interleaved_write_frame(o_fmt_ctx, &(m_packbufferPer->packets[i % MAX_PACKETS]));
            }
        }else {
            for (int i = 0; i < m_packbufferPer->packet_count; i++)
            {
                av_interleaved_write_frame(o_fmt_ctx, m_packbufferPer->packets + i);
            }
        }
    }


    isSaveVideo = true;
}

void saveAlarmVideo::saveVideo(QString fileName, AVFormatContext *inputFormatCtx, int videoIndex)
{
    m_videoName = fileName;
    m_inputFormatCtx = inputFormatCtx;
    m_videoIndex = videoIndex;

//    m_packbufferPre;
//       PacketBuffer m_saveVidePre;
    {
//        QMutexLocker locker(&mutex);
//        memcpy(&m_saveVidePre, &m_packbufferPre, sizeof(m_saveVidePre));
    }


    this->start();


//    AVStream *i_stream=m_inputFormatCtx->streams[m_videoIndex];

//    o_fmt_ctx = avformat_alloc_context();
//    avformat_alloc_output_context2(&o_fmt_ctx, nullptr, nullptr, m_videoName.toLocal8Bit().data());
//    o_video_stream = avformat_new_stream(o_fmt_ctx, nullptr);

//    avcodec_parameters_copy(o_video_stream->codecpar,i_stream->codecpar);

//    // 设置输出流的时间基和帧率
//    o_video_stream->time_base = i_stream->time_base; // 设置时间基
//    o_video_stream->r_frame_rate = i_stream->r_frame_rate; // 设置帧率


//    o_video_stream->codecpar->codec_tag = 0;

//    avio_open(&o_fmt_ctx->pb, m_videoName.toLocal8Bit().data(), AVIO_FLAG_WRITE);

//    if(avformat_write_header(o_fmt_ctx, nullptr))
//    {
//       qDebug()<<"avformat_write_header run error";
//    }
//    qDebug() << "hello";
//    if(isExternQueen == true)
//    {
//        int base = m_packbufferPer->start_index;
//        for (int i = base; i < base + MAX_PACKETS; i++)
//        {
//            qDebug() << "i = " << i;
//            av_interleaved_write_frame(o_fmt_ctx, &(m_packbufferPer->packets[i % MAX_PACKETS]));
//        }
//    }else {
//        for (int i = 0; i < m_packbufferPer->packet_count; i++)
//        {
//            av_interleaved_write_frame(o_fmt_ctx, m_packbufferPer->packets + i);
//        }
//    }

//    isSaveVideo = true;
}


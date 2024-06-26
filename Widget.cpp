#include "Widget.h"
#include <QtDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
//    av_format_network();
//    avformat_network_init
   avformat_network_init();


       videoLabel = new QLabel(this);
       videoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

       openButton = new QPushButton("Open", this);
       playButton = new QPushButton("Play", this);


       positionSlider = new QSlider(Qt::Horizontal, this);
       positionSlider->setEnabled(true);
       positionSlider->setRange(0, 100); // 将范围设置为 0 到 100，以简化处理
       positionSlider->setValue(0);

       QVBoxLayout *mainLayout = new QVBoxLayout;
       QHBoxLayout *buttonLayout = new QHBoxLayout;
       buttonLayout->addWidget(openButton);
       buttonLayout->addWidget(playButton);

       mainLayout->addWidget(videoLabel);
        mainLayout->addWidget(positionSlider);
       mainLayout->addLayout(buttonLayout);
       setLayout(mainLayout);

       connect(openButton, &QPushButton::clicked, this, &Widget::openFile);
       connect(playButton, &QPushButton::clicked, this, &Widget::playVideo);
       connect(positionSlider, &QSlider::sliderMoved, this, &Widget::seekVideo);

       timer = new QTimer(this);
       connect(timer, &QTimer::timeout, this, &Widget::updateFrame);



       frame = av_frame_alloc();
       packet = av_packet_alloc();
}

void Widget::seekVideo(int position) {
    if (!formatContext || videoStreamIndex == -1) return;

    int64_t seekTarget = static_cast<int64_t>((position / 100.0) * formatContext->duration);
    qDebug() << "formatContext->duration is  " << formatContext->duration / AV_TIME_BASE;
    qDebug() << "formatContext->start_time is  " << formatContext->start_time / AV_TIME_BASE;
     qDebug() << "formatContext->start_time is  " << formatContext->url;
     qDebug() << "av_q2d(formatContext->streams[videoStreamIndex]->time_base) = " << av_q2d(formatContext->streams[videoStreamIndex]->time_base);
     qDebug() << "av_q2d(formatContext->streams[videoStreamIndex]->avg_frame_rate) = " << av_q2d(formatContext->streams[videoStreamIndex]->avg_frame_rate);
     qDebug() << "av_q2d(formatContext->streams[videoStreamIndex]->r_frame_rate) = " << av_q2d(formatContext->streams[videoStreamIndex]->r_frame_rate);
             qDebug() << "av_q2d(formatContext->streams[videoStreamIndex]->nb_frames) = " << formatContext->streams[videoStreamIndex]->nb_frames;
    av_seek_frame(formatContext, videoStreamIndex, seekTarget, AVSEEK_FLAG_BACKWARD);
    avcodec_flush_buffers(codecContext);
}

void Widget::openFile()
{
    filePath = QFileDialog::getOpenFileName(this, "Open Video File", "", "Video Files (*.mp4 *.avi *.mkv)");
    if (filePath.isEmpty()) return;

    if (formatContext)
    {
        avformat_close_input(&formatContext);
        formatContext = nullptr;
    }

    avformat_open_input(&formatContext, filePath.toStdString().c_str(), nullptr, nullptr);
    avformat_find_stream_info(formatContext, nullptr);

    videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
      if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
          videoStreamIndex = i;
          break;
      }
    }

    m_fps = 1000 / static_cast<uint8_t>(av_q2d(formatContext->streams[videoStreamIndex]->r_frame_rate));
    m_frames = formatContext->streams[videoStreamIndex]->nb_frames;

    timer->setInterval((m_fps == 0 ? 30 : m_fps));

    if (videoStreamIndex == -1) {
        QMessageBox::warning(this, "Error", "No video stream found.");
        return;
    }else {
        QMessageBox::warning(this, "success", "video stream found.");
    }

    const AVCodec *codec = avcodec_find_decoder(formatContext->streams[videoStreamIndex]->codecpar->codec_id);

    if (codecContext)
    {
        avcodec_free_context(&codecContext);
        codecContext = nullptr;
    }


    codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, formatContext->streams[videoStreamIndex]->codecpar);
    avcodec_open2(codecContext, codec, nullptr);


     if (swsContext)
     {
         sws_freeContext(swsContext);
         swsContext = nullptr;
     }

    swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                                codecContext->width, codecContext->height, AV_PIX_FMT_RGB24,
                                SWS_BILINEAR, nullptr, nullptr, nullptr);


}
void Widget::playVideo()
{
    if (!formatContext) {
        QMessageBox::warning(this, "Error", "No video file opened.");
        return;
    }

    if(isPlaying)
    {
        timer->stop();
        playButton->setText("play");
    }else {
        timer->start(); // Roughly 30 frames per second
        playButton->setText("pause");
    }

    isPlaying = !isPlaying;
}
void Widget::updateFrame()
{
//    qDebug() << "m_cur_frame / m_frames" << m_cur_frame / m_frames;
    positionSlider->setValue(100*(m_cur_frame / static_cast<double>(m_frames) ));
    decodeVideo();
}


void Widget::decodeVideo()
{
    static int count  =0;
    if (av_read_frame(formatContext, packet) >= 0) {
        qDebug() << "count = " << count++;
        m_cur_frame++;
        if (packet->stream_index == videoStreamIndex) {
//            qDebug() << "pts = " << packet->pts * av_q2d(formatContext->streams[videoStreamIndex]->time_base);
//            qDebug() << "dts = " << packet->dts * av_q2d(formatContext->streams[videoStreamIndex]->time_base);
//            qDebug() << "duration = " << packet->duration * av_q2d(formatContext->streams[videoStreamIndex]->time_base);
            avcodec_send_packet(codecContext, packet);
            if (avcodec_receive_frame(codecContext, frame) == 0) {
                QImage image(codecContext->width, codecContext->height, QImage::Format_RGB888);
                uint8_t *data[AV_NUM_DATA_POINTERS] = { image.bits(), nullptr };
                int linesize[AV_NUM_DATA_POINTERS] = { image.bytesPerLine(), 0 };

                sws_scale(swsContext, frame->data, frame->linesize, 0, codecContext->height, data, linesize);
                videoLabel->setPixmap(QPixmap::fromImage(image));
            }
        }
        av_packet_unref(packet);
    }else {
        count = 0;
        m_cur_frame = 0;
        av_seek_frame(formatContext, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD); // 回到视频开始位置
        avcodec_flush_buffers(codecContext);
    }
}

Widget::~Widget()
{
   if (swsContext) sws_freeContext(swsContext);
   if (codecContext) avcodec_free_context(&codecContext);
   if (formatContext) avformat_close_input(&formatContext);
   if (frame) av_frame_free(&frame);
   if (packet) av_packet_free(&packet);

       avformat_network_deinit();
}

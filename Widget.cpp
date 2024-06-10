#include "Widget.h"

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

       QVBoxLayout *mainLayout = new QVBoxLayout;
       QHBoxLayout *buttonLayout = new QHBoxLayout;
       buttonLayout->addWidget(openButton);
       buttonLayout->addWidget(playButton);

       mainLayout->addWidget(videoLabel);
       mainLayout->addLayout(buttonLayout);
       setLayout(mainLayout);

       connect(openButton, &QPushButton::clicked, this, &Widget::openFile);
       connect(playButton, &QPushButton::clicked, this, &Widget::playVideo);

       timer = new QTimer(this);
       connect(timer, &QTimer::timeout, this, &Widget::updateFrame);

       frame = av_frame_alloc();
       packet = av_packet_alloc();
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

    timer->start(30); // Roughly 30 frames per second
}
void Widget::updateFrame()
{
    decodeVideo();
}


void Widget::decodeVideo()
{
    if (av_read_frame(formatContext, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
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

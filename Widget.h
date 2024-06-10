#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QSlider>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void openFile();
    void playVideo();
    void updateFrame();
    void seekVideo(int position);

private:
    void decodeVideo();

    AVFormatContext *formatContext = nullptr;
    AVCodecContext *codecContext = nullptr;
    SwsContext *swsContext = nullptr;
    AVFrame *frame = nullptr;
    AVPacket *packet = nullptr;
    int videoStreamIndex = -1;

    QTimer *timer = nullptr;
    QLabel *videoLabel = nullptr;
    QPushButton *openButton = nullptr;
    QPushButton *playButton = nullptr;
    QString filePath;
    bool isPlaying = false;
    QSlider *positionSlider = nullptr;
};

#endif // WIDGET_H

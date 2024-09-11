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
#include <QMutex>
#include <saveAlarmVideo.h>

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
    void saveVideo();
    void openFile();
    void playVideo();
    void updateFrame();
    void seekVideo(int position);

protected:
    void paintEvent(QPaintEvent *event) override;

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
    QPushButton *saveBtn = nullptr;
    QString filePath;
    bool isPlaying = false;
    QSlider *positionSlider = nullptr;
    int m_fps = 0;
    int64_t m_frames = 0;
    int64_t m_cur_frame = 0;

    QImage m_image;
    QMutex m_imageMutex;

    saveAlarmVideo *m_videosave;
};

#endif // WIDGET_H

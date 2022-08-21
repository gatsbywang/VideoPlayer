//
// Created by 花歹 on 2022/7/20.
//

#ifndef MY_APPLICATION_DZFFMPEG_H
#define MY_APPLICATION_DZFFMPEG_H

#include "DZJNICall.h"
#include "DZAudio.h"
#include <pthread.h>
#include "DZConstDefine.h"
#include "DZVideo.h"

extern "C"{
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};



class DZFFmpeg {
public:
    AVFormatContext *pFormatContext = NULL;
//    AVCodecContext *pCodecContext = NULL;
//    SwrContext *swrContext = NULL;
    char* url = NULL;
    DZJNICall *pJniCall = NULL;
    DZAudio *pAudio = NULL;
    DZVideo *pVideo = NULL;
    DZPlayerStatus *pPlayerStatus = NULL;

public:
    DZFFmpeg(DZJNICall *dzjniCall, const char *url);
    ~DZFFmpeg();

public:

    void play();

    void prepare();

    void prepareAsync();

    void prepare(ThreadMode threadMode);

    void callPlayerJniError(ThreadMode threadMode,int code,char *msg);

    void setSurface(jobject surface);

    void release();
};


#endif //MY_APPLICATION_DZFFMPEG_H

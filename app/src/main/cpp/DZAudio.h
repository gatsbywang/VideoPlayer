//
// Created by 花歹 on 2022/7/29.
//

#ifndef DEMOOPENSLES_DZAUDIO_H
#define DEMOOPENSLES_DZAUDIO_H


#include "DZJNICall.h"
#include "DZConstDefine.h"
#include "DZPacketQueue.h"
#include "DZPlayerStatus.h"
#include "DZMedia.h"
#include <pthread.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

extern "C"{
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};

class DZAudio : public DZMedia{
public:
    AVFormatContext *pFormatContext = NULL;
//    AVCodecContext *pCodecContext = NULL;
    SwrContext *pSwrContext = NULL;
//    DZJNICall *pJniCall = NULL;
    uint8_t * resampleOutBuffer = NULL;
//    int audioStreamIndex = -1;
//    DZPacketQueue *pPacketQueue = NULL;
//    DZPlayerStatus *pPlayerStatus = NULL;

public:
    DZAudio(int audioStreamIndex,DZJNICall *pJniCal,DZPlayerStatus *pPlayStatus);

    ~DZAudio();

    void play();

    void initCrateOpenSLES();

    int resampleAudio();

    void privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext);

    void release();
};


#endif //DEMOOPENSLES_DZAUDIO_H

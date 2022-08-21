//
// Created by 花歹 on 2022/8/4.
//

#ifndef VIDEOPLAYER_DZVIDEO_H
#define VIDEOPLAYER_DZVIDEO_H


#include "DZMedia.h"
#include <android/native_window.h>
#include <android/native_window_jni.h>

extern "C"{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
};

class DZVideo : public DZMedia{

public:
    SwsContext *pSwsContext = NULL;

    uint8_t *pFrameBuffer = NULL;

    AVFrame  *pRgbaFrame = NULL;

    int frameSize;

    jobject surface;



public:
    DZVideo(int videoStreamIndex,DZJNICall *pJniCall,DZPlayerStatus *pPlayerStatus);
    ~DZVideo();

public:
    void play();

    void privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext);

    void setSurface(jobject surface);

    void release();
};


#endif //VIDEOPLAYER_DZVIDEO_H

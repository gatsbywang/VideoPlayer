//
// Created by 花歹 on 2022/8/4.
//

#ifndef VIDEOPLAYER_DZVIDEO_H
#define VIDEOPLAYER_DZVIDEO_H


#include "DZMedia.h"
#include "DZAudio.h"
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

    DZAudio *pAudio;

    /**
     * 视频的延时时间
     */
    double delayTime =0;

    /**
     * 默认情况下最适合的一个延迟时间，需要动态获取
     */
    double defaultDelayTime =0.04;

public:
    DZVideo(int videoStreamIndex,DZJNICall *pJniCall,DZPlayerStatus *pPlayerStatus,DZAudio *pAudio);
    ~DZVideo();

public:
    void play();

    void privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext);

    void setSurface(jobject surface);

    void release();

    /**
     * 视频同步音频，计算获取休眠的时间
     * @param pFrame 当前视频帧
     * @return 休眠时间（s）
     */
    double getFrameSleepTime(AVFrame *pFrame);
};


#endif //VIDEOPLAYER_DZVIDEO_H

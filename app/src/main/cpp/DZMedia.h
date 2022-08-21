//
// Created by 花歹 on 2022/8/4.
//

#ifndef VIDEOPLAYER_DZMEDIA_H
#define VIDEOPLAYER_DZMEDIA_H

#include "DZJNICall.h"
#include "DZPacketQueue.h"
#include "DZPlayerStatus.h"
#include "DZConstDefine.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
};

class DZMedia {
public:
    int streamIndex  = -1;
    AVCodecContext *pCodecContext = NULL;
    DZJNICall *pJniCall = NULL;
    DZPacketQueue *pPacketQueue = NULL;
    DZPlayerStatus *pPlayerStatus = NULL;

    /**
     * 记录当前播放时间
     */
    double currentTime = 0;

    /**
     * 上次更新的时间（回调到java层）
     */
    double lastUpdateTime = 0;

    /**
     *  时间基
     */
    AVRational  timeBase;

    /**
     * 整个视频的时长
     */
    int duration = 0;

public:
    DZMedia(int streamIndex,DZJNICall *pJniCall,DZPlayerStatus *pPlayerStatus);
    ~DZMedia();

public:
    virtual void play()=0;

    void analysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext);

    virtual void privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext) = 0;

    void release();

public:

    void publicAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext);

    void callPlayerJniError(ThreadMode threadMode, int code, char *msg);
};


#endif //VIDEOPLAYER_DZMEDIA_H

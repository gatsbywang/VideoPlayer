//
// Created by 花歹 on 2022/7/20.
//

#include "DZFFmpeg.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>


DZFFmpeg::DZFFmpeg(DZJNICall *dzjniCall, const char *url) {

    this->pJniCall = dzjniCall;
    // 多线程下url 有可能会被消耗，需要复制一份
    this->url = static_cast<char *>(malloc(strlen(url) + 1));

    memcpy(this->url, url, strlen(url) + 1);

    pPlayerStatus = new DZPlayerStatus();
}

DZFFmpeg::~DZFFmpeg() {
    release();
}

void DZFFmpeg::release() {

//    if (pCodecContext != NULL) {
//
//        avcodec_close(pCodecContext);
//        avcodec_free_context(&pCodecContext);
//        pCodecContext = NULL;
//    }
//
    if (pFormatContext != NULL) {
        avformat_close_input(&pFormatContext);
        avformat_free_context(pFormatContext);
        pFormatContext = NULL;
    }

//    if (swrContext != NULL) {
//        swr_close(swrContext);
//        swr_free(&swrContext);
//        swrContext = NULL;
//    }

    avformat_network_deinit();

    if (url != NULL) {
        free(url);
        url = NULL;
    }

    if(pPlayerStatus !=NULL){
        delete (pPlayerStatus);
        pPlayerStatus = NULL;
    }

    if (pAudio != NULL) {
        delete (pAudio);
        pAudio = NULL;
    }

    if (pVideo != NULL) {
        delete (pVideo);
        pVideo = NULL;
    }

}

void *threadReadPacket(void *context) {

    DZFFmpeg *pFFmpeg = static_cast<DZFFmpeg *>(context);

    while (pFFmpeg->pPlayerStatus != NULL && !pFFmpeg->pPlayerStatus->isExit) {
        AVPacket *pPacket = av_packet_alloc();
        // av_read_frame 从流里面解析一个一个的packat(压缩数据),
        if (av_read_frame(pFFmpeg->pFormatContext, pPacket) >= 0) {
            if (pPacket->stream_index == pFFmpeg->pAudio->streamIndex) {
                pFFmpeg->pAudio->pPacketQueue->push(pPacket);

            } else if(pPacket->stream_index == pFFmpeg->pVideo->streamIndex){
                pFFmpeg->pVideo->pPacketQueue->push(pPacket);
            }else{
                av_packet_free(&pPacket);
            }
        } else {
            av_packet_free(&pPacket);
            // 睡眠，不去消耗cpu资源或者退出循环，销毁线程
//            break;
        }

    }
    return 0;
}

void DZFFmpeg::play() {

    //一个线程读取packet
    pthread_t readPacketThreadT;
    pthread_create(&readPacketThreadT, NULL, threadReadPacket, this);
    pthread_detach(readPacketThreadT);

    if(pAudio !=NULL){
        pAudio->play();
    }

    if(pVideo !=NULL){
        pVideo->play();
    }
}

void DZFFmpeg::prepare() {
    prepare(THREAD_MAIN);
}

void *threadPrepare(void *context) {
    DZFFmpeg *pFFmpeg = (DZFFmpeg *) context;
    pFFmpeg->prepare(THREAD_CHILD);
    return 0;
}

void DZFFmpeg::prepareAsync() {
    // 创建一个线程去播放，多线程编解码边播放
    pthread_t prepareThreadT;
    pthread_create(&prepareThreadT, NULL, threadPrepare, this);
    pthread_detach(prepareThreadT);
}


void DZFFmpeg::prepare(ThreadMode threadMode) {

    av_register_all();
    avformat_network_init();

    int formatOpenInputRes = 0;
    int formatFindStreamInfoRes = 0;



    formatOpenInputRes = avformat_open_input(&pFormatContext, url, NULL, NULL);
    if (formatOpenInputRes != 0) {
        // 打开失败了，要释放资源
        LOGE("format open input error:%s", av_err2str(formatOpenInputRes));
        callPlayerJniError(threadMode, formatFindStreamInfoRes, av_err2str(formatOpenInputRes));
        return;
    }


    formatFindStreamInfoRes = avformat_find_stream_info(pFormatContext, NULL);
    if (formatFindStreamInfoRes < 0) {
        LOGE("format find stream info error:%s", av_err2str(formatFindStreamInfoRes));
        callPlayerJniError(threadMode, formatFindStreamInfoRes,
                           av_err2str(formatFindStreamInfoRes));
        return;
    }

    // 这是找音频流（mp4中可能存在音频流和视频流）
    int audioStreamIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_AUDIO,
                                           -1, -1, NULL, 0);
    if (audioStreamIndex < 0) {
        LOGE("find best stream error:%s", av_err2str(audioStreamIndex));
        callPlayerJniError(threadMode, FIND_STREAM_ERROR_CODE, "find audio stream error");
        return;
    }


    // 不是我的事我不干，
    pAudio = new DZAudio(audioStreamIndex,pJniCall,pPlayerStatus);
    pAudio->analysisStream(threadMode,pFormatContext);


    // 这是找视频流（mp4中可能存在音频流和视频流）
    int videoStreamIndex = av_find_best_stream(pFormatContext, AVMediaType::AVMEDIA_TYPE_VIDEO,
                                               -1, -1, NULL, 0);
    if (videoStreamIndex < 0) {
        LOGE("find best stream error:%s", av_err2str(audioStreamIndex));
        callPlayerJniError(threadMode, FIND_STREAM_ERROR_CODE, "find video stream error");
        return;
    }

    // 不是我的事我不干，
    pVideo = new DZVideo(videoStreamIndex,pJniCall,pPlayerStatus);
    pVideo->analysisStream(threadMode,pFormatContext);


    // ---------------重采样 end

    // 回调java 准备好了
    pJniCall->callPlayerPrepared(threadMode);

}


void DZFFmpeg::callPlayerJniError(ThreadMode threadMode, int code, char *msg) {

    //回收资源
    release();
    // 回调java层
    pJniCall->callPlayerError(threadMode, code, msg);
}

void DZFFmpeg::setSurface(jobject surface) {
    if(pVideo !=NULL){
        pVideo -> setSurface(surface);
    }
}
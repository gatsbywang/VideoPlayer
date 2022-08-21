//
// Created by 花歹 on 2022/8/4.
//

#include "DZMedia.h"

DZMedia::DZMedia(int streamIndex, DZJNICall *pJniCall, DZPlayerStatus *pPlayerStatus) {
    this->streamIndex = streamIndex;
    this->pJniCall = pJniCall;
    this->pPlayerStatus = pPlayerStatus;
    pPacketQueue =  new DZPacketQueue();
}

DZMedia::~DZMedia() {
    release();
}

void DZMedia::analysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext) {

        publicAnalysisStream(threadMode,pFormatContext);
        privateAnalysisStream(threadMode,pFormatContext);
}

void DZMedia::release() {

    if(pPlayerStatus != NULL){
        delete (pPlayerStatus);
        pPlayerStatus = NULL;
    }

    if (pCodecContext != NULL) {

        avcodec_close(pCodecContext);
        avcodec_free_context(&pCodecContext);
        pCodecContext = NULL;
    }

    if (pPacketQueue != NULL) {
        delete (pPacketQueue);
        pPacketQueue = NULL;
    }
}

void DZMedia::callPlayerJniError(ThreadMode threadMode, int code, char *msg) {

    //回收资源
    release();
    // 回调java层
    pJniCall->callPlayerError(threadMode, code, msg);
}

void DZMedia::publicAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext) {

    // 查找解码器
    AVCodecParameters *pCodecParameters = pFormatContext->streams[streamIndex]->codecpar;
    AVCodec *avCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (avCodec == NULL) {
        LOGE("codec find audio decoder error");
        callPlayerJniError(threadMode, CODEC_FIND_DECODER_ERROR_CODE,
                           "codec find audio decoder error");
        return;
    }

    // 打开解码器
    pCodecContext = avcodec_alloc_context3(avCodec);
    if (pCodecContext == NULL) {
        LOGE("codec alloc context error");
        callPlayerJniError(threadMode, CODEC_ALLOC_CONTEXT_ERROR_CODE,
                           "codec find audio decoder error");
        return;
    }

    int codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext, pCodecParameters);
    if (codecParametersToContextRes < 0) {
        LOGE("codec parameters to context error:%s", av_err2str(codecParametersToContextRes));
        callPlayerJniError(threadMode, codecParametersToContextRes,
                           av_err2str(codecParametersToContextRes));
        return;
    }

    int codecOpenRes = avcodec_open2(pCodecContext, avCodec, NULL);
    if (codecOpenRes != 0) {
        LOGE("codec audio open error:%s", av_err2str(codecParametersToContextRes));
        callPlayerJniError(threadMode, codecOpenRes, av_err2str(codecParametersToContextRes));
        return;
    }

    duration = pFormatContext->duration;
    timeBase = pFormatContext->streams[streamIndex]->time_base;

}


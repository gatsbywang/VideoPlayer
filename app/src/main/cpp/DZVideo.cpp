//
// Created by 花歹 on 2022/8/4.
//

#include "DZVideo.h"

DZVideo::DZVideo(int videoStreamIndex, DZJNICall *pJniCall, DZPlayerStatus *pPlayerStatus,DZAudio *pAudio) : DZMedia(
        videoStreamIndex, pJniCall, pPlayerStatus) {
    this->pAudio = pAudio;
}

DZVideo::~DZVideo() {
    release();
}

void *threadVideoPlay(void *context) {
    DZVideo *pVideo = static_cast<DZVideo *>(context);

    // 获取当前线程的 JNIEnv， 通过 JavaVM
    JNIEnv *env;
    if (pVideo->pJniCall->javaVM->AttachCurrentThread(&env, 0) != JNI_OK) {
        LOGE("get child thread jniEnv error!");
        return 0;
    }

    // 1. 获取窗体
    ANativeWindow *pNativeWindow = ANativeWindow_fromSurface(env, pVideo->surface);
    pVideo->pJniCall->javaVM->DetachCurrentThread();

    // 2. 设置缓存区的数据
    ANativeWindow_setBuffersGeometry(pNativeWindow, pVideo->pCodecContext->width,
                                     pVideo->pCodecContext->height, WINDOW_FORMAT_RGBA_8888);


    // Window 缓冲区的 Buffer
    ANativeWindow_Buffer outBuffer;

    AVPacket *pPacket = NULL;
    AVFrame *pFrame = av_frame_alloc();

    while (pVideo->pPlayerStatus != NULL && !pVideo->pPlayerStatus->isExit) {
        // 这个里面不断从队列里取
        pPacket = pVideo->pPacketQueue->pop();

        // Packet 包，压缩的数据，解码成pcm数据
        int codecSendPacketRes = avcodec_send_packet(pVideo->pCodecContext, pPacket);
        if (codecSendPacketRes == 0) {
            int codecReceiveFrameRes = avcodec_receive_frame(pVideo->pCodecContext, pFrame);
            if (codecReceiveFrameRes == 0) {

                // 渲染，显示，OpenGLES (高效，硬件支持)，SurfaceView
                // 硬件加速和不加速有什么区别？cpu 主要是用于计算，gpu 图像支持（硬件）
                // 这个 pFrame->data , 一般 yuv420P 的，RGBA8888，因此需要转换
                // 假设拿到了转换后的 RGBA 的 data 数据，如何渲染，把数据推到缓冲区
                sws_scale(pVideo->pSwsContext,
                        (const uint8_t *const *) pFrame->data,
                          pFrame->linesize,
                          0,
                          pVideo->pCodecContext->height,
                          (uint8_t *const *)(pVideo->pRgbaFrame->data),
                          pVideo->pRgbaFrame->linesize);

                // 在播放之前判断一下需要休眠多久,为了同步音频和视频
                double frameSleepTime = pVideo->getFrameSleepTime(pFrame);
                av_usleep(frameSleepTime * 1000000);// 微秒 10的6次方

                // 把数据推到缓冲区
                ANativeWindow_lock(pNativeWindow, &outBuffer, NULL);
                memcpy(outBuffer.bits, pVideo->pFrameBuffer, pVideo->frameSize);
                ANativeWindow_unlockAndPost(pNativeWindow);

            }
        }

        // 解引用,资源释放，
        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
    }

    // 释放pPacket的内存
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);


    return 0;
}

void DZVideo::play() {

    // 创建一个线程去播放，多线程编解码边播放
    pthread_t playThreadT;
    pthread_create(&playThreadT, NULL, threadVideoPlay, this);
    pthread_detach(playThreadT);
}

void DZVideo::privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext) {
    pSwsContext = sws_getContext(pCodecContext->width, pCodecContext->height,
                                 pCodecContext->pix_fmt, pCodecContext->width, pCodecContext->height,
                                 AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);
    pRgbaFrame = av_frame_alloc();
    frameSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, pCodecContext->width,
                                         pCodecContext->height, 1);
    pFrameBuffer = (uint8_t *) malloc(frameSize);
    av_image_fill_arrays(const_cast<uint8_t **>(pRgbaFrame->data), pRgbaFrame->linesize, pFrameBuffer, AV_PIX_FMT_RGBA,
                         pCodecContext->width, pCodecContext->height, 1);


    // 计算视频帧率。
    int num = pFormatContext->streams[streamIndex]->avg_frame_rate.num;
    int den = pFormatContext->streams[streamIndex]->avg_frame_rate.den;

    if(den !=0 && num !=0){
        defaultDelayTime = 1.0f*den/num;
    }

}

void DZVideo::setSurface(jobject surface) {
    this->surface = pJniCall->jniEnv->NewGlobalRef(surface);
}

double DZVideo::getFrameSleepTime(AVFrame *pFrame) {

    double  times = av_frame_get_best_effort_timestamp(pFrame)* av_q2d(timeBase);
    if(times > currentTime){
        currentTime = times;
    }

    // 尽量把时间 控制在视频的帧率时间范围左右，1/24,0.04  1/30  0.033

    // 第一次控制0.016s 到 -0.016s

    // 相差多少秒
    double diffTime = currentTime - pAudio->currentTime;

    if(diffTime > 0.016 || diffTime < -0.016){
        if(diffTime > 0.016){

            delayTime =delayTime *2/3;
        } else if(diffTime < -0.016){
            delayTime =delayTime *3/2;
        }

        // 第二次控制 ，defaultDelayTime *2/3 到defaultDelayTime *3/2
        if(delayTime < defaultDelayTime/2){
            delayTime = defaultDelayTime*2/3;
        }else if(delayTime > defaultDelayTime *2){
            delayTime = defaultDelayTime*3/2;
        }
    }

    // 第三次控制，那这基本属于异常其情况
    if(diffTime >=0.25){
        delayTime  = 0;
    }else if(delayTime <= -0.25){
        delayTime = defaultDelayTime*2;
    }

    return delayTime;
}

void DZVideo::release() {
    DZMedia::release();

    if (pSwsContext != NULL) {
        sws_freeContext(pSwsContext);
        free(pSwsContext);
        pSwsContext = NULL;
    }

    if (pFrameBuffer != NULL) {
        free(pFrameBuffer);
        pFrameBuffer = NULL;
    }

    if(pRgbaFrame!=NULL){
        av_frame_free(&pRgbaFrame);
        pRgbaFrame = NULL;
    }

    if(pJniCall != NULL){
        pJniCall->jniEnv->DeleteGlobalRef(surface);
    }

}

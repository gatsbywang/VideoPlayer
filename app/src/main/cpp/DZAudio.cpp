//
// Created by 花歹 on 2022/7/29.
//

#include "DZAudio.h"

DZAudio::DZAudio(int audioStreamIndex, DZJNICall *pJniCall,DZPlayerStatus *pPlayStatus)
:DZMedia(audioStreamIndex,pJniCall,pPlayStatus){

//    AVCodecContext *pCodecContext,
//    AVFormatContext *pFormatContext, SwrContext *pSwrContext
//    this->audioStreamIndex = audioStreamIndex;
//    this->pJniCall = pJniCall;
//    this->pCodecContext = pCodecContext;
//    this->pFormatContext = pFormatContext;
//    this->pSwrContext = pSwrContext;
    // 1帧数据，单元大小2字节，2通道
//    resampleOutBuffer = (uint8_t *) malloc(pCodecContext->frame_size * 2 * 2);
//    pPacketQueue = new DZPacketQueue();
//    pPlayerStatus = new DZPlayerStatus();


}

// 此线程不断从一帧中读packet
//void *threadReadPacket(void *context) {
//
//    DZAudio *pDZAudio = static_cast<DZAudio *>(context);
//
//    while (pDZAudio->pPlayerStatus != NULL && !pDZAudio->pPlayerStatus->isExit) {
//        AVPacket *pPacket = av_packet_alloc();
//        // av_read_frame 从流里面解析一个一个的packat(压缩数据),
//        if (av_read_frame(pDZAudio->pFormatContext, pPacket) >= 0) {
//            if (pPacket->stream_index == pDZAudio->streamIndex) {
//                pDZAudio->pPacketQueue->push(pPacket);
//
//            } else {
//                av_packet_free(&pPacket);
//            }
//        } else {
//            av_packet_free(&pPacket);
//            // 睡眠，不去消耗cpu资源或者退出循环，销毁线程
//            break;
//        }
//
//    }
//    return 0;
//}

void *threadAudioPlay(void *context) {
    DZAudio *pDZAudio = static_cast<DZAudio *>(context);
    pDZAudio->initCrateOpenSLES();
    return 0;
}

void DZAudio::play() {
    //一个线程读取packet
//    pthread_t readPacketThreadT;
//    pthread_create(&readPacketThreadT, NULL, threadReadPacket, this);
//    pthread_detach(readPacketThreadT);


    // 创建一个线程去播放，多线程编解码边播放
    pthread_t playThreadT;
    pthread_create(&playThreadT, NULL, threadAudioPlay, this);
    pthread_detach(playThreadT);
}

void playerCallback(
        SLAndroidSimpleBufferQueueItf caller,
        void *pContext) {
    DZAudio *pDZAudio = (DZAudio *) pContext;
    // 解码
    int dataSize = pDZAudio->resampleAudio();

    //
    (*caller)->Enqueue(caller, pDZAudio->resampleOutBuffer, dataSize);

}

void DZAudio::initCrateOpenSLES() {
    //    opensles步骤：
//    1、创建引擎接口对象
    SLObjectItf slObjectItf = NULL;
    SLEngineItf slEngineItf = NULL;
    slCreateEngine(&slObjectItf, 0, NULL, 0, NULL, NULL);
    (*slObjectItf)->Realize(slObjectItf, SL_BOOLEAN_FALSE);
    (*slObjectItf)->GetInterface(slObjectItf, SL_IID_ENGINE, &slEngineItf);
//    2、设置混音器
    SLObjectItf outputMixObject = NULL;
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    (*slEngineItf)->CreateOutputMix(slEngineItf, &outputMixObject, 1, ids, req);
    (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                     &outputMixEnvironmentalReverb);
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
    (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb,
                                                                      &reverbSettings);

//    3、创建播放器
    SLObjectItf pPlayer = NULL;
    SLPlayItf pPlayItf = NULL;
    SLDataLocator_AndroidSimpleBufferQueue simpleBufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM formatPcm = {
            SL_DATAFORMAT_PCM,//PCM数据
            2,//2通道
            SL_SAMPLINGRATE_44_1,//44100
            SL_PCMSAMPLEFORMAT_FIXED_16,//2字节
            SL_PCMSAMPLEFORMAT_FIXED_16,//2字节
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//2通道
            SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource audioSrc = {&simpleBufferQueue, &formatPcm};
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};

    // SL_IID_VOLUME 声音大小  SL_IID_PLAYBACKRATE 声音变速

    const SLInterfaceID interfaceIds[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_PLAYBACKRATE};
    const SLboolean interfaceRequired[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    (*slEngineItf)->CreateAudioPlayer(slEngineItf, &pPlayer, &audioSrc, &audioSnk, 3, interfaceIds,
                                      interfaceRequired);
    (*pPlayer)->Realize(pPlayer, SL_BOOLEAN_FALSE);
    (*pPlayer)->GetInterface(pPlayer, SL_IID_PLAY, &pPlayItf);

//    4、设置缓存队列和回调函数
    SLAndroidSimpleBufferQueueItf playerBufferQueue;
    (*pPlayer)->GetInterface(pPlayer, SL_IID_BUFFERQUEUE, &playerBufferQueue);
    // 每次回调，this会被带给playerCallback 的context
    (*playerBufferQueue)->RegisterCallback(playerBufferQueue, playerCallback, this);
//5、设置播放状态
    (*pPlayItf)->SetPlayState(pPlayItf, SL_PLAYSTATE_PLAYING);
//    5、调用下回调函数
    playerCallback(playerBufferQueue, this);
}

int DZAudio::resampleAudio() {

    //开始读取帧，播放
    // 解码
    int dataSize = 0;
    AVPacket *pPacket = NULL;
    AVFrame *pFrame = av_frame_alloc();
    while (pPlayerStatus != NULL && !pPlayerStatus->isExit) {
        // 这个里面不断从队列里取
        pPacket = pPacketQueue->pop();

        // Packet 包，压缩的数据，解码成pcm数据
        int codecSendPacketRes = avcodec_send_packet(pCodecContext, pPacket);
        if (codecSendPacketRes == 0) {
            int codecReceiveFrameRes = avcodec_receive_frame(pCodecContext, pFrame);
            if (codecReceiveFrameRes == 0) {
                // AVPacker -> AVFrame
                // 1、采取OpenSLES ,系统内置的播放框架（跨平台）
                LOGE("解码音频帧");

                // 调用重采样方法,返回值返回的是重采样的个数，也就是pFrame->nb_samples
                dataSize = swr_convert(pSwrContext, &resampleOutBuffer, pFrame->nb_samples,
                                       (const uint8_t **) (pFrame->data), pFrame->nb_samples);

                // 每个点2个字节，双通道
                dataSize = dataSize * 2 * 2;

                // 设置下当前的时间，方便回调进度给java，方便视频同步音频
                double times = pFrame->pts * av_q2d(timeBase);
                if(times > currentTime ){
                    currentTime = times;
                }


                LOGE("解码音频帧：%d，%d", dataSize, pFrame->nb_samples);

                break;
            }
        }

        // 解引用,资源释放，
        av_packet_unref(pPacket);
        av_frame_unref(pFrame);
    }

    // 释放pPacket的内存
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    return dataSize;
}

DZAudio::~DZAudio() {
    release();
}

//void DZAudio::callPlayerJniError(ThreadMode threadMode, int code, char *msg) {
//
//    //回收资源
//    release();
//    // 回调java层
//    pJniCall->callPlayerError(threadMode, code, msg);
//}

void DZAudio::privateAnalysisStream(ThreadMode threadMode, AVFormatContext *pFormatContext) {
    // 查找解码器
//    AVCodecParameters *pCodecParameters = streams[audioStreamIndex]->codecpar;
//    AVCodec *avCodec = avcodec_find_decoder(pCodecParameters->codec_id);
//    if (avCodec == NULL) {
//        LOGE("codec find audio decoder error");
//        callPlayerJniError(threadMode, CODEC_FIND_DECODER_ERROR_CODE,
//                           "codec find audio decoder error");
//        return;
//    }
//
//    // 打开解码器
//    pCodecContext = avcodec_alloc_context3(avCodec);
//    if (pCodecContext == NULL) {
//        LOGE("codec alloc context error");
//        callPlayerJniError(threadMode, CODEC_ALLOC_CONTEXT_ERROR_CODE,
//                           "codec find audio decoder error");
//        return;
//    }
//
//    int codecParametersToContextRes = avcodec_parameters_to_context(pCodecContext, pCodecParameters);
//    if (codecParametersToContextRes < 0) {
//        LOGE("codec parameters to context error:%s", av_err2str(codecParametersToContextRes));
//        callPlayerJniError(threadMode, codecParametersToContextRes,
//                           av_err2str(codecParametersToContextRes));
//        return;
//    }
//
//    int codecOpenRes = avcodec_open2(pCodecContext, avCodec, NULL);
//    if (codecOpenRes != 0) {
//        LOGE("codec audio open error:%s", av_err2str(codecParametersToContextRes));
//        callPlayerJniError(threadMode, codecOpenRes, av_err2str(codecParametersToContextRes));
//        return;
//    }

    // ---------------重采样 start -----------------------

    //输出的声道布局（立体声）
    int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
    //输出采样率
    int out_sample_rate = AUDIO_SAMPLE_RATE;
    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
    int64_t in_ch_layout = pCodecContext->channel_layout;
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = pCodecContext->sample_fmt;
    //输入采样率
    int in_sample_rate = pCodecContext->sample_rate;

    pSwrContext = swr_alloc_set_opts(NULL, out_ch_layout, out_sample_fmt, out_sample_rate,
                                    in_ch_layout, in_sample_fmt, in_sample_rate, 0, NULL);
    if (pSwrContext == NULL) {

        LOGE("swr alloc set opts error");
        callPlayerJniError(threadMode, SWR_ALLOC_SET_OPTS_ERROR_CODE, "swr alloc set opts error");
        return;
    }

    int swrInitRes = swr_init(pSwrContext);
    if (swrInitRes < 0) {
        LOGE("swr_init error");
        callPlayerJniError(threadMode, SWR_INIT_ERROR_CODE, "swr_init error");
        return;
    }

    resampleOutBuffer = (uint8_t *) malloc(pCodecContext->frame_size * 2 * 2);
    // ---------------重采样 end
}

void DZAudio::release() {

//    if (pCodecContext != NULL) {
//
//        avcodec_close(pCodecContext);
//        avcodec_free_context(&pCodecContext);
//        pCodecContext = NULL;
//    }

//    if (pFormatContext != NULL) {
//        avformat_close_input(&pFormatContext);
//        avformat_free_context(pFormatContext);
//        pFormatContext = NULL;
//    }


    DZMedia::release();
    if (pSwrContext != NULL) {
        swr_close(pSwrContext);
        swr_free(&pSwrContext);
        pSwrContext = NULL;
    }

//    if (pPacketQueue != NULL) {
//        delete (pPacketQueue);
//        pPacketQueue = NULL;
//    }

    if (resampleOutBuffer) {
        resampleOutBuffer = NULL;
    }

//    if(pPlayerStatus != NULL){
//        delete (pPlayerStatus);
//        pPlayerStatus = NULL;
//    }
}

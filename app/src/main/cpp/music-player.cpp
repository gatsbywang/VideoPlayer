//
// Created by 花歹 on 2022/6/30.
//
// 系统提供的MediaPlayer 不支持格式，噪音等很多不兼容
//
//
//

#include <jni.h>

// 在C++中采用C的编译方式
extern "C" {
    #include "libavformat/avformat.h"
    #include "libswresample/swresample.h"
}
#include "DZConstDefine.h"
#include "DZJNICall.h"
#include "DZFFmpeg.h"

DZJNICall *pJniCall;
DZFFmpeg *pFFmpeg;
JavaVM *pJavaVM = NULL;

extern "C"
JNIEXPORT  jint JNICALL JNI_onLoad(JavaVM *javaVm,void *reserved){
    LOGE("JNI_OnLoad -->");
    pJavaVM = javaVm;
    JNIEnv *env = NULL;
    if(javaVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_4) != JNI_OK){
        return -1;
    }

    return JNI_VERSION_1_4;
}



extern "C"
JNIEXPORT void JNICALL
Java_com_example_videoplayer_CustomPlayer_nPlay(JNIEnv *env, jobject instance) {

    if(pFFmpeg !=NULL){
        pFFmpeg->play();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_videoplayer_CustomPlayer_nPrepare(JNIEnv *env, jobject instance, jstring url_) {
    // instance 和 env ,url如果涉及到多线程，需要另外处理
    // env 在子线程中通过javaVm->GetEnv获取，instance需要通过env->newGlobalRef获取
    const  char  *url = env->GetStringUTFChars(url_,0);
    if(pFFmpeg ==NULL){
        pJniCall = new DZJNICall(pJavaVM,env,instance);
        pFFmpeg = new DZFFmpeg(pJniCall,url);
        pFFmpeg->prepare();
    }
    env->ReleaseStringUTFChars(url_,url);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_videoplayer_CustomPlayer_nPrepareAsync(JNIEnv *env, jobject instance, jstring url_) {

    const  char  *url = env->GetStringUTFChars(url_,0);
    if(pFFmpeg ==NULL){
        pJniCall = new DZJNICall(pJavaVM,env,instance);
        pFFmpeg = new DZFFmpeg(pJniCall,url);
        pFFmpeg->prepareAsync();
    }
    env->ReleaseStringUTFChars(url_,url);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_videoplayer_CustomPlayer_setSurface(JNIEnv *env, jobject thiz, jobject surface) {

    if(pFFmpeg !=NULL){
        pFFmpeg->setSurface(surface);
    }
}
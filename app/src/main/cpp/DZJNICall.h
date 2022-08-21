//
// Created by 花歹 on 2022/7/8.
//

#include <jni.h>

#ifndef MY_APPLICATION_DZJNICALL_H
#define MY_APPLICATION_DZJNICALL_H

enum ThreadMode{
    THREAD_CHILD,THREAD_MAIN
};

class DZJNICall{
public:
    JavaVM *javaVM;
    JNIEnv *jniEnv;
    jobject jPlayerObj;
    jmethodID jPlayerErrorMid;
    jmethodID jPlayerPrepareMid;

public:
    DZJNICall(JavaVM *javaVM,JNIEnv *env,jobject jPlayerObj);
    ~DZJNICall();

public:
    void callPlayerError(ThreadMode threadMode,int code,char *msg);

    void callPlayerPrepared(ThreadMode threadMode);

};

#endif //MY_APPLICATION_DZJNICALL_H

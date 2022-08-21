package com.example.videoplayer;

import android.text.TextUtils;
import android.view.Surface;

/**
 * 音频播放器逻辑类
 */
public class CustomPlayer {

    static {
        System.loadLibrary("music-player");
    }

    private String url;

    private MediaErrorListener mErrorListener;

    private MediaPreparedListener mPrepareListener;

    // call from jni
    public void setOnErrorListener(MediaErrorListener errorListener) {
        this.mErrorListener = mErrorListener;
    }

    // call from jni
    public void setOnPrepareListener(MediaPreparedListener prepareListener) {
        this.mPrepareListener = prepareListener;
    }

    public void setDataSource(String url){
        this.url = url;
    }

    private void onError(int code, String msg) {
        if(mErrorListener != null){
            mErrorListener.onError(code,msg);
        }
    }

    public  void  play(){
        if(TextUtils.isEmpty(url)){

            throw new NullPointerException("url is null,please call method setDataSource");

        }

        nPlay();
    }

    public  void  prepare(){
        if(TextUtils.isEmpty(url)){

            throw new NullPointerException("url is null,please call method setDataSource");

        }

        nPrepare(url);
    }

    /**
     * 异步准备
     */
    public  void  prepareAsync(){
        if(TextUtils.isEmpty(url)){

            throw new NullPointerException("url is null,please call method setDataSource");

        }

        nPrepareAsync(url);
    }

    public void stop(){

    }




    private native void nPlay();


    private native void nPrepare(String url);

    private native void nPrepareAsync(String url);

    public native void setSurface(Surface surface);

    public  interface MediaErrorListener{

        void onError(int code,String msg);
    }

    public  interface MediaPreparedListener{

        void onPrepare();
    }

}

package com.example.videoplayer;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class DZVideoView extends SurfaceView implements CustomPlayer.MediaPreparedListener {

    private  CustomPlayer mPlayer;

    public DZVideoView(Context context) {
        this(context,null);
    }

    public DZVideoView(Context context, AttributeSet attrs) {
        this(context, attrs,0);
    }

    public DZVideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        SurfaceHolder holder = getHolder();
        holder.setFormat(PixelFormat.RGBA_8888);
        mPlayer = new CustomPlayer();

    }

    public void play(String uri) {
        stop();
        mPlayer.setDataSource(uri);
        mPlayer.prepareAsync();
    }

    /**
     * 停止方法，释放上一个视频的内存
     */
    public void stop(){
        mPlayer.stop();
    }

    @Override
    public void onPrepare() {
        mPlayer.setSurface(getHolder().getSurface());
        mPlayer.play();
    }
}

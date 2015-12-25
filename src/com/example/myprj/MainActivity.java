package com.example.myprj;

import java.io.File;

import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.ActionBarActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.VideoView;

public class MainActivity extends ActionBarActivity {
	
	private TextView tv;
	private Button btn;
	private VideoView vv;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		String dir = "/storage/sdcard1/";
		String instr = "/storage/sdcard1/out.h264";
		String outstr = "/storage/sdcard1/res.yuv";
		
		tv = (TextView)findViewById(R.id.info);
		btn = (Button)findViewById(R.id.play);
		vv = (VideoView)findViewById(R.id.videoShow);
		
		File out = new File(dir + "out.h264");
		tv.setText(fromNativeInter());
		vv.setVideoPath(dir + "sintel.mp4");
		btn.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				vv.start();
			}
		});
		
		findViewById(R.id.pause).setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				vv.pause();
			}
		});
		
		findViewById(R.id.stop).setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Decode();
			}
		});
		Decode();
	}

	
	public native String fromNativeInter();
	public native int Decode();
	
	static {
		System.loadLibrary("avutil-54");
    	System.loadLibrary("swresample-1");
    	System.loadLibrary("avcodec-56");
    	System.loadLibrary("avformat-56");
    	System.loadLibrary("swscale-3");
    	System.loadLibrary("postproc-53");
    	System.loadLibrary("avfilter-5");
    	System.loadLibrary("avdevice-56");
		System.loadLibrary("UserLib");
	}
}

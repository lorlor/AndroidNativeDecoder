#include <jni.h>
#include <android/log.h>
#include <stdio.h>

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/log.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"

#include <time.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#define LOGE(...)	__android_log_print(ANDROID_LOG_ERROR, "TAG", __VA_ARGS__)
#define LOGI(...)	__android_log_print(ANDROID_LOG_INFO, "TAG", __VA_ARGS__)
#define LOGV(...)	__android_log_print(ANDROID_LOG_VERBOSE, "TAG", __VA_ARGS__)
#define LOGD(...)	__android_log_print(ANDROID_LOG_DEBUG, "TAG", __VA_ARGS__)

jstring Java_com_example_myprj_MainActivity_fromNativeInter(JNIEnv* env,
															jobject thiz)
{
	char info[10000] = { 0 };
	av_register_all();

	sprintf(info, "%s\n", avcodec_configuration());
	LOGD("Debuf info");
	return (*env)->NewStringUTF(env, "Try again!!!");
}

jint Java_com_example_myprj_MainActivity_Decode(JNIEnv* env,
													jobject thiz)
{
		AVFormatContext	*pFormatCtx;
		int				i, videoindex;
		AVCodecContext	*pCodecCtx;
		AVCodec			*pCodec;
		AVFrame	*pFrame,*pFrameYUV;
		unsigned char *out_buffer;
		AVPacket *packet;
		int y_size;
		int ret, got_picture;
		struct SwsContext *img_convert_ctx;
		FILE *fp_yuv;
		int frame_cnt;
		clock_t time_start, time_finish;
		double  time_duration = 0.0;

		LOGD("Debug info");

//		char input_str[500]={0};
//		char output_str[500]={0};
//		char info[1000]={0};
//		sprintf(input_str,"%s",(*env)->GetStringUTFChars(env,in_file, NULL));
//		sprintf(output_str,"%s",(*env)->GetStringUTFChars(env,out_file, NULL));
		char input_str[500] = { 0 };
		char output_str[500] = { 0 };
		char info[1000] = { 0 };

		sprintf(input_str, "%s\n", "/storage/sdcard1/out.h264");
		sprintf(output_str, "%s\n", "/storage/sdcard1/res.yuv");

		av_register_all();
		avformat_network_init();
		pFormatCtx = avformat_alloc_context();

		if(avformat_open_input(&pFormatCtx,input_str,NULL,NULL)!=0){
//			LOGE("Couldn't open input stream.\n");
			return -1;
		}
		if(avformat_find_stream_info(pFormatCtx,NULL)<0){
//			LOGE("Couldn't find stream information.\n");
			return -1;
		}
		videoindex=-1;
		for(i=0; i<pFormatCtx->nb_streams; i++)
			if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
				videoindex=i;
				break;
			}
		if(videoindex==-1){
//			LOGE("Couldn't find a video stream.\n");
			return -1;
		}
		pCodecCtx=pFormatCtx->streams[videoindex]->codec;
		pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
		if(pCodec==NULL){
//			LOGE("Couldn't find Codec.\n");
			return -1;
		}
		if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
//			LOGE("Couldn't open codec.\n");
			return -1;
		}

		pFrame=av_frame_alloc();
		pFrameYUV=av_frame_alloc();
		out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
		avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
		packet=(AVPacket *)av_malloc(sizeof(AVPacket));

		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);


	  sprintf(info,   "[Input     ]%s\n", input_str);
	  sprintf(info, "%s[Output    ]%s\n",info,output_str);
	  sprintf(info, "%s[Format    ]%s\n",info, pFormatCtx->iformat->name);
	  sprintf(info, "%s[Codec     ]%s\n",info, pCodecCtx->codec->name);
	  sprintf(info, "%s[Resolution]%dx%d\n",info, pCodecCtx->width,pCodecCtx->height);

	  fp_yuv=fopen(output_str,"wb+");
	  if(fp_yuv==NULL){
			printf("Cannot open output file.\n");
			return -1;
		}

		frame_cnt=0;
		time_start = clock();

		while(av_read_frame(pFormatCtx, packet)>=0){
			if(packet->stream_index==videoindex){
				ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
				if(ret < 0){
//					LOGE("Decode Error.\n");
					return -1;
				}
				if(got_picture){
					sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
						pFrameYUV->data, pFrameYUV->linesize);

					y_size=pCodecCtx->width*pCodecCtx->height;
					fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y
					fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
					fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
					//Output info
					char pictype_str[10]={0};
					switch(pFrame->pict_type){
						case AV_PICTURE_TYPE_I:sprintf(pictype_str,"I");break;
					  case AV_PICTURE_TYPE_P:sprintf(pictype_str,"P");break;
						case AV_PICTURE_TYPE_B:sprintf(pictype_str,"B");break;
						default:sprintf(pictype_str,"Other");break;
					}
					frame_cnt++;
				}
			}
			av_free_packet(packet);
		}
		//flush decoder
		//FIX: Flush Frames remained in Codec
		while (1) {
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (ret < 0)
				break;
			if (!got_picture)
				break;
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
				pFrameYUV->data, pFrameYUV->linesize);
			int y_size=pCodecCtx->width*pCodecCtx->height;
			fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y
			fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
			fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
			//Output info
			char pictype_str[10]={0};
			switch(pFrame->pict_type){
				case AV_PICTURE_TYPE_I:sprintf(pictype_str,"I");break;
			  case AV_PICTURE_TYPE_P:sprintf(pictype_str,"P");break;
				case AV_PICTURE_TYPE_B:sprintf(pictype_str,"B");break;
				default:sprintf(pictype_str,"Other");break;
			}
			frame_cnt++;
		}
		time_finish = clock();
		time_duration=(double)(time_finish - time_start);

		sprintf(info, "%s[Time      ]%fms\n",info,time_duration);
		sprintf(info, "%s[Count     ]%d\n",info,frame_cnt);

		sws_freeContext(img_convert_ctx);

	  fclose(fp_yuv);

		av_frame_free(&pFrameYUV);
		av_frame_free(&pFrame);
		avcodec_close(pCodecCtx);
		avformat_close_input(&pFormatCtx);

		return 0;
}

#include "StdAfx.h"
#include "FileRecoder.h"
#include "VideoSourceDlg.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")




static AVStream* addStream(AVFormatContext* pFormatCxt, AVCodec **pCodec, enum AVCodecID nCodecId, EncodeParam* pParam)
{
	AVCodecContext *pCodecCxt;
	AVStream	*pStream = NULL;

	//find encoder
	*pCodec = avcodec_find_encoder(nCodecId);
	if(*pCodec == NULL)
	{
		cout<<"\nCould not find codec.";
		return NULL;
	}

	pStream = avformat_new_stream(pFormatCxt, *pCodec);
	if(pStream == NULL)
	{
		cout<<"\nCould not allocate new stream";
		return NULL;
	}

	pStream->id = pFormatCxt->nb_streams - 1;
	pCodecCxt = pStream->codec;

	switch((*pCodec)->type)
	{
	case AVMEDIA_TYPE_AUDIO:
		avcodec_get_context_defaults3(pCodecCxt, *pCodec);
		pCodecCxt->codec_id = nCodecId;

		if(pParam->bit_per_sample == 16)
			pCodecCxt->sample_fmt = AV_SAMPLE_FMT_S16;	
		else if(pParam->bit_per_sample == 8)
			pCodecCxt->sample_fmt = AV_SAMPLE_FMT_U8;	

		pCodecCxt->bit_rate = pParam->bit_rate;
		pCodecCxt->sample_rate = pParam->sample_rate;
		pCodecCxt->channels = pParam->channels;

		pCodecCxt->time_base.num = 1;
		pCodecCxt->time_base.den = pParam->sample_rate;

		break;
	case AVMEDIA_TYPE_VIDEO:
		avcodec_get_context_defaults3(pCodecCxt, *pCodec);
		
		pCodecCxt->codec_id = nCodecId;
		
		//Resolution must be multiple of two
		pCodecCxt->width = pParam->nWidth;
		pCodecCxt->height = pParam->nHeight;

		pCodecCxt->bit_rate = pParam->nBitrate*1000;	

		/* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1. */
		pCodecCxt->time_base.den = pParam->nFrameRate; //30f/s
		pCodecCxt->time_base.num = 1;
		pCodecCxt->gop_size = 900; /* emit one intra frame every 300 frames at most */
		pCodecCxt->pix_fmt = AV_PIX_FMT_YUV420P;

		if(pCodecCxt->codec_id == CODEC_ID_H264)
		{		
			//pCodecCxt->bit_rate_tolerance = pCodecCxt->bit_rate;
			//pCodecCxt->rc_max_rate = pCodecCxt->bit_rate;
			//pCodecCxt->rc_min_rate = pCodecCxt->bit_rate;
			//pCodecCxt->rc_buffer_size = (pCodecCxt->bit_rate/8)*3;
		
		
			pCodecCxt->max_b_frames = 0;
			//pCodecCxt->b_frame_strategy = 1;
			//pCodecCxt->coder_type = 1;
			//pCodecCxt->me_cmp = 1;
			//pCodecCxt->me_range = 16;
			//pCodecCxt->qmin = 10;
			//pCodecCxt->qmax = 51;
			//pCodecCxt->scenechange_threshold = 40;
			pCodecCxt->flags |= CODEC_FLAG_LOOP_FILTER;
			//pCodecCxt->me_method = ME_HEX;
			//pCodecCxt->me_method = 0;
			//pCodecCxt->me_subpel_quality = 5;
			//pCodecCxt->i_quant_factor = 0.71;
			//pCodecCxt->qcompress = 0.6;

			pCodecCxt->coder_type =  FF_CODER_TYPE_VLC;
			pCodecCxt->thread_count = 2;
		
		
			pCodecCxt->level = 51;
			pCodecCxt->profile = FF_PROFILE_H264_BASELINE;

			av_opt_set(pCodecCxt->priv_data, "preset", "ultrafast", 0);
			av_opt_set(pCodecCxt->priv_data, "tune", "zerolatency", 0);

			//av_opt_set(pCodecCxt->priv_data, "x264opts", "no-mbtree:sliced-threads:sync-lookahead=0", 0);
		}

        if (pCodecCxt->codec_id == AV_CODEC_ID_MPEG2VIDEO) 
		{
             /* just for testing, we also add B frames */
             //pCodecCxt->max_b_frames = 2;
        }

		break;

	default:
		break;
	}

	/* Some formats want stream headers to be separate. */
	if (pFormatCxt->oformat->flags & AVFMT_GLOBALHEADER)
		pCodecCxt->flags |= CODEC_FLAG_GLOBAL_HEADER;

	return pStream;
}

static AVFrame *frame;
static AVPicture picture;
static int openVideo(AVFormatContext *pFormatCxt, AVCodec *pCodec, AVStream *pStream)
{
	int ret;
	AVCodecContext* pCodecCxt = pStream->codec;

	//open codec
	if(avcodec_open2(pCodecCxt, pCodec, NULL) < 0)
	{
		cout<<"\nCould not open Video Codec.";
		return -1;
	}

	//allocate and init a re-usable frame		
	frame = avcodec_alloc_frame();
	if(!frame)
	{
		cout<<"\nCould not alloc frame.";
		return -1;
	}

	//Alloc the encoded raw picture
	ret = avpicture_alloc(&picture, pCodecCxt->pix_fmt, pCodecCxt->width, pCodecCxt->height);
	if(ret < 0)
	{
		cout<<"\nCould not alloc picture.";
		return -1;		
	}

	*((AVPicture *)frame) = picture;

}

static void close_video(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);
    //av_free(picture.data);
    av_free(frame);
}

static float t, tincr, tincr2;
static int16_t *samples;
static int audio_input_frame_size;
static int openAudio(AVFormatContext *pFormatCxt, AVCodec *pCodec, AVStream *pStream)
{
	AVCodecContext *pCodecCxt = pCodecCxt = pStream->codec;
	int ret = 0;

	//open codec
	if(avcodec_open2(pCodecCxt, pCodec, NULL) < 0)
	{
		cout<<"\nCould not open Audio Codec";
		return -1;
	}

	if (pCodecCxt->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE)
		audio_input_frame_size = 640;
	else
		audio_input_frame_size = pCodecCxt->frame_size;

	samples = (int16_t*)av_malloc(audio_input_frame_size * av_get_bytes_per_sample(pCodecCxt->sample_fmt) * pCodecCxt->channels);

	if (!samples) 
	{
		cout<<"\nCould not allocate audio samples buffer";
		return -1;
	}
}

static void close_audio(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);
 
    av_free(samples);
}

CFileRecorder::CFileRecorder(void)
{
}


CFileRecorder::CFileRecorder(void* pUser)
{
	m_pParent = (CVideoSourceDlg*)pUser;

	m_pFilename = NULL;
	m_pOutputFormat = NULL;
	m_pFormatContext = NULL;
	m_pAudioStream = NULL;
	m_pVideoStream = NULL;
	audio_codec = NULL;
	video_codec = NULL;

	m_bRecordEnable = false;

	m_bFirstAudio = true;
	m_bFirstVideo = true;
	
	m_nFrameRate = 0;
	m_nFrameCount = 0;

	m_nLastTickCount = 0;

	m_hVideoMutex = CreateMutex(NULL, FALSE, NULL);
	m_hAudioMutex = CreateMutex(NULL, FALSE, NULL);

}

CFileRecorder::~CFileRecorder(void)
{
}

int CFileRecorder::startRecord(void* pParam)
{
	av_register_all();
	avcodec_register_all();

	m_pEncodeParam = (EncodeParam*)pParam;

	m_pFilename = m_pEncodeParam->sFileName.c_str();
	int ret = 0;

	/* allocate the output media context */
	ret = avformat_alloc_output_context2(&m_pFormatContext, NULL, NULL, m_pFilename);
	if (!m_pFormatContext)
	{
        printf("Could not deduce output format from file extension: using MPEG.\n");
        ret = avformat_alloc_output_context2(&m_pFormatContext, NULL, "mpeg", NULL);
    }

	if (!m_pFormatContext) {
         return 1;
     }

     m_pOutputFormat = m_pFormatContext->oformat;

	 /* Add the audio and video streams using the default format codecs
     * and initialize the codecs. */

	 if(m_pOutputFormat->video_codec != AV_CODEC_ID_NONE)
	 {
		AVCodecID nVideoCodec;

		switch(m_pEncodeParam->nVideoCodec)
		{
		case CODEC_H264:
			nVideoCodec = CODEC_ID_H264;
			break;
		case CODEC_MPEG:
			nVideoCodec = CODEC_ID_MPEG4;
			break;
		default:
			nVideoCodec = CODEC_ID_H264;
			break;
		}

		m_pVideoStream = addStream(m_pFormatContext, &video_codec, nVideoCodec, m_pEncodeParam);

	 }
	 if(m_pOutputFormat->audio_codec != AV_CODEC_ID_NONE)
	 {
		 AVCodecID nAudioCodec;

		switch(m_pEncodeParam->nVideoCodec)
		{
		case CODEC_MP2:
			nAudioCodec = CODEC_ID_MP2;
			break;
		case CODEC_MP3:
			nAudioCodec = CODEC_ID_MP3;
			break;
		default:
			nAudioCodec = CODEC_ID_MP3;
			break;
		}

		 m_pAudioStream = addStream(m_pFormatContext, &audio_codec, nAudioCodec, m_pEncodeParam);
	 }

    /* Now that all the parameters are set, we can open the audio and
      * video codecs and allocate the necessary encode buffers. */
     if (video_codec)
		 openVideo(m_pFormatContext, video_codec, m_pVideoStream);
     if (audio_codec)
         openAudio(m_pFormatContext, audio_codec, m_pAudioStream);


	 av_dump_format(m_pFormatContext, 0, m_pFilename, 1);

     /* open the output file, if needed */
     if (!(m_pOutputFormat->flags & AVFMT_NOFILE)) {
         if (avio_open(&m_pFormatContext->pb, m_pFilename, AVIO_FLAG_WRITE) < 0) {
             cout<<"\nCould not open '%s'\n"<<m_pFilename;
             return -1;
         }
     }
 
     /* Write the stream header, if any. */
     if (avformat_write_header(m_pFormatContext, NULL) < 0) {
         cout<<"\nError occurred when opening output file\n";
         return -1;
     }

	 m_bRecordEnable = true;

	return 0;
}

int CFileRecorder::stopRecord()
{
	WaitForSingleObject(m_hVideoMutex, INFINITE);
	WaitForSingleObject(m_hAudioMutex, INFINITE);

	m_bRecordEnable = false;

	//Sleep(1000);

	/* Write the trailer, if any. The trailer must be written before you
	* close the CodecContexts open when you wrote the header; otherwise
	* av_write_trailer() may try to use memory that was freed on
	* av_codec_close(). */
	av_write_trailer(m_pFormatContext);
 
	/* Close each codec. */
	if (m_pVideoStream)
		close_video(m_pFormatContext, m_pVideoStream);
	if (m_pAudioStream)
		close_audio(m_pFormatContext, m_pAudioStream);
 
	/* Free the streams. */
	for (int i = 0; i < m_pFormatContext->nb_streams; i++) {
		av_freep(&m_pFormatContext->streams[i]->codec);
		av_freep(&m_pFormatContext->streams[i]);
	}
 
	if (!(m_pOutputFormat->flags & AVFMT_NOFILE))
		/* Close the output file. */
		avio_close(m_pFormatContext->pb);
 
	/* free the stream */
	av_free(m_pFormatContext);

	m_bFirstAudio = true;
	m_bFirstVideo = true;

	m_pFilename = NULL;
	m_pOutputFormat = NULL;
	m_pFormatContext = NULL;
	m_pAudioStream = NULL;
	m_pVideoStream = NULL;
	audio_codec = NULL;
	video_codec = NULL;

	ReleaseMutex(m_hVideoMutex);
	ReleaseMutex(m_hAudioMutex);

	return 0;
}

void CFileRecorder::writeData(int nStreamId, void* pData, int nSize, DWORD pts, bool bCopy)
{
	if(!m_bRecordEnable)
		return;

	if(nStreamId == 0)
		writeAudioData(pData, nSize, pts, bCopy);
	else
		writeVideoData(pData, nSize, pts, bCopy);

}

void CFileRecorder::writeVideoData(void* pData, int nSize, DWORD pts, bool bCopy)
{
	WaitForSingleObject(m_hVideoMutex, INFINITE);

	AVCodecContext *pCodecCxt = m_pVideoStream->codec;

	int nWidth, nHeight;
	nWidth = pCodecCxt->width;
	nHeight = pCodecCxt->height;

	AVRational time_base;
	time_base.den = 30;
	time_base.num = 1;

	//Set the start record time
	if(m_bFirstVideo)
	{
		m_bFirstVideo = false;

		m_nLastTickCount = GetTickCount();
		m_nFrameRate = m_pEncodeParam->nFrameRate;
		m_nFrameCount++;

		frame->pts = 0;
		m_nLastPts = pts;
	}
	else
	{
		m_nFrameCount++;

		if((GetTickCount() - m_nLastTickCount) >= 1000)
		{
			m_nFrameRate = m_nFrameCount;
			//TRACE("\nTime = %d ms", GetTickCount() - m_nLastTickCount);
			//TRACE("\nFrame rate = %d",m_nFrameRate);
			m_nFrameCount = 0;

			m_nLastTickCount = GetTickCount();
		}

		frame->pts += pts - m_nLastPts;

		m_nLastPts = pts;

		frame->pts += av_rescale_q(1, time_base, m_pVideoStream->time_base);
	}

	//TRACE("\nFrame Pts = %d", frame->pts);
	

	int ret = 0;

	if(!bCopy)
	{

		frame->data[0] = (uint8_t*)pData;
		frame->data[1] = (uint8_t*)pData + nWidth*nHeight;
		frame->data[2] = (uint8_t*)pData + nWidth*nHeight*5/4;

		frame->linesize[0] = nWidth;
		frame->linesize[2] = frame->linesize[1] = nWidth >> 1;

		//encode image
		AVPacket pkt;
		int got_output;

		av_init_packet(&pkt);
		pkt.data = NULL;  //packet data will be allocated by the encoder
		pkt.size = 0;


		DWORD nEncodeTime = GetTickCount();
		ret = avcodec_encode_video2(pCodecCxt, &pkt, frame, &got_output);
		//TRACE("\nEncode time = %d", GetTickCount() - nEncodeTime);

		if(ret < 0)
		{
			cout<<"\Error Encoding frame.";
			ReleaseMutex(m_hVideoMutex);
			return;
		}

		//If size is zero, it means the image was buffered
		if(got_output)
		{

			if(pCodecCxt->coded_frame->key_frame)
				pkt.flags |= AV_PKT_FLAG_KEY;
			
			pkt.stream_index = m_pVideoStream->index;
			//TRACE("\nPacket size = %d",pkt.size);

			//Write the compressed frame to the media file
			ret = av_interleaved_write_frame(m_pFormatContext, &pkt);

			//av_free_packet(&pkt);
		}
	}
	else
	{
	
	}

	if(ret < 0)
	{
		cout<<"\Error while writing video frame.";
	}
                    
	ReleaseMutex(m_hVideoMutex);
}

void CFileRecorder::writeAudioData(void* pData, int nSize, DWORD pts, bool bCopy)
{
	WaitForSingleObject(m_hAudioMutex, INFINITE);

	AVCodecContext *pCodecCxt = m_pAudioStream->codec;
	AVPacket pkt = { 0 }; // data and size must be 0;
	pkt.data = NULL;
	pkt.size = 0;
	AVFrame *pFrame = avcodec_alloc_frame();

	int got_packet, ret;
	av_init_packet(&pkt);

	pFrame->nb_samples = audio_input_frame_size;
	//pFrame->nb_samples = nSize;

	DWORD dPts;
	
	//Set the start record time
	if(m_bFirstAudio)
	{
		m_bFirstAudio = false;
		//m_nStartVideoTS = pts;
		dPts = 0;
	}
	else
	{
		//dPts = pts - m_nStartVideoTS;
	}


	if(!bCopy)
	{
		
		avcodec_fill_audio_frame(pFrame, pCodecCxt->channels, pCodecCxt->sample_fmt,
                              (uint8_t *)pData,
                              audio_input_frame_size *av_get_bytes_per_sample(pCodecCxt->sample_fmt) *pCodecCxt->channels, 
							  1);

		ret = avcodec_encode_audio2(pCodecCxt, &pkt, pFrame, &got_packet);

		if (ret < 0) 
		{
			cout<<"\nError encoding audio frame";
			ReleaseMutex(m_hAudioMutex);
			return;
		}

		if (!got_packet)
		{
			ReleaseMutex(m_hAudioMutex);
			return;
		}

		pkt.stream_index = m_pAudioStream->index;
		//pkt.pts = dPts;
 
		/* Write the compressed frame to the media file. */
		if (av_interleaved_write_frame(m_pFormatContext, &pkt) != 0) 
		{
			cout<<"\nError while writing audio frame\n";
			ReleaseMutex(m_hAudioMutex);
			return;
		}
		avcodec_free_frame(&pFrame);
	}

	av_free_packet(&pkt);


	ReleaseMutex(m_hAudioMutex);
}

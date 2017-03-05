#pragma once

extern "C"
{
#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libavutil\mathematics.h>
#include <libavutil\opt.h>
}

#include <string>

using namespace std;

#define CONFIG_OGG_MUXER 0

//Video Codec
#define CODEC_H264	1
#define CODEC_MPEG  2

//Audio Codec 
#define CODEC_MP2	10
#define CODEC_MP3	11

class CVideoSourceDlg;

typedef struct EncodeParam
{
	string sFileName;

	//Video Config
	int nVideoCodec;
	int nWidth;
	int nHeight;
	int nBitrate;
	int nFrameRate;

	//Audio Config
	int nAudioCodec;
	int sample_rate;
	int bit_per_sample;
	int channels;
	int bit_rate;
};

class CFileRecorder
{
public:
	CFileRecorder(void);
	CFileRecorder(void* pUser);

	~CFileRecorder(void);

	int startRecord(void* pParam);
	int stopRecord();

	void writeData(int nStreamId, void* pData, int nSize, DWORD pts, bool bCopy = false);

protected:
	void writeVideoData(void* pData, int nSize, DWORD pts, bool bCopy);
	void writeAudioData(void* pData, int nSize, DWORD pts, bool bCopy);


protected:
	CVideoSourceDlg* m_pParent;
	
	EncodeParam			*m_pEncodeParam;

	bool				m_bRecordEnable;

	bool				m_bFirstVideo;
	bool				m_bFirstAudio;

	HANDLE				m_hVideoMutex;
	HANDLE				m_hAudioMutex;

	const char			*m_pFilename;
    AVOutputFormat		*m_pOutputFormat;
    AVFormatContext		*m_pFormatContext;
    AVStream			*m_pAudioStream;
	AVStream			*m_pVideoStream;
    AVCodec				*audio_codec;
	AVCodec				*video_codec;
    double				audio_pts; 
	double				video_pts;	

	DWORD				m_nLastTickCount;
	DWORD				m_nLastPts;
	
	int					m_nFrameRate;
	int					m_nFrameCount;
};


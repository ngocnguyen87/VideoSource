#include "StdAfx.h"
#include "H264Encoder.h"


CH264Encoder::CH264Encoder(void)
{
}

CH264Encoder::CH264Encoder(void* pUser)
{
	m_pVideoSrc = pUser;

	m_hEncoder = NULL;

	m_bFastupdate = true;

	m_pBuffer = NULL;
	m_pEncodeCallback = NULL;

	m_hMutex = CreateMutex(NULL, false, NULL);
}

CH264Encoder::~CH264Encoder(void)
{
}

int CH264Encoder::Init(void* pParam)
{
	x264_setting* setting = (x264_setting*)pParam;

	//x264_param_default(&param);

	//param.i_threads = 1;
	//param.i_width = setting->nWidth;
	//param.i_height = setting->nHeight;
	//param.i_fps_num = 30;
	//param.i_fps_den = 1;

	//// Intra refres:
	//param.i_keyint_max = 30;
	//param.b_intra_refresh = 1;

	////Rate control:
	//param.rc.i_rc_method = X264_RC_CRF;
	//param.rc.f_rf_constant = 25;
	//param.rc.f_rf_constant_max = 35;

	////For streaming:
	//param.b_repeat_headers = 1;
	//param.b_annexb = 1;

	////Bitrate
	//param.rc.i_rc_method = X264_RC_ABR;
	//param.rc.i_bitrate = setting->nBitrate;

	//param.rc.i_vbv_max_bitrate = setting->nBitrate;
	//param.rc.i_vbv_buffer_size = setting->nBitrate;

	//m_pEncodeCallback = setting->pEncodeCallback;

	//param.i_keyint_max = 300;

	////Buffer Encode
	//m_nBufferSize = param.rc.i_bitrate * 1000;
	//m_pBuffer = (BYTE*)malloc(m_nBufferSize);


	////Open Encode
	//m_hEncoder = x264_encoder_open(&param);
	
	int nBitrateEncode = setting->nBitrate*1000;
	nBitrateEncode -= (nBitrateEncode*96) / 1000;
	nBitrateEncode = nBitrateEncode / 1000;

	x264_param_default( &param );
	param.i_threads = 0;
	param.i_width = setting->nWidth;
	param.i_height = setting->nHeight;
	param.i_fps_den = 1;
	param.i_fps_num = 30;
	
	if( param.i_fps_num == 0 )
		param.i_fps_num = 25;

	param.i_level_idc = 51;

	param.i_bframe = 0;
	param.i_bframe_bias = 0;
	param.i_bframe_adaptive = 0;
	//param.b_bframe_pyramid = 0;
	param.b_cabac = 0;
	param.i_cabac_init_idc = 0;
	param.b_interlaced = 0;


	param.rc.i_rc_method = X264_RC_ABR;
	param.rc.i_bitrate = nBitrateEncode;

	//if(pSetting->nRateControlMode == VSS_RATE_CONTROL_CBR)
	{
		param.rc.i_vbv_max_bitrate = nBitrateEncode;
		param.rc.i_vbv_buffer_size = nBitrateEncode;
	}
	param.i_keyint_max = 300;
	param.i_keyint_min = 1;

	//param.nal_constraint.b_nal_limit = 1;
	//param.nal_constraint.i_nal_size_limit = 800;


	m_nBufferSize = param.rc.i_bitrate * 1000;
	m_pBuffer = (BYTE*)malloc(m_nBufferSize);

	m_hEncoder = x264_encoder_open(&param);

	m_pEncodeCallback = setting->pEncodeCallback;

	if(m_hEncoder)
	{
		x264_encoder_parameters( m_hEncoder, &param );
		return 0;
	}

	return -1;
	//x264_picture_t pic_in, pic_out;
	//x264_picture_alloc(&pic_in, X264_CSP_I420, 1280, 720);
}

void CH264Encoder::Exit()
{
	if(m_hEncoder)
	{
		x264_encoder_close(m_hEncoder);
		m_hEncoder = NULL;
	}

	if(m_pBuffer)
	{
		free(m_pBuffer);
		m_pBuffer = NULL;
	}

	m_bFastupdate = false;
	m_nBufferSize = 0;	
}

int CH264Encoder::encode(BYTE* pData, DWORD dwSize, DWORD dwTS)
{
	WaitForSingleObject(m_hMutex, INFINITE);

	int i,j;
	x264_picture_t pic;
	x264_picture_t pic_out;
	int i_size;

	x264_nal_t* pNal;
	int iNal;

	if(m_bFastupdate)
		pic.i_type = X264_TYPE_IDR;
	else
		pic.i_type = X264_TYPE_AUTO;

	m_bFastupdate = FALSE;

	x264_picture_init( &pic );
	//x264_picture_alloc(&pic, X264_CSP_I420, param.i_width, param.i_height);

    pic.i_qpplus1 = 0;
    pic.img.i_csp = X264_CSP_I420;

	pic.img.i_plane = 3;
    pic.img.plane[0] = pData;
    pic.img.plane[1] = pic.img.plane[0] + param.i_width*param.i_height;
    pic.img.plane[2] = pic.img.plane[1] + param.i_width*param.i_height / 4;
    pic.img.i_stride[0] = param.i_width;
    pic.img.i_stride[1] = param.i_width / 2;
    pic.img.i_stride[2] = param.i_width / 2;

	DWORD startT = GetTickCount();
	if(x264_encoder_encode(m_hEncoder, &pNal, &iNal ,&pic,&pic_out) >= 0)
	{
		for(i = 0; i < iNal; i++)
		{
			i_size = m_nBufferSize;
			x264_nal_encode(m_hEncoder, m_pBuffer, &pNal[i] );
			if(m_pEncodeCallback)
			{
				m_pEncodeCallback(m_pBuffer, pNal[i].i_payload, dwTS, m_pVideoSrc);
			}
		}

		ReleaseMutex(m_hMutex);
		return 0;
	}
	else
	{
		TRACE("******************************************************************** time to encode = %d\n",GetTickCount() - startT);

		ReleaseMutex(m_hMutex);

		return -1;
	}

	ReleaseMutex(m_hMutex);
	return 0;
}